#include "server.h"
#include "workerserver.h"

Server::Server(QObject *parent) : QTcpServer (parent) {}

QString Server::GetName(){
    if(_serverName.isEmpty())
        return "Unknown server";

    return this->_serverName;
}

void Server::incomingConnection(qintptr socketDescriptor){

    emit logMessage("Incoming connection");

    WorkerServer *worker = new WorkerServer(this);

    if (!worker->setSocketDescriptor(socketDescriptor)) {
          worker->deleteLater();
          return;
    }

    //<signal, slot>

    //connect the signals coming from the worker to the slots of the central server
    //connect(worker, &WorkerServer::disconnectedFromClient, this, std::bind(&Server::userDisconnected, this, worker));
    //connect(worker, &WorkerServer::error, this, std::bind(&Server::userError, this, worker));
    //connect(worker, &WorkerServer::jsonReceived, this, std::bind(&Server::jsonReceived, this, worker, std::placeholders::_1));
    //connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);

    //Aggiungiamo ste connect quando servono cercando di renderle meno marce, sta bind non mi e' troppo chiara
    connect(worker, &WorkerServer::jsonReceived, this, &Server::jsonReceived);
    connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);

    m_clients.append(worker);
    emit logMessage("New client connected");
}

bool Server::ConnectToDatabase(QString databaseLocation){

    const QString DRIVER(this->_database);

    if(QSqlDatabase::isDriverAvailable(DRIVER)){
        this->_db = QSqlDatabase::addDatabase(DRIVER);

        if(databaseLocation.isNull() || databaseLocation.isEmpty())
               this->_db.setDatabaseName(this->_defaultDatabaseLocation);
           else
               this->_db.setDatabaseName(databaseLocation);
    }
    else
        return false;

    //Open connection
    if(!this->_db.open()){
        emit logMessage("Couldn't connect to database");
        return false;
    }
    else
        emit logMessage("Successfully connected to database");

    return true;
}

bool Server::queryDatabase(QSqlQuery& query){

    if(!query.exec()){
        std::cout << query.lastError().text().toUtf8().constData() << std::endl;
        return false;
    }
    else {
        std::cout << "Succesfull query: " << query.lastQuery().toStdString() << std::endl;
        std::cout << "Executed query: " << query.executedQuery().toStdString() << std::endl;
   }

   return true;
}


void Server::sendListFile() {

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    QDir dir;
    QJsonArray listFile;

    //set directory
    dir.filePath("Files"); //what path?
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Size | QDir::Reversed);

    //create filelist
    QFileInfoList list = dir.entryInfoList();
    for(int i=0; i<list.size(); ++i) {
        QString fileName = list.at(i).fileName();
        QJsonObject file_data;
        file_data.insert("Filename", QJsonValue(fileName));
        listFile.push_back(QJsonValue(file_data));
    }

    //put in json document
    QJsonDocument final_list(listFile);
    out.setVersion(QDataStream::Qt_5_10);
    out << final_list.toJson(QJsonDocument::Indented);

    //send to client
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, &QAbstractSocket::disconnected, clientConnection, &QObject::deleteLater);
    clientConnection->write(block);

    //than disconnect(?)
    //clientConn->disconnectFromHost();
}

void Server::sendJson(WorkerServer& dest, const QJsonObject &msg) {

    dest.sendJson(msg);
}

void Server::jsonReceived(WorkerServer& sender, const QJsonObject &doc) {

    emit logMessage("JSON received " + QString::fromUtf8(QJsonDocument(doc).toJson()));
    if(sender.userName().isEmpty())
        return jsonFromLoggedOut(sender, doc);
    //jsonFromLoggedIn(sender, doc);
}

void Server::stopServer() {
    for(WorkerServer *worker : m_clients) {
        worker->disconnectFromClient();
    }
    _db.removeDatabase(_db.connectionName());
    _db.removeDatabase((QSqlDatabase::defaultConnection));
    this->close();
}

void Server::userDisconnected(WorkerServer& sender) {

    m_clients.removeAll(&sender);
    const QString userName = sender.userName();

    if(!userName.isEmpty()) {
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = QString("userdisconnected");
        disconnectedMessage["username"] = userName;
        broadcastAll(disconnectedMessage);
        emit logMessage(userName + " disconnected");
    }

    sender.deleteLater();
}

void Server::userError(WorkerServer& sender)  {

    emit logMessage("Error from "+ sender.userName());
}

void Server::broadcastAll(const QJsonObject& message) {

    for(WorkerServer* worker : m_clients) {
        sendJson(*worker, message);
    }
}

void Server::broadcast(const QJsonObject& message, WorkerServer& exclude) {

    for(WorkerServer* worker : m_clients) {

        //TO CHECK - comparing addresses here?
        if(worker == &exclude)
        continue;

        sendJson(*worker, message);
    }
}

void Server::jsonFromLoggedOut(WorkerServer& sender, const QJsonObject &doc) {

    //Q_ASSERT(&sender);   //cosa fa? Fa debug nel caso in cui il sender non esista - passando per
    //reference il sender non puo' essere null: ci serve ancora una Q_Assert??

    if(!ConnectToDatabase())
        return;

    QSqlQuery qUser, qSignup, qVerify;
    qUser.prepare("SELECT * FROM users WHERE username = :USERNAME AND password = :PASSWORD");
    qVerify.prepare("SELECT * FROM users WHERE username =:USERNAME");
    qSignup.prepare("INSERT INTO users (username, password) VALUES (:USERNAME, :PASSWORD)");
    const QJsonValue typeVal = doc.value("type");

    if(typeVal.isNull() || !typeVal.isString())
        return;

    //login
    if(typeVal.toString().compare("login", Qt::CaseInsensitive) == 0){
        login(qUser,doc, sender);
        return;
    }

    //signup
    if(typeVal.toString().compare("signup", Qt::CaseInsensitive) == 0){
        signup(qVerify, qSignup, doc, sender);
        return;
    }

}

void Server::signup(QSqlQuery& qVerify, QSqlQuery& qSignup, const QJsonObject& doc, WorkerServer& sender) {

    const QString simplifiedUser = doc.value("username").toString().simplified();
    QJsonObject failmsg, successmsg;
    if(simplifiedUser.isNull()) return;
    qVerify.bindValue(":USERNAME", simplifiedUser);
    bindValues(qSignup,doc);

    if(queryDatabase(qVerify)) {

        if(this->countReturnedRows(qVerify) != 0) {
            failmsg["type"] = QString("signup");
            failmsg["success"] = false;
            failmsg["reason"] = QString("Username already present");
            sendJson(sender,failmsg);
            return;
        }

        if(!queryDatabase(qSignup)) {
            failmsg["type"] = QString("signup");
            failmsg["success"] = false;
            failmsg["reason"] = QString("Problem with database");
            sendJson(sender,failmsg);
            return;
        }
        sender.setUserName(simplifiedUser);
        successmsg["type"] = QString("signup");
        successmsg["success"] = true;
        successmsg["user"] = simplifiedUser;
        sendJson(sender,successmsg);
    }
}

void Server::login(QSqlQuery& q, const QJsonObject &doc, WorkerServer& sender) {

    bindValues(q,doc);

    if(queryDatabase(q)) {
        if(this->countReturnedRows(q) == 1) {
            QJsonObject msg;
            msg["type"] = QString("login");
            msg["success"] = true;
            msg["user"] = doc.value("username").toString().simplified();
            sender.setUserName(msg["user"].toString());
            sendJson(sender, msg);

            //now sending information to update other clients' GUI
            QJsonObject connectedMsg;
            connectedMsg["type"] = QString("newuser");
            connectedMsg["username"] = doc.value("username").toString().simplified();
            broadcast(connectedMsg, sender);
        }
        else { //no user found
            QJsonObject failmsg2;
            failmsg2["type"] = QString("login");
            failmsg2["success"] = false;
            failmsg2["reason"] = QString("Incorrect username and password");
            sendJson(sender,failmsg2);
        }
    }
}

void Server::bindValues(QSqlQuery& q, const QJsonObject &doc) {
    const QJsonValue userVal = doc.value("username");
    if(userVal.isNull() || !userVal.isString())
        return;

    const QString simplifiedUser = userVal.toString().simplified(); //deletes extra white spaces
    const QString password = doc.value("password").toString(); //TODO: hash password from client
    q.bindValue(":USERNAME", simplifiedUser);
    q.bindValue(":PASSWORD", password);
}

int Server::countReturnedRows(QSqlQuery& executedQuery){

    int cnt = 0;

    while(executedQuery.next())
        cnt++;

    emit logMessage("Query returned " +  QString::number(cnt) + " results");
    return cnt;
}

void Server::jsonFromLoggedIn(WorkerServer& sender, const QJsonObject &doc) {

}

void Server::logQueryResults(QSqlQuery executedQuery){

    int cnt = 0;
    QString records;

    while(executedQuery.next()){
        cnt++;
        auto record = executedQuery.record();
        for(int i=0; i < record.count(); i++){
            QSqlField field=record.field(i);
            records.append(field.name() + ":" + field.value().toString() + "\t");
        }
        records.append("\n");
    }
    records = cnt == 0 ? "Query returned 0 results" : records.prepend("Query returned " + QString::number(cnt) + " results:\n");
    emit logMessage(records);
}

void Server::executeCommand(QString cmd){

    emit logMessage("Executing command: " + cmd);
    ConnectToDatabase();
    QSqlQuery query (cmd);
    queryDatabase(query);
    logQueryResults(query);
}




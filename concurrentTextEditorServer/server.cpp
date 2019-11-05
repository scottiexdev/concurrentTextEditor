#include "server.h"
#include "workerserver.h"

Server::Server(QObject *parent) : QTcpServer (parent) {}

std::string Server::GetName(){
    if(_serverName.empty())
        return "Unknown server";

    return this->_serverName;
}

void Server::incomingConnection(qintptr socketDescriptor){
//    std::cout<<"Incoming connection"<<std::endl;
    emit logMessage("Incoming connection");
    WorkerServer *worker = new WorkerServer(this);

    if (!worker->setSocketDescriptor(socketDescriptor)) {
          worker->deleteLater();
          return;
    }

    //connect the signals coming from the worker to the slots of the central server
    connect(worker, &WorkerServer::disconnectedFromClient, this, std::bind(&Server::userDisconnected, this, worker));
    connect(worker, &WorkerServer::error, this, std::bind(&Server::userError, this, worker));
    connect(worker, &WorkerServer::jsonReceived, this, std::bind(&Server::jsonReceived, this, worker, std::placeholders::_1));
    connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);

    m_clients.append(worker);
    emit logMessage("New client connected");
}

bool Server::ConnectToDatabase(QString databaseLocation){

    const QString DRIVER(this->_database);
    if(QSqlDatabase::isDriverAvailable(DRIVER))

    this->_db = QSqlDatabase::addDatabase(DRIVER);

    //Set db
    if(databaseLocation.isNull() || databaseLocation.isEmpty())
        this->_db.setDatabaseName(this->_defaultDatabaseLocation);
    else
        this->_db.setDatabaseName(databaseLocation);

    //Open connection
    if(!this->_db.open()){
        //std::cout<<"Couldn't connect to database, Error: " << this->_db.lastError().text().toUtf8().constData() << std::endl;
        emit logMessage("Couldn't connect to database");
        return false;
    }
    else {
        //std::cout<<"Successfully connected to database"<<std::endl;
        emit logMessage("Successfully connected to database");
    }
    return true;
}

bool Server::queryDatabase(QSqlQuery* query){

    if(query == nullptr){
        std::cout << "Query is null"<< std::endl;
        return false;
    }

    if(!query->exec()){
        std::cout << query->lastError().text().toUtf8().constData() << std::endl;
        return false;
    }
    else {
        std::cout << "Succesfull query: " << query->lastQuery().toStdString() << std::endl;
        std::cout << "Executed query: " << query->executedQuery().toStdString() << std::endl;
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

void Server::sendJson(WorkerServer *dest, const QJsonObject &msg) {
    Q_ASSERT(dest);
    dest->sendJson(msg);
}

void Server::jsonReceived(WorkerServer *sender, const QJsonObject &doc) {
    Q_ASSERT(sender);
    emit logMessage("JSON received " + QString::fromUtf8(QJsonDocument(doc).toJson()));
    if(sender->userName().isEmpty())
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

void Server::userDisconnected(WorkerServer *sender) {
    m_clients.removeAll(sender);
    const QString userName = sender->userName();
    if(!userName.isEmpty()) {
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = QString("userdisconnected");
        disconnectedMessage["username"] = userName;
        broadcast(disconnectedMessage, nullptr);
        emit logMessage(userName + " disconnected");
    }

    sender->deleteLater();
}

void Server::userError(WorkerServer *sender)  {
    Q_UNUSED(sender)
    emit logMessage("Error from "+ sender->userName());
}

void Server::broadcast(const QJsonObject &message, WorkerServer *exclude) {
    for(WorkerServer *worker : m_clients) {
        Q_ASSERT(worker);
        if(worker == exclude)
            continue;
        sendJson(worker, message);
    }
}

void Server::jsonFromLoggedOut(WorkerServer *sender, const QJsonObject &doc) {

    Q_ASSERT(sender);   //cosa fa? Fa debug nel caso in cui il sender non esista

    if(!ConnectToDatabase())
        return;

    QSqlQuery qUser, qSignup, qVerify;
    qUser.prepare("SELECT * FROM users WHERE username = :USERNAME AND password = :PASSWORD");
    //qUser.prepare("SELECT * FROM users");
    qVerify.prepare("SELECT * FROM users WHERE username =:USERNAME");
    qSignup.prepare("INSERT INTO users (username, password) VALUES (:USERNAME, :PASSWORD)");
    const QJsonValue typeVal = doc.value("type");

    if(typeVal.isNull() || !typeVal.isString())
        return;

    //TODO: login and shit FUNCTIONS - refactor

    //login
    if(typeVal.toString().compare("login", Qt::CaseInsensitive) == 0) {

        login(&qUser,doc, sender);
    }

    //signup
    if(typeVal.toString().compare("signup", Qt::CaseInsensitive) == 0) {

        signup(&qVerify, &qSignup, doc, sender);
    }

}

void Server::signup(QSqlQuery *qVerify, QSqlQuery *qSignup, const QJsonObject &doc, WorkerServer *sender) {

    const QString simplifiedUser = doc.value("username").toString().simplified();
    QJsonObject failmsg, successmsg;
    if(simplifiedUser.isNull()) return;
    qVerify->bindValue(":USERNAME", simplifiedUser);
    bindValues(qSignup,doc);

    if(queryDatabase(qVerify)) {
        if(qVerify->size()!=0) {
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
        successmsg["type"] = QString("signup");
        successmsg["success"] = true;
        sendJson(sender,successmsg);
    }

}

void Server::login(QSqlQuery *q, const QJsonObject &doc, WorkerServer *sender) {
    bindValues(q,doc);
    if(queryDatabase(q)) {
        if(this->countReturnedRows(*q) == 1) {
            QJsonObject msg;
            msg["type"] = QString("login");
            msg["success"] = true;
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

void Server::bindValues(QSqlQuery *q, const QJsonObject &doc) {
    const QJsonValue userVal = doc.value("username");
    if(userVal.isNull() || !userVal.isString())
        return;

    const QString simplifiedUser = userVal.toString().simplified(); //deletes extra white spaces
    const QString password = doc.value("password").toString(); //TODO: hash password from client
    q->bindValue(":USERNAME", simplifiedUser);
    q->bindValue(":PASSWORD", password);
}

int Server::countReturnedRows(QSqlQuery executedQuery){

    int cnt = 0;

    while(executedQuery.next())
        cnt++;

    emit logMessage("Query returned " +  QString::number(cnt) + " results");
    return cnt;
}

void Server::jsonFromLoggedIn(WorkerServer *sender, const QJsonObject &doc) {

}



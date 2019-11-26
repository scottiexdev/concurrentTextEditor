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


void Server::sendListFile(WorkerServer &sender) {

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    QDir* dir = new QDir(_defaultAbsoluteFilesLocation);
    QJsonArray listFile;

    //set directory
    dir->setFilter(QDir::Files);
    dir->setSorting(QDir::Size | QDir::Reversed);

    //create filelist
    QFileInfoList list = dir->entryInfoList();

    //TODO: if directory is empty
    QJsonObject file_data;
    file_data["type"] = QString("filesRequest");
    file_data["num"] = list.size();
    file_data["requestedFiles"] = QString("all"); //for switch in showAllFileHandler

    QString buf;

    for(int i=0; i<list.size(); ++i) {
        QString fileName = list.at(i).fileName();
        if(i == 0)
            buf += fileName;
        else
            buf += "," + fileName;
    }

    file_data.insert("Filename", QJsonValue(buf));

    sendJson(sender, file_data);
}

void Server::sendJson(WorkerServer& dest, const QJsonObject &msg) {

    dest.sendJson(msg);
}

void Server::jsonReceived(WorkerServer& sender, const QJsonObject &doc) {

    emit logMessage("JSON received " + QString::fromUtf8(QJsonDocument(doc).toJson()));

    if(sender.userName().isEmpty())
        return jsonFromLoggedOut(sender, doc);

    jsonFromLoggedIn(sender, doc);
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
        successmsg["username"] = simplifiedUser;
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
            msg["username"] = doc.value("username").toString().simplified();
            sender.setUserName(msg["username"].toString());
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


void Server::jsonFromLoggedIn(WorkerServer& sender, const QJsonObject &doc) {
    messageType type = getMessageType(doc);

    switch(type) {

        case messageType::filesRequest:
            filesRequestHandler(sender, doc);
            break;

        case messageType::invalid:
            emit logMessage("JSON type request non handled");
            break;

    }
}

void Server::filesRequestHandler(WorkerServer& sender, const QJsonObject &doc) {

    QString requestedFile = doc.value(QLatin1String("requestedFiles")).toString();

    if(requestedFile == "all")
        sendListFile(sender);
    else {
        sendFile(sender, requestedFile);
    }
}

void Server::sendFile(WorkerServer& sender, QString fileName){

    //Get file and send it through the WorkerServer sender
    //add your path and comment the others
    QDir::setCurrent("C:/Users/silvi/Google Drive/Politecnico/Magistrale/ProgettoDefinitivo/concurrentTextEditor/concurrentTextEditorServer/Files/");
    QFile f(fileName);
    if(!f.open(QIODevice::Text | QIODevice::ReadWrite))
        return; //handle error, if it is deleted or else
    QJsonObject msgF;
    msgF["type"] = QString("filesRequest");
    msgF["requestedFiles"] = fileName;

    QTextStream in(&f);
    while(!in.atEnd()) {
        //QTextStream: convers 8.bit data in 16-bit unicode
        QString line = in.readLine().toLatin1();
        msgF["content"] = line;
        sendJson(sender, msgF);
    }
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

int Server::countReturnedRows(QSqlQuery& executedQuery){

    int cnt = 0;

    while(executedQuery.next())
        cnt++;

    emit logMessage("Query returned " +  QString::number(cnt) + " results");
    return cnt;
}


Server::messageType Server::getMessageType(const QJsonObject &docObj) {

    const QJsonValue typeVal = docObj.value(QLatin1String("type"));

    if(typeVal.isNull() || !typeVal.isString())
        return Server::messageType::invalid;

    const QString type = typeVal.toString();

    if(type.compare(QLatin1String("filesRequest"), Qt::CaseInsensitive) == 0)
         return Server::messageType::filesRequest;

    return Server::messageType::invalid;
}


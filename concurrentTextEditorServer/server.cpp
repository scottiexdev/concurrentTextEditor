#include "server.h"
#include "workerserver.h"

Server::Server(QObject *parent) : QTcpServer (parent) {}

std::string Server::GetName(){
    if(_serverName.empty())
        return "Unknown server";

    return this->_serverName;
}

void Server::incomingConnection(qintptr socketDescriptor){
    std::cout<<"Incoming connection"<<std::endl;

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

  //  FortuneThread *thread = new FortuneThread(socketDescriptor, fortune, this);
    //connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    //thread->start();
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
    else
        //std::cout<<"Successfully connected to database"<<std::endl;
        emit logMessage("Successfully connected to database");
    return true;
}

bool Server::queryDatabase(QSqlQuery query){

    if(!query.exec()){
        std::cout << query.lastError().text().toUtf8().constData();
        return false;
    }
    else
        std::cout << "Succesfull query: " << query.lastQuery().toStdString() << std::endl;

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
        file_data.insert("Name", QJsonValue(fileName));
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

//void Server::logMessage(const QString &msg){

//    std::cout << msg.toUtf8().constData() << std::endl;
//}

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
    close();
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
    Q_ASSERT(sender);
    if(ConnectToDatabase()) {
        QSqlQuery qUser, qSignup;
        qUser.prepare("SELECT * FROM users WHERE username=':username' AND password=':password'");
        qSignup.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
        const QJsonValue typeVal = doc.value("type");
        if(typeVal.isNull() || !typeVal.isString())
            return;

        //login
        if(typeVal.toString().compare("login", Qt::CaseInsensitive) == 0) {
            const QJsonValue userVal = doc.value("username");
            if(userVal.isNull() || !userVal.isString())
                return;

            const QString simplifiedUser = userVal.toString().simplified(); //deletes extra white spaces
            const QString password = doc.value("password").toString(); //TODO: hash password from client
            qUser.bindValue(":username", simplifiedUser);
            qUser.bindValue(":password", password);

            if(queryDatabase(qUser)) {
                if(qUser.size()==1) {
                    QJsonObject msg;
                    msg["type"] = QString("login");
                    msg["success"] = true;
                    msg["reason"] = QString("Correct username and password");
                    sendJson(sender, msg);

                    //now sending information to update other clients' GUI
                    QJsonObject connectedMsg;
                    connectedMsg["type"] = QString("newuser");
                    connectedMsg["username"] = simplifiedUser;
                    broadcast(connectedMsg, sender);
                } else { //no user found
                    QJsonObject failmsg2;
                    failmsg2["type"] = QString("login");
                    failmsg2["success"] = false;
                    failmsg2["reason"] = QString("Non existing user");
                    sendJson(sender,failmsg2);
                    return;
                }
            }
        }

        //signup
        if(typeVal.toString().compare("signup", Qt::CaseInsensitive) == 0) {
            const QJsonValue userVal = doc.value("username");
            if(userVal.isNull() || !userVal.isString())
                return;

            const QString simplifiedUser = userVal.toString().simplified(); //deletes extra white spaces
            const QString password = doc.value("password").toString(); //TODO: hash password from client
            qUser.bindValue(":username", simplifiedUser);
            qUser.bindValue(":password", password);

            if(queryDatabase(qUser)) {
                if(qSignup.size()!=0) {
                    QJsonObject failmsg;
                    failmsg["type"] = QString("signup");
                    failmsg["success"] = false;
                    failmsg["reason"] = QString("Username already present");
                    sendJson(sender,failmsg);
                    return;
                }
                //TODO: check password and hash
                qSignup.bindValue(":username", simplifiedUser);
                qSignup.bindValue(":password", password);
                if(!queryDatabase(qSignup)) {
                    QJsonObject failmsg;
                    failmsg["type"] = QString("signup");
                    failmsg["success"] = false;
                    failmsg["reason"] = QString("Problem with database");
                    sendJson(sender,failmsg);
                    return;
                }
                QJsonObject successMsg;
                successMsg["type"] = QString("signup");
                successMsg["success"] = true;
                sendJson(sender,successMsg);
            }
        }
    }

}



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

    connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);

    emit logMessage("User connected with socket: ");

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
        std::cout<<"Couldn't connect to database, Error: " << this->_db.lastError().text().toUtf8().constData() << std::endl;
        return false;
    }
    else
        std::cout<<"Successfully connected to database"<<std::endl;

    return true;
}

bool Server::queryDatabase(QString queryString){

    QSqlQuery query;

    if(!query.exec(queryString)){
        std::cout << query.lastError().text().toUtf8().constData();
        return false;
    }
    else
        std::cout << "Succesfull query: " << queryString.toUtf8().constData() << std::endl;

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

void Server::stopServer() {
    for(WorkerServer *worker : m_clients) {
        worker->disconnectFromClient();
    }
    close();
}




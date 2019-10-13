#include "server.h"

std::string Server::GetName(){
    if(_serverName.empty())
        return "Unknown server";

    return this->_serverName;
}

void Server::incomingConnection(qintptr socketDescriptor){
    std::cout<<"Incoming connection"<<std::endl;
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

    _db.isOpen();
    _db.tables();

    //Try query
    QSqlQuery query;


    if(!query.exec("INSERT INTO users (username, password) VALUES ('Silviussss', NULL);")){
        //std::cout<<"Query Error"<<std::endl;
        std::cout << query.lastError().text().toUtf8().constData();
    }



    return true;
}

void Server::sendListFile() {
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    QFileDialog listFile;

    listFile.setFileMode(QFileDialog::AnyFile); //existing and non
    listFile.setNameFilter(tr("Text: (*.txt)"));
    listFile.setViewMode(QFileDialog::List);
    out.setVersion(QDataStream::Qt_5_10); //for retrocompatibility
    listFile.setDirectory(QDir::currentPath());
    out << listFile.saveState(); //transofrm in QByteArray
    QTcpSocket *clientConn = tcpServer->nextPendingConnection();
    connect(clientConn, &QAbstractSocket::disconnected, clientConn, &QObject::deleteLater); not working
    clientConn->write(block);
    //than disconnect(?)
    //clientConn->disconnectFromHost();
}



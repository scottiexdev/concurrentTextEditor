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

bool Server::ConnectToDatabase(QString database, QString connectionString){
    this->db = QSqlDatabase::addDatabase(database);
    this->db.setDatabaseName(connectionString);

    if(!this->db.open()){
        std::cout<<"Couldn't connect to database"<<std::endl;
        return false;
    }

    return true;
}



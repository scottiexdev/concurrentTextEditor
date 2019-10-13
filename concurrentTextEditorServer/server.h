#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <iostream>
#include <string>
#include <QString>
#include <QtSql>
#include <QFileInfo>
#include <QString>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QFileDialog>
#include <QDialog>
#include <QTcpSocket>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    //Constructors
    Server() : QTcpServer(nullptr) {}
    Server(std::string serverName) : QTcpServer(nullptr), _serverName(serverName) {
        connect(tcpServer, &QTcpServer::newConnection, this, &Server::sendListFile); //new connection => send list file to show in the client
    }

    //Methods
    std::string GetName(void);
    bool ConnectToDatabase(QString databaseLocation = nullptr);


private slots:
    void sendListFile();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:    
    QTcpServer *tcpServer = nullptr;
    std::string  _serverName;
    QSqlDatabase _db;
    const QString _database = "QSQLITE";
    const QString _defaultDatabaseLocation = "/home/the_albo/Music/concurrentDb.db";
    //const QString _defaultDatabaseLocation = QDir::currentPath().append("/concurrentDb.db");
};

#endif // SERVER_H

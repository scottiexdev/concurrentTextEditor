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

class Server : public QTcpServer
{
    Q_OBJECT

public:
    //Constructors
    Server() : QTcpServer(nullptr) {}
    Server(std::string serverName) : QTcpServer(nullptr), _serverName(serverName) {}

    //Methods
    std::string GetName(void);
    bool ConnectToDatabase(QString databaseLocation = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:    
    std::string  _serverName;
    QSqlDatabase _db;
    const QString _database = "QSQLITE";
    const QString _defaultDatabaseLocation = "/home/the_albo/Music/concurrentDb.db";
    //const QString _defaultDatabaseLocation = QDir::currentPath().append("/concurrentDb.db");
};

#endif // SERVER_H

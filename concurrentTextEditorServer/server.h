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
#include "workerserver.h"

class Server : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(Server)

public:
    //Constructors
    explicit Server(QObject *parent = nullptr);
//    Server() : QTcpServer(nullptr) {}
//    Server(std::string serverName) : QTcpServer(nullptr), _serverName(serverName) {
//        //connect(tcpServer, &QTcpServer::newConnection, this, &Server::sendListFile); //new connection => send list file to show in the client
//    }

    //Methods
    std::string GetName(void);
    bool ConnectToDatabase(QString databaseLocation = nullptr);
    bool queryDatabase(QSqlQuery* query);

signals:
    void logMessage(const QString &msg);

public slots:
    void sendListFile();
    void stopServer();

private slots:
    void broadcast(const QJsonObject &message, WorkerServer *exclude);
    void jsonReceived(WorkerServer *sender, const QJsonObject &doc);
    void userDisconnected(WorkerServer *sender);
    void userError(WorkerServer *sender);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    int countReturnedRows(QSqlQuery executedQuery);

private:    
    QTcpServer *tcpServer = nullptr;
    std::string  _serverName;
    QSqlDatabase _db;
    const QString _database = "QSQLITE";
    const QString _defaultDatabaseLocation = "/home/albo/Documents/concurrentDb.db";
    //const QString _defaultDatabaseLocation = QDir::currentPath().append("/concurrentDb.db");
    QVector<WorkerServer *> m_clients;
    void jsonFromLoggedOut(WorkerServer *sender, const QJsonObject &doc);
    void jsonFromLoggedIn(WorkerServer *sender, const QJsonObject &doc);
    void sendJson(WorkerServer *dest, const QJsonObject &msg);    
};

#endif // SERVER_H

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

    //Methods
    QString GetName(void);
    bool ConnectToDatabase(QString databaseLocation = nullptr);
    bool queryDatabase(QSqlQuery& query);
    void logQueryResults(QSqlQuery query);


signals:
    void logMessage(const QString &msg);

public slots:
    void stopServer();
    void executeCommand(QString cmd);

private slots:
    void broadcast(const QJsonObject &message, WorkerServer& exclude);
    void broadcastAll(const QJsonObject& message);
    void jsonReceived(WorkerServer& sender, const QJsonObject &doc);
    void userDisconnected(WorkerServer& sender);
    void userError(WorkerServer& sender);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    int countReturnedRows(QSqlQuery& executedQuery);
    enum messageType { filesRequest, invalid };

private:    
    QTcpServer *tcpServer = nullptr;
    QString  _serverName;
    QSqlDatabase _db;
    const QString _database = "QSQLITE";
    const QString _defaultDatabaseLocation = "/home/albo/Documents/repos/concurrentTextEditor/concurrentTextEditorServer/concurrentDb.db";
    const QString _defaultAbsoluteFilesLocation = "/home/albo/Documents/repos/concurrentTextEditor/concurrentTextEditorServer/Files";
    //const QString _defaultDatabaseLocation = QDir::currentPath().append("/concurrentDb.db");
    QVector<WorkerServer *> m_clients;
    messageType getMessageType(const QJsonObject &docObj);

    void jsonFromLoggedOut(WorkerServer& sender, const QJsonObject &doc);
    void jsonFromLoggedIn(WorkerServer& sender, const QJsonObject &doc);
    void sendJson(WorkerServer& dest, const QJsonObject& msg);
    void login(QSqlQuery& q, const QJsonObject &doc, WorkerServer& sender);
    void signup(QSqlQuery& qUser, QSqlQuery& qSignup, const QJsonObject &doc, WorkerServer& sender);
    void bindValues(QSqlQuery& q, const QJsonObject &doc);
    void filesRequestHandler(WorkerServer& sender, const QJsonObject &doc);
    void sendListFile(WorkerServer& sender);
};

#endif // SERVER_H

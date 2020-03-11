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
#include "../concurrentTextEditor/Enums.h"


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

private:    
    QTcpServer *tcpServer = nullptr;
    QString  _serverName;
    QSqlDatabase _db;
    const QString _database = "QSQLITE";
    const QString _defaultDatabaseLocation = "/home/albo/Documents/repos/concurrentTextEditor/concurrentTextEditorServer/concurrentDb.db";
    const QString _defaultAbsoluteFilesLocation = "/home/albo/Documents/repos/concurrentTextEditor/concurrentTextEditorServer/Files";
    //const QString _defaultDatabaseLocation = QDir::currentPath().append("/concurrentDb.db");
//    const QString _defaultDatabaseLocation = "C:/Users/giorg/Documents/GitHub/concurrentTextEditor/concurrentTextEditor/concurrentTextEditorServer/concurrentDb.db";
//    const QString _defaultAbsoluteFilesLocation = "C:/Users/giorg/Documents/GitHub/concurrentTextEditor/concurrentTextEditor/concurrentTextEditorServer/Files";
    //const QString _defaultAbsoluteFilesLocation = "C:/Users/silvi/Google Drive/Politecnico/Magistrale/ProgettoDefinitivo/concurrentTextEditor/concurrentTextEditorServer/Files";
    //const QString _defaultDatabaseLocation = "C:/Users/silvi/Google Drive/Politecnico/Magistrale/ProgettoDefinitivo/concurrentTextEditor/concurrentTextEditorServer/concurrentDb.db";
    QVector<WorkerServer *> m_clients;
    QString _defaultDocument = "Welcome.txt";

    messageType getMessageType(const QJsonObject &docObj);
    void sendJson(WorkerServer& dest, const QJsonObject& msg);

    void jsonFromLoggedOut(WorkerServer& sender, const QJsonObject &doc);
    void jsonFromLoggedIn(WorkerServer& sender, const QJsonObject &doc);

    void login(QSqlQuery& q, const QJsonObject &doc, WorkerServer& sender);
    void signup(QSqlQuery& qUser, QSqlQuery& qSignup, const QJsonObject &doc, WorkerServer& sender);
    void bindValues(QSqlQuery& q, const QJsonObject &doc);

    void sendFile(WorkerServer& sender, QString fileName);
    void sendListFile(WorkerServer& sender);
    bool checkFilenameAvailability(QString fn);
    QJsonObject createFileData(QFileInfoList file_data);

    void newFileHandler(WorkerServer &sender, const QJsonObject &doc);
    void filesRequestHandler(WorkerServer& sender, const QJsonObject &doc);
    void userListHandler(WorkerServer& sender, const QJsonObject &doc);
    void editHandler(WorkerServer& sender, const QJsonObject &doc);

    void write(QJsonObject &qjo, QString filename) const;
};

#endif // SERVER_H

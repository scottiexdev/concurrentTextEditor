#ifndef WORKERSERVER_H
#define WORKERSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>

#include "../concurrentTextEditor/crdt.h"
class QJsonObject;

class WorkerServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkerServer)

public:
    explicit WorkerServer(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor);
    QString userName() const;
    void setUserName(const QString &userName);
    void sendJson(const QJsonObject &jsonData);
    QTcpSocket* getSocket() {return m_serverSocket;}

    void delOpenFile(const QString &fileName);
    void addOpenFile(const QString &fileName);
    QList<QString> openedFileList() const;
    Crdt getCrdt();
    void setCrdt(QString siteID);
    void insertionHandler(const QJsonObject &doc);
    void deletionHandler(const QJsonObject &doc);

signals:
    void jsonReceived(WorkerServer& sender, const QJsonObject &jsonDoc);
    void error();
    void logMessage(const QString &msg);

public slots:
    void disconnectFromClient();


private slots:
    void receiveJson();

private:
    QTcpSocket *m_serverSocket;
    QString m_userName;
    QList<QString> _openedFileList;
    Crdt _crdt;
    QMap<QString, Crdt> _openedFiles;
};

#endif // WORKERSERVER_H

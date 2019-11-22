#ifndef WORKERSERVER_H
#define WORKERSERVER_H

#include <QObject>
#include <QTcpSocket>
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
};

#endif // WORKERSERVER_H

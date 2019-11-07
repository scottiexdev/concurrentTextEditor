#ifndef WORKERCLIENT_H
#define WORKERCLIENT_H

#include <QObject>
#include <QTcpSocket>

class WorkerClient : public QObject
{
    Q_OBJECT

public:
    WorkerClient(QObject *parent= nullptr);
    void connectToServer(const QHostAddress& address, quint16 port);
    void SendLoginCred(QJsonObject qj);
    //bool receiveLoginResult();
    void setUser(QString loggedUser);
    QString getUser();

private slots:
    void onReadyRead();

signals:
    void myLoggedIn(QString loggedUser);

private:
    void jsonReceived(const QJsonObject &qjo);
    QTcpSocket* _clientSocket;
    bool _loggedIn;
    QString _loggedUser;
    const QString DEFAULT_USER  = "unknownUsername";
};

#endif // WORKERCLIENT_H

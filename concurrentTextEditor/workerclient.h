#ifndef WORKERCLIENT_H
#define WORKERCLIENT_H

#include <QObject>
#include <QTcpSocket>

class WorkerClient : public QObject
{

public:
    WorkerClient(QObject *parent= nullptr);
    void connectToServer(const QHostAddress& address, quint16 port);
    void SendLoginCred(QJsonObject qj);
    bool receiveLoginResult();

private:
    QTcpSocket* _clientSocket;
    bool _loggedIn;

};

#endif // WORKERCLIENT_H

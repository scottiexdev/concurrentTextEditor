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

private slots:
    void onReadyRead();

signals:
    void myLoggedIn();

private:
    void jsonReceived(const QJsonObject &qjo);
    QTcpSocket* _clientSocket;
    bool _loggedIn;

};

#endif // WORKERCLIENT_H

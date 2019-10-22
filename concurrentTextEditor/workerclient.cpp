#include "workerclient.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

WorkerClient::WorkerClient(QObject *parent)
    : QObject(parent)
    , _clientSocket(new QTcpSocket(this))
    , _loggedIn(false)
{

}

void WorkerClient::connectToServer(const QHostAddress& address, quint16 port){
    _clientSocket->connectToHost(address, port);
}


#include "workerserver.h"

WorkerServer::WorkerServer(QObject *parent) : QObject(parent), m_serverSocket(new QTcpSocket(this))
{
    //readyread connected to handler input data
    connect(m_serverSocket, &QTcpSocket::readyRead, this, &WorkerServer::receiveJson);

    //handlers of disconnection and errors
    connect(m_serverSocket, &QTcpSocket::disconnected, this, &WorkerServer::disconnectFromClient);
    connect(m_serverSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &WorkerServer::error);
}

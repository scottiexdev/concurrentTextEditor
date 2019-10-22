#include "workerserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>

WorkerServer::WorkerServer(QObject *parent) : QObject(parent), m_serverSocket(new QTcpSocket(this))
{
    //readyread connected to handler input data
    connect(m_serverSocket, &QTcpSocket::readyRead, this, &WorkerServer::receiveJson);

    //handlers of disconnection and errors
    connect(m_serverSocket, &QTcpSocket::disconnected, this, &WorkerServer::disconnectFromClient);
    connect(m_serverSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &WorkerServer::error);
}

bool WorkerServer::setSocketDescriptor(qintptr socketDescriptor) {
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

void WorkerServer::sendJson(const QJsonObject &json) {
    //create QJsonDocument to convert it into QByteArray
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    //notify central server about sending dats
    emit logMessage("Sending to "+ userName() + " - " + QString::fromUtf8(jsonData));

    //send message to the socket
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_10);
    socketStream << jsonData;
}

void WorkerServer::receiveJson() {
    QByteArray jsonData; //prepare container
    QDataStream socketStream(m_serverSocket); //datastrea on the socket
    socketStream.setVersion(QDataStream::Qt_5_10);
    //INFINITE LOOP
    for (;;) {
        socketStream.startTransaction(); //so we can revert to previous state in case of reading more data
        socketStream >> jsonData;
        if(socketStream.commitTransaction()) {
            //we read successfully data
            //it's a valid JSON?
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if(parseError.error == QJsonParseError::NoError) {
                //the data is a valid JSON
                if(jsonDoc.isObject()) //and an object
                    emit jsonReceived(jsonDoc.object());
                else
                    emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            } else {
                 emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            }
            //loop and try to read more JSON
        } else {
            break; //the read dailed, socket go back to the stat before the transaction and we exit the loop
        }
    }
}

void WorkerServer::disconnectFromClient() {
    m_serverSocket->disconnectFromHost();
}

QString WorkerServer::userName() const {
    return m_userName;
}

void WorkerServer::setUserName(const QString &userName) {
    m_userName = userName;
}



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
    QDataStream socketStream(m_serverSocket); //datastream on the socket
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
                    emit jsonReceived(*this, jsonDoc.object());
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

void WorkerServer::disconnectFromClient(){
    m_serverSocket->disconnectFromHost();
}

QString WorkerServer::userName() const {
    return m_userName;
}

void WorkerServer::setUserName(const QString &userName) {
    m_userName = userName;
}

void WorkerServer::addOpenFile(const QString &fileName) {
    _openedFileList.append(fileName);
    _openedFiles.insert(fileName, Crdt());
}

void WorkerServer::delOpenFile(const QString &fileName) {
    _openedFileList.removeOne(fileName);
    _openedFiles.remove(fileName);
}
QList<QString> WorkerServer::openedFileList() const {
    return _openedFileList;
}

Crdt WorkerServer::getCrdt() {
    return _crdt;
}

void WorkerServer::setCrdt(QString siteID) {
    _crdt = Crdt(siteID);
}

void WorkerServer::insertionHandler(const QJsonObject &doc){

    //Open file from database - cteFile
    QFile file(doc["fileName"].toString());
    file.open(QIODevice::ReadWrite);
    QJsonDocument cteFile = QJsonDocument::fromJson(file.readAll());
    file.close();

    //Estrazione campi del json
    QJsonObject cteData = cteFile.object();
    QJsonArray cteContent = cteData["content"].toArray(); //Array di Char da parsare

    // Nuovo char viene preso da "doc" (JsonObject ricevuto)
     QJsonObject newChar = doc["content"].toObject();

    /*
    // Estrazione di Char da newChar JSonObject
    QChar val = newChar["value"].toInt();
    QUuid siteID = newChar["siteID"].toString();
    int counter = newChar["counter"].toInt();
    QJsonArray identifiers = newChar["position"].toArray();
    QList<Identifier> positions;

    foreach (const QJsonValue &tmpID, identifiers) {
        QJsonObject ID = tmpID.toObject();
        int digit = ID["digit"].toInt();
        QUuid oldSiteID = ID["siteID"].toString();
        Identifier identifier(digit,oldSiteID);
        positions.append(identifier);
    }

    Char c(val,counter,siteID,positions);

    */

    // DOESN'T MATTER WHERE WE APPEND
    cteContent.append(newChar);

    /*
    // Find correct index in file
    int index = sender.getCrdt().findInsertIndex(c);
    // Insert in Json file at correct index
    cteContent.insert(index, newChar);
    // Insert Char in file Crdt
    sender.getCrdt().updateFileAtIndex(index, c);
    */

    cteData["content"] = cteContent;
    cteFile.setObject(cteData);

    // Write Json file to disk
    file.open(QIODevice::WriteOnly);
    file.write(cteFile.toJson());
    file.close();
}

void WorkerServer::deletionHandler(const QJsonObject &doc){

}

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

void WorkerClient::SendLoginCred(QJsonObject qj) {
    QDataStream loginStream(_clientSocket);

    loginStream << QJsonDocument(qj).toJson();
}

bool WorkerClient::receiveLoginResult() {
    QByteArray res;
    QDataStream socketStream(_clientSocket);

    for(;;) {
        socketStream>>res;
        if(socketStream.commitTransaction()) {
            QJsonParseError check;
            const QJsonDocument jDoc = QJsonDocument::fromJson(res, &check);

            if(check.error == QJsonParseError::NoError) {
                if(jDoc.isObject()){
                    //start Json parsing - TODO errors to be displayed
                    const QJsonValue typeVal = jDoc.object().value(QLatin1String("type"));
                    //check integrity
                    if (typeVal.isNull() || !typeVal.isString())
                        return false;
                    //check type
                    if (typeVal.toString().compare(QLatin1String("login"), Qt::CaseInsensitive) == 0) { //it's login msg
                        const QJsonValue resultVal = jDoc.object().value(QLatin1String("success"));
                        if (resultVal.isNull() || !resultVal.isBool())
                            return false;
                        return resultVal.toBool();
                    } else
                        return false;
                }
            }
        } else
            break;
    }
}

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
    connect(_clientSocket, &QTcpSocket::readyRead, this, &WorkerClient::onReadyRead);
}

void WorkerClient::connectToServer(const QHostAddress& address, quint16 port){
    _clientSocket->connectToHost(address, port);
}

void WorkerClient::sendLoginCred(QJsonObject qj) {

    QDataStream loginStream(_clientSocket);
    loginStream << QJsonDocument(qj).toJson();
}

void WorkerClient::onReadyRead()
{
    // prepare a container to hold the UTF-8 encoded JSON we receive from the socket
    QByteArray jsonData;
    // create a QDataStream operating on the socket
    QDataStream socketStream(_clientSocket);
    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    socketStream.setVersion(QDataStream::Qt_5_7);
    // start an infinite loop
    for (;;) {
        // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        socketStream.startTransaction();
        // we try to read the JSON data
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            // we successfully read some data
            // we now need to make sure it's in fact a valid JSON
            QJsonParseError parseError;
            // we try to create a json document with the data we received
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                // if the data was indeed valid JSON
                if (jsonDoc.isObject()) // and is a JSON object
                    jsonReceived(jsonDoc.object()); // parse the JSON
            }
            // loop and try to read more JSONs if they are available
        } else {
            // the read failed, the socket goes automatically back to the state it was in before the transaction started
            // we just exit the loop and wait for more data to become available
            break;
        }
    }
}

//DROPPED CONST MODIFIER ON QJSONOBJECT - TO CHECK WHY IT WAS THERE
void WorkerClient::jsonReceived(const QJsonObject &docObj)
{

    messageType type = getMessageType(docObj);

    switch (type) {

        case messageType::login:
            loginHandler(docObj);
            break;
        case messageType::signup:
                signupHandler(docObj);
                break;
        case messageType::filesRequest:
                showallFilesHandler(docObj);
                break;
        default:
                return;
    }
}

void WorkerClient::setUser(QString loggedUser){
    _loggedUser = (loggedUser.isNull() || loggedUser.isEmpty()) ? DEFAULT_USER : loggedUser;
}

QString WorkerClient::getUser(){
    return _loggedUser;
}

//Setup and send Json asking for file list
void WorkerClient::getFileList(){

    //Request message
    QJsonObject filesRequest;

    filesRequest["type"] = "filesRequest";
    filesRequest["requestedFiles"] = "all";

    //Send request message
    QDataStream filesRequestStream(_clientSocket);
    filesRequestStream << QJsonDocument(filesRequest).toJson();
}

WorkerClient::messageType WorkerClient::getMessageType(const QJsonObject &docObj){

    //Get type of message received
    const QJsonValue typeVal = docObj.value(QLatin1String("type"));

    if (typeVal.isNull() || !typeVal.isString())
        return WorkerClient::messageType::invalid; // a message with no type was received so we just ignore it

    const QString type = typeVal.toString();

    if(type.compare(QLatin1String("signup"), Qt::CaseInsensitive) == 0)
            return WorkerClient::messageType::signup;

    if(type.compare(QLatin1String("login"), Qt::CaseInsensitive) == 0)
        return WorkerClient::messageType::login;

    if(type.compare(QLatin1String("filesRequest"), Qt::CaseInsensitive) == 0)
        return WorkerClient::messageType::filesRequest;
}

void WorkerClient::loginHandler(const QJsonObject& docObj){

    if (_loggedIn)
        return;

    const QJsonValue resultVal = docObj.value(QLatin1String("success"));

    //No success field
    if (resultVal.isNull() || !resultVal.isBool())
        return;

    const bool loginSuccess = resultVal.toBool();

    //Check success field
    if (loginSuccess) {
        //Notify with signal that the login was successfull
        const QJsonValue resultVal = docObj.value(QLatin1String("user"));
        emit myLoggedIn(resultVal.toString());
        return;
    }
    // the login attempt failed, we extract the reason of the failure from the JSON
    // and notify it via the loginError signal
    //************** ADAPT LOGIN ERROR MSG TO BE CONSISTENT WITH CURRENT IMPLEMENTATION ********
    //const QJsonValue reasonVal = docObj.value(QLatin1String("reason"));
    //emit loginError(reasonVal.toString());
}

void WorkerClient::signupHandler(const QJsonObject &jsonObj) {
    const QJsonValue res = jsonObj.value(QLatin1String("success"));

    //No success field
    if (res.isNull() || !res.isBool())
        return;
    const bool signSucc = res.toBool();

    if(signSucc) {
        //signup is ok, i can close dialogsignup window and open homeloggedin
        const QJsonValue resultVal = jsonObj.value(QLatin1String("username"));
        emit mySignupOk(resultVal.toString());
        return;
    }
}

void WorkerClient::showallFilesHandler(const QJsonObject &qjo) {

}



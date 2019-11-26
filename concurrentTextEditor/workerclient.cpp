#include "workerclient.h"


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

/*bool WorkerClient::receiveLoginResult() {
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
                        if(resultVal.toBool())
                            return true;
                        else return false;
                    } else
                        return false;
                }
            }
        } else
            return false;
    }
}*/

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
//    else if (typeVal.toString().compare(QLatin1String("message"), Qt::CaseInsensitive) == 0) { //It's a chat message
//        // we extract the text field containing the chat text
//        const QJsonValue textVal = docObj.value(QLatin1String("text"));
//        // we extract the sender field containing the username of the sender
//        const QJsonValue senderVal = docObj.value(QLatin1String("sender"));
//        if (textVal.isNull() || !textVal.isString())
//            return; // the text field was invalid so we ignore
//        if (senderVal.isNull() || !senderVal.isString())
//            return; // the sender field was invalid so we ignore
//        // we notify a new message was received via the messageReceived signal
//        emit messageReceived(senderVal.toString(), textVal.toString());
//    } else if (typeVal.toString().compare(QLatin1String("newuser"), Qt::CaseInsensitive) == 0) { // A user joined the chat
//        // we extract the username of the new user
//        const QJsonValue usernameVal = docObj.value(QLatin1String("username"));
//        if (usernameVal.isNull() || !usernameVal.isString())
//            return; // the username was invalid so we ignore
//        // we notify of the new user via the userJoined signal
//        emit userJoined(usernameVal.toString());
//    } else if (typeVal.toString().compare(QLatin1String("userdisconnected"), Qt::CaseInsensitive) == 0) { // A user left the chat
//         // we extract the username of the new user
//        const QJsonValue usernameVal = docObj.value(QLatin1String("username"));
//        if (usernameVal.isNull() || !usernameVal.isString())
//            return; // the username was invalid so we ignore
//        // we notify of the user disconnection the userLeft signal
//        emit userLeft(usernameVal.toString());
//    }

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

    return WorkerClient::messageType::invalid;
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
        const QJsonValue resultVal = docObj.value(QLatin1String("username"));
        _loggedUser = resultVal.toString();
        this->_loggedIn = true;
        emit myLoggedIn();
        return;
    }

    //added sub.handler to handle unsuccessful login attempts
    if(!loginSuccess) {
        QString msg = docObj.value(QLatin1String("reason")).toString();
        QMessageBox err;
        err.setText(msg);
        err.exec();
    }
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
        this->_loggedUser = resultVal.toString();
        this->_loggedIn = true;
        emit mySignupOk();
        return;
    }

    //added sub.handler to handle unsuccessful signup attempts
    if(!signSucc) {
        QString msg = jsonObj.value(QLatin1String("reason")).toString();
        QMessageBox err;
        err.setText(msg);
        err.exec();
    }
}

void WorkerClient::showallFilesHandler(const QJsonObject &qjo) {
    //emit verso la gui per update della gui
    int n = qjo["num"].toInt();
    QString buf = qjo["Filename"].toString();
    QStringList list = buf.split(",", QString::SkipEmptyParts);
    emit showFiles(list);
}

void WorkerClient::requestFile(QString fileName){
    QJsonObject fileRequest;

    fileRequest["type"] = "filesRequest";
    fileRequest["requestedFiles"] = fileName;

    QDataStream filesRequestStream(_clientSocket);
    filesRequestStream << QJsonDocument(fileRequest).toJson();
}

void WorkerClient::newFileRequest(const QJsonObject &qjo){
    //Json with filename and request type is passed as argument

    //I Just send it to the server
    QDataStream newFileReq(_clientSocket);
    newFileReq << QJsonDocument(qjo).toJson();
}

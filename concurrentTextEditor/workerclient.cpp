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

<<<<<<< Updated upstream
void WorkerClient::SendLoginCred(QJsonObject qj) {
=======
void WorkerClient::sendLoginCred(QJsonObject qj) {
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
void WorkerClient::jsonReceived(const QJsonObject &docObj)
{
    // actions depend on the type of message
    const QJsonValue typeVal = docObj.value(QLatin1String("type"));
    if (typeVal.isNull() || !typeVal.isString())
        return; // a message with no type was received so we just ignore it
    if (typeVal.toString().compare(QLatin1String("login"), Qt::CaseInsensitive) == 0) { //It's a login message
        if (_loggedIn)
            return; // if we are already logged in we ignore
        // the success field will contain the result of our attempt to login
        const QJsonValue resultVal = docObj.value(QLatin1String("success"));
        if (resultVal.isNull() || !resultVal.isBool())
            return; // the message had no success field so we ignore
        const bool loginSuccess = resultVal.toBool();
        if (loginSuccess) {
            // we logged in succesfully and we notify it via the loggedIn signal
            emit myLoggedIn();
=======
//DROPPED CONST MODIFIER ON QJSONOBJECT - TO CHECK WHY IT WAS THERE
void WorkerClient::jsonReceived(QJsonObject docObj)
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
>>>>>>> Stashed changes
            return;
        }
        // the login attempt failed, we extract the reason of the failure from the JSON
        // and notify it via the loginError signal
        //************** ADAPT LOGIN ERROR MSG TO BE CONSISTENT WITH CURRENT IMPLEMENTATION ********
        //const QJsonValue reasonVal = docObj.value(QLatin1String("reason"));
        //emit loginError(reasonVal.toString());
    }
<<<<<<< Updated upstream
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
=======

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

    //added else to mute warning about reaching end of non void function
    else {
        return WorkerClient::messageType::invalid;
    }
}

void WorkerClient::loginHandler(QJsonObject& docObj){

    if (_loggedIn) //we are already logged in
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

void WorkerClient::signupHandler(QJsonObject &jsonObj) {
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

void WorkerClient::showallFilesHandler(QJsonObject &qjo) {

>>>>>>> Stashed changes
}

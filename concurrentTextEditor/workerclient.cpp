#include "workerclient.h"

#include <QErrorMessage>

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
    messageType type = static_cast<messageType>(docObj["type"].toInt());

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
        case messageType::newFile:
            newFileError();
            break;
        case messageType::userList:
            showUserListHandler(docObj); //qua ci metto anche la rimozione di un utente da mandare in broadcast
            break;
        case messageType::edit:
            emit handleRemoteEdit(docObj);
            break;
    case messageType::openFile:
            emit ifFileOpenOk(docObj);
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
void WorkerClient::getFileList(QString access){

    //Request message
    QJsonObject filesRequest;

    filesRequest["type"] = messageType::filesRequest;
    filesRequest["requestedFiles"] = "all";

    // Request update of "private" "public" or "all" files
    filesRequest["access"] = access.isNull() || access.isEmpty() ? "all" : access;

    //Send request message
    sendJson(filesRequest);
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
    if(qjo["requestedFiles"] == "all") {

        bool isPublic = qjo["access"].toBool();
        QString buf = qjo["Filename"].toString();
        QStringList list = buf.split(",", QString::SkipEmptyParts);
        QString buf2 = qjo["Created"].toString();
        QStringList list2 = buf2.split(",", QString::SkipEmptyParts);
        QString buf3 = qjo["Owner"].toString();
        QStringList list3 = buf3.split(",", QString::SkipEmptyParts);

        emit showFiles(list,list2,list3, isPublic);
    } else {

        QJsonDocument doc(qjo);
        emit handleFile(doc);
    }
}

void WorkerClient::requestFile(QString fileName, QUuid siteID, bool isPublic){
    QJsonObject fileRequest;

    fileRequest["type"] = messageType::filesRequest;
    fileRequest["requestedFiles"] = fileName;
    fileRequest["siteID"] = siteID.toString();
    fileRequest["access"] = isPublic == true ? "public" : "private";

    sendJson(fileRequest);
}

void WorkerClient::newFileRequest(const QJsonObject &qjo){
    //Json with filename and request type is passed as argument
    //I Just send it to the server
    sendJson(qjo);
}

void WorkerClient::newFileError() {
    emit genericError("File with this name is already present. Please choose another");
}

void WorkerClient::requestUserList(QString fileName) {

    QJsonObject userListRequest;

    userListRequest["type"] = messageType::userList;
    userListRequest["action"] = action::request;
    userListRequest["fileName"] = fileName;
    QDataStream userListRequestStream(_clientSocket);
    userListRequestStream << QJsonDocument(userListRequest).toJson();
}

void WorkerClient::showUserListHandler(const QJsonObject &qjo) {
    action act = static_cast<action>(qjo["action"].toInt());

    switch(act) {

        case action::add:
            emit showUser(qjo["username"].toString());
            break;

        case action::del:
            emit deleteUser(qjo["username"].toString());
            break;

        case action::show:
            QString buf = qjo["username"].toString();
            QStringList list = buf.split(",", QString::SkipEmptyParts);
            for(QString user : list) {
                emit showUser(user); //one user at time to permit to use the same signal when somebody else connects
            }
            break;
    }
}

void WorkerClient::userJoined(QString fileName, QString user) {
    QJsonObject userJoined;
    userJoined["type"] = messageType::userList;
    userJoined["action"] = action::add;
    userJoined["fileName"] = QString(fileName);
    userJoined["user"] = QString(user);

    sendJson(userJoined);
}

void WorkerClient::userLeft(QString fileName, QString user) {
    QJsonObject userLeft;
    userLeft["type"] = messageType::userList;
    userLeft["action"] = action::del;
    userLeft["fileName"] = QString(fileName);
    userLeft["user"] = QString(user);

    sendJson(userLeft);
}

void WorkerClient::broadcastEditWorker(QString fileName, Char c, EditType editType, int index, bool isPublic){

    QJsonObject edit;
    QJsonObject content;

    content["value"] = QJsonValue(c._value);
    content["counter"] = QJsonValue(c._counter);
    content["siteID"] = QJsonValue(c._siteID.toString());
    QJsonArray position;
    for(Identifier id : c._position) {
        QJsonObject pos;
        pos["digit"] = QJsonValue(id._digit);
        pos["siteID"] = QJsonValue(id._siteID.toString());
        position.append(pos);
    }
    content["position"] = position;
    content["index"] = index;

    edit["fileName"] = fileName;
    edit["username"] = _loggedUser;
    edit["type"] = messageType::edit;
    edit["editType"] = editType;
    edit["content"] = content;

    // Set access
    edit["access"] = isPublic;

    sendJson(edit);
}

void WorkerClient::sendJson(const QJsonObject &doc) {
    QDataStream stream(_clientSocket);
    stream << QJsonDocument(doc).toJson();
}

void WorkerClient::saveLinkToServer(const QJsonObject& qjo){

    sendJson(qjo);
}

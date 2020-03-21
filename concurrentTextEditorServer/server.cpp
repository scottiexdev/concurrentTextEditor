#include "server.h"
#include "workerserver.h"

Server::Server(QObject *parent) : QTcpServer (parent) {}

QString Server::GetName(){
    if(_serverName.isEmpty())
        return "Unknown server";

    return this->_serverName;
}

void Server::incomingConnection(qintptr socketDescriptor){

    emit logMessage("Incoming connection");

    WorkerServer *worker = new WorkerServer(this);

    if (!worker->setSocketDescriptor(socketDescriptor)) {
          worker->deleteLater();
          return;
    }

    //<signal, slot>

    //connect the signals coming from the worker to the slots of the central server
    //connect(worker, &WorkerServer::disconnectedFromClient, this, std::bind(&Server::userDisconnected, this, worker));
    //connect(worker, &WorkerServer::error, this, std::bind(&Server::userError, this, worker));
    //connect(worker, &WorkerServer::jsonReceived, this, std::bind(&Server::jsonReceived, this, worker, std::placeholders::_1));
    //connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);

    //Aggiungiamo ste connect quando servono cercando di renderle meno marce, sta bind non mi e' troppo chiara
    connect(worker, &WorkerServer::jsonReceived, this, &Server::jsonReceived);
    connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);

    m_clients.append(worker);
    emit logMessage("New client connected");
}

bool Server::ConnectToDatabase(QString databaseLocation){

    const QString DRIVER(this->_database);

    if(QSqlDatabase::isDriverAvailable(DRIVER)){
        this->_db = QSqlDatabase::addDatabase(DRIVER);

        if(databaseLocation.isNull() || databaseLocation.isEmpty())
               this->_db.setDatabaseName(this->_defaultDatabaseLocation);
           else
               this->_db.setDatabaseName(databaseLocation);
    }
    else
        return false;

    //Open connection
    if(!this->_db.open()){
        emit logMessage("Couldn't connect to database");
        return false;
    }
    else
        emit logMessage("Successfully connected to database");

    return true;
}

bool Server::queryDatabase(QSqlQuery& query){

    if(!query.exec()){
        std::cout << query.lastError().text().toUtf8().constData() << std::endl;
        return false;
    }
    else {
        std::cout << "Succesfull query: " << query.lastQuery().toStdString() << std::endl;
        std::cout << "Executed query: " << query.executedQuery().toStdString() << std::endl;
   }

   return true;
}


void Server::sendListFile(WorkerServer &sender, bool isPublic) {

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    QDir dir;

    if(isPublic)
        dir = QDir(_defaultAbsolutePublicFilesLocation);
    else
        dir = QDir(_defaultAbsoluteFilesLocation + sender.userName());

    QJsonArray listFile;

    //set directory
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Size | QDir::Reversed);

    //create filelist
    QFileInfoList list = dir.entryInfoList();

    QJsonObject file_data = createFileData(list, isPublic);
    sendJson(sender, file_data);
}


QJsonObject Server::createFileData(QFileInfoList list, bool isPublic){

    QJsonObject file_data;
    file_data["type"] = messageType::filesRequest;
    file_data["num"] = list.size();
    file_data["requestedFiles"] = QString("all"); //for switch in showAllFileHandler
    file_data["access"] = isPublic;
    QString buf;

    //TO CHECK and think through when CRDT is set up
    if(list.size() == 1 && list.at(0).fileName() == _defaultDocument){
        file_data.insert("Filename", _defaultDocument);
        file_data.insert("Created", list.at(0).birthTime().toString());
        file_data.insert("Owner", "ConcurrentTextEditorTeam");

        return file_data;
    }

    for(int i=0; i<list.size(); ++i) {
        QString fileName = list.at(i).fileName();
        if(i == 0)
            buf += fileName;
        else
            buf += "," + fileName;
    }

    file_data.insert("Filename", QJsonValue(buf));

    QString buf2; //for data creation
    for(int i=0; i<list.size(); ++i) {
        QString created = list.at(i).birthTime().toString();
        if(i == 0)
            buf2 += created;
        else
            buf2 += "," + created;
    }

    file_data.insert("Created", QJsonValue(buf2));

    QString buf3; //for data owner
    for(int i=0; i<list.size(); ++i) {
        QString owner = list.at(i).owner();
        if(i == 0)
            buf3 += owner;
        else
            buf3 += "," + owner;
    }

    return file_data;
}

void Server::sendJson(WorkerServer& dest, const QJsonObject &msg) {

    dest.sendJson(msg);
}

void Server::jsonReceived(WorkerServer& sender, const QJsonObject &doc) {

    emit logMessage("JSON received " + QString::fromUtf8(QJsonDocument(doc).toJson()));

    if(sender.userName().isEmpty())
        return jsonFromLoggedOut(sender, doc);

    jsonFromLoggedIn(sender, doc);
}

void Server::stopServer() {
    for(WorkerServer *worker : m_clients) {
        worker->disconnectFromClient();
    }
    _db.removeDatabase(_db.connectionName());
    _db.removeDatabase((QSqlDatabase::defaultConnection));
    this->close();
}

void Server::userDisconnected(WorkerServer& sender) {

    m_clients.removeAll(&sender);
    const QString userName = sender.userName();

    if(!userName.isEmpty()) {
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = QString("userdisconnected");
        disconnectedMessage["username"] = userName;
        broadcastAll(disconnectedMessage);
        emit logMessage(userName + " disconnected");
    }

    sender.deleteLater();
}

void Server::userError(WorkerServer& sender)  {

    emit logMessage("Error from "+ sender.userName());
}

void Server::broadcastAll(const QJsonObject& message) {

    for(WorkerServer* worker : m_clients) {
        sendJson(*worker, message);
    }
}

void Server::broadcast(const QJsonObject& message, WorkerServer& exclude) {

    for(WorkerServer* worker : m_clients) {

        //TO CHECK - comparing addresses here?
        if(worker == &exclude)
            continue;

        sendJson(*worker, message);
    }
}

void Server::jsonFromLoggedOut(WorkerServer& sender, const QJsonObject &doc) {

    if(!ConnectToDatabase())
        return;

    QSqlQuery qUser, qSignup, qVerify;
    qUser.prepare("SELECT * FROM users WHERE username = :USERNAME AND password = :PASSWORD");
    qVerify.prepare("SELECT * FROM users WHERE username =:USERNAME");
    qSignup.prepare("INSERT INTO users (username, password) VALUES (:USERNAME, :PASSWORD)");
    const QJsonValue typeVal = doc.value("type");

    if(typeVal.isNull() || !typeVal.isString())
        return;

    //login
    if(typeVal.toString().compare("login", Qt::CaseInsensitive) == 0){
        login(qUser,doc, sender);
        return;
    }

    //signup
    if(typeVal.toString().compare("signup", Qt::CaseInsensitive) == 0){
        signup(qVerify, qSignup, doc, sender);
        return;
    }

}

void Server::signup(QSqlQuery& qVerify, QSqlQuery& qSignup, const QJsonObject& doc, WorkerServer& sender) {

    const QString simplifiedUser = doc.value("username").toString().simplified();
    QJsonObject failmsg, successmsg;
    if(simplifiedUser.isNull()) return;
    qVerify.bindValue(":USERNAME", simplifiedUser);
    bindValues(qSignup,doc);

    if(queryDatabase(qVerify)) {

        if(this->countReturnedRows(qVerify) != 0) {
            failmsg["type"] = QString("signup");
            failmsg["success"] = false;
            failmsg["reason"] = QString("Username already present");
            sendJson(sender,failmsg);
            return;
        }

        if(!queryDatabase(qSignup)) {
            failmsg["type"] = QString("signup");
            failmsg["success"] = false;
            failmsg["reason"] = QString("Problem with database");
            sendJson(sender,failmsg);
            return;
        }
        sender.setUserName(simplifiedUser);
        successmsg["type"] = QString("signup");
        successmsg["success"] = true;
        successmsg["username"] = simplifiedUser;
        sendJson(sender,successmsg);
    }
}

void Server::login(QSqlQuery& q, const QJsonObject &doc, WorkerServer& sender) {

    bindValues(q,doc);

    if(queryDatabase(q)) {

        if(this->countReturnedRows(q) == 1) {
            QJsonObject msg;
            msg["type"] = QString("login");
            msg["success"] = true;
            msg["username"] = doc.value("username").toString().simplified();
            sender.setUserName(msg["username"].toString());
            sendJson(sender, msg);

            //now sending information to update other clients' GUI
            QJsonObject connectedMsg;
            connectedMsg["type"] = QString("newuser");
            connectedMsg["username"] = doc.value("username").toString().simplified();
            broadcast(connectedMsg, sender);
        }
        else { //no user found
            QJsonObject failmsg2;
            failmsg2["type"] = QString("login");
            failmsg2["success"] = false;
            failmsg2["reason"] = QString("Incorrect username and password");
            sendJson(sender,failmsg2);
        }
    }
}

void Server::bindValues(QSqlQuery& q, const QJsonObject &doc) {
    const QJsonValue userVal = doc.value("username");
    if(userVal.isNull() || !userVal.isString())
        return;

    const QString simplifiedUser = userVal.toString().simplified(); //deletes extra white spaces
    const QString password = doc.value("password").toString(); //TODO: hash password from client
    q.bindValue(":USERNAME", simplifiedUser);
    q.bindValue(":PASSWORD", password);
}


void Server::jsonFromLoggedIn(WorkerServer& sender, const QJsonObject &doc) {

    messageType type = static_cast<messageType>(doc["type"].toInt());

    switch(type) {

        case messageType::filesRequest:
            filesRequestHandler(sender, doc);
            break;

        case messageType::invalid:
            emit logMessage("JSON type request non handled");
            break;

        case messageType::newFile:
            newFileHandler(sender, doc);
            break;

        case messageType::userList:
            userListHandler(sender, doc); //qua ci metto la gestione della rimozione di un utente da mandare in broadcast
            break;

        case messageType::edit:
            editHandler(sender, doc);
            break;

        case messageType::invite:
            inviteHandler(sender, doc);
            break;

        case messageType::deleteFile:
            deleteFileHandler(sender, doc);
            break;

        default:
            emit logMessage("Message type not handled");

    }
}

void Server::filesRequestHandler(WorkerServer& sender, const QJsonObject &doc) {

    QString requestedFile = doc.value(QLatin1String("requestedFiles")).toString();
    QString access = doc["access"].toString();
    bool isPublic = access == "public";

    if(requestedFile == "all"){
        // Send list of files for both private and public directories
        if(access == "all"){
            sendListFile(sender, true);
            sendListFile(sender, false);
        }
        else
            // Send only public if public requested, send private otherwise
            sendListFile(sender, isPublic);
    }
    else
        sendFile(sender, requestedFile, isPublic);

    return;
}

void Server::sendFile(WorkerServer& sender, QString fileName, bool isPublic){

    //Get file and send it through the WorkerServer sender
    // Is the new file private or public? Set directory according to it

    checkPublic(fileName, sender.userName(), isPublic);

    QFile f(fileName);
    if(!f.open(QIODevice::ReadWrite))
        return; //handle error, if it is deleted or else
    QJsonObject msgF;
    msgF["type"] = messageType::filesRequest;
    msgF["requestedFiles"] = fileName;
    QString buf = f.readAll();
    msgF["fileContent"] = buf;
    if(fileName.split("/").size() == 2) { //without this server sees two copies of the same file opened
        fileName = fileName.split("/")[1];
    }
    // Carica e parsa file se non gia' aperto
    if(!_openedFiles.contains(fileName)){
        Crdt file;
        file.parseCteFile(QJsonDocument(msgF));
        _openedFiles.insert(fileName, file);
    }

    sender.addOpenFile(fileName); // FORSE IL SENDER DEVE COMUNQUE TENERE USER/FILE Aggiunge file alla lista dei file aperti

    sendJson(sender, msgF);       // Manda file in formato Json, unparsed
    f.close();
}

void Server::logQueryResults(QSqlQuery executedQuery){

    int cnt = 0;
    QString records;

    while(executedQuery.next()){
        cnt++;
        auto record = executedQuery.record();
        for(int i=0; i < record.count(); i++){
            QSqlField field=record.field(i);
            records.append(field.name() + ":" + field.value().toString() + "\t");
        }
        records.append("\n");
    }
    records = cnt == 0 ? "Query returned 0 results" : records.prepend("Query returned " + QString::number(cnt) + " results:\n");
    emit logMessage(records);
}

void Server::executeCommand(QString cmd){

    emit logMessage("Executing command: " + cmd);
    ConnectToDatabase();
    QSqlQuery query (cmd);
    queryDatabase(query);
    logQueryResults(query);
}

int Server::countReturnedRows(QSqlQuery& executedQuery){

    int cnt = 0;

    while(executedQuery.next())
        cnt++;

    emit logMessage("Query returned " +  QString::number(cnt) + " results");
    return cnt;
}


bool Server::checkFilenameAvailability(QString filename, QString username, bool isPublic){

    // Check for public files directory existence and create it if it doesn't exist
    QDir publicDir = QDir(_defaultAbsolutePublicFilesLocation);
    if(!publicDir.exists())
        QDir().mkdir(publicDir.path());

    if(isPublic){
        return checkFilenameInDirectory(filename, QDir(_defaultAbsolutePublicFilesLocation), isPublic);
    }
    else{        
        // Create private directory path
        QString privateDirectoryPath = _defaultAbsoluteFilesLocation + username;
        return checkFilenameInDirectory(filename, QDir(privateDirectoryPath), isPublic);
    }
}

bool Server::checkFilenameInDirectory(QString filename, QDir directory, bool isPublic){

    QJsonArray listPublicFile, listPrivateFile;

    // Check for private directory existence and create it if it doesn't exist
    if(!directory.exists() && !isPublic){
        // does this work?
        QDir().mkdir(directory.path());
    }

    //get files in directory and sort them
    directory.setFilter(QDir::Files);
    directory.setSorting(QDir::Size | QDir::Reversed);

    //create filelist
    QFileInfoList filesList = directory.entryInfoList();

    // Check presence of file
    for(int i=0; i< filesList.size(); i++) {
        if (filesList.at(i) == filename)
            return false;
    }

    return true;
}

void Server::newFileHandler(WorkerServer &sender, const QJsonObject &doc) {

     //TODO: implement this with exceptions
     QString filename = doc.value("filename").toString();
     bool publicAccess = doc["access"].toBool();

     if (checkFilenameAvailability(filename, sender.userName(), publicAccess)){

        checkPublic(filename, sender.userName(), publicAccess);
        // Is the new file private or public? Set directory according to it

        // Create file
        QJsonObject qjo;

        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        writeEmptyFile(qjo, filename);
        QByteArray data = QJsonDocument(qjo).toJson();
        file.write(data);
        file.close();

        sendListFile(sender, publicAccess);

        //Refresh file list for everyone
        for(WorkerServer * s : m_clients) {
            sendListFile(*s, true);
            sendListFile(*s, false);
        }

     } else {
         QJsonObject err;
         err["type"] = "newFile";
         err["success"] = false;
         sender.sendJson(err);
     }
}

void Server::writeEmptyFile(QJsonObject &qjo, QString filename) const {

    qjo["filename"] = filename;
    qjo["content"] = QJsonValue::Null;
}

void Server::userListHandler(WorkerServer &sender, const QJsonObject &doc) {

    action act = static_cast<action>(doc["action"].toInt());
    QString fileName = doc.value("fileName").toString();
    QString effectiveFileName = fileName;
    //openedFileList may contain either {fileName} or {user}/{fileName}
    if(fileName.split("/").size() == 2) {
        effectiveFileName = fileName.split("/")[1];
    } else {
        effectiveFileName = sender.userName()+"/"+fileName;
    }
    QJsonObject userList;
    QJsonObject userAdd;
    QJsonObject userDel;
    QString buf;
    int i=0;

    switch(act) {
        case action::request:

            userList["type"] = messageType::userList;
            userList["action"] = action::show;
            for(WorkerServer* c : m_clients) {
                QList<QString> openedFile = c->openedFileList();
                if(openedFile.contains(fileName) || openedFile.contains(effectiveFileName)) {
                    if(i==0) {
                        buf += c->userName();
                        i++;
                    }
                    else {
                        buf += "," + c->userName();
                    }
                }
            }
            userList["username"] = buf;
            sender.sendJson(userList);
            break;

        case action::add:

            userAdd["type"] = messageType::userList;
            userAdd["action"] = action::add;
            userAdd["username"] = QString(doc["user"].toString());
            //broadcast specifico, invio solo a quelli che hanno il file aperto
            for(WorkerServer* worker : m_clients) {

                if(worker == &sender)
                    continue;
                QList<QString> openedFile = worker->openedFileList();
                if(openedFile.contains(fileName) || openedFile.contains(effectiveFileName))
                    sendJson(*worker, userAdd);
            }
            break;

        case action::del:
            userDel["type"] = messageType::userList;
            userDel["action"] = action::del;
            userDel["username"] = QString(doc["user"].toString());

            //broadcast specifico, invio solo a quelli che hanno il file aperto
            for(WorkerServer* worker : m_clients) {
                QList<QString> openedFile = worker->openedFileList();
                if(openedFile.contains(fileName) || openedFile.contains(effectiveFileName)) {
                    sender.delOpenFile(fileName);
                    sendJson(*worker, userDel);
                }
            }
            break;
        }
}



void Server::editHandler(WorkerServer &sender, const QJsonObject &doc) {

    EditType edit = static_cast<EditType>(doc["editType"].toInt());

    switch(edit) {

        case EditType::insertion:
            insertionHandler(doc, sender);
            break;

        case EditType::deletion:
            deletionHandler(doc, sender);
            break;

        case EditType::format:
            formatHandler(doc, sender);
            break;

    }
}
void Server::insertionHandler(const QJsonObject &doc, WorkerServer &sender){

    //Open file from database - cteFile
    QString filename = doc["fileName"].toString();
    bool isPublic= doc["access"].toBool();

    // Change application working directory based on public or private file write on disk
    checkPublic(filename, sender.userName(), isPublic);

    //Open Json file
    QFile file(filename);
    file.open(QIODevice::ReadWrite);
    QJsonDocument cteFile = QJsonDocument::fromJson(file.readAll());
    file.close();
    if(filename.split("/").count() == 2) {
        filename = filename.split("/")[1];
    }
    Crdt crdtFile = _openedFiles.value(filename);

    //Estrazione campi del json
    QJsonObject cteData = cteFile.object();
    QJsonArray cteContent = cteData["content"].toArray(); //Array di Char da parsare

    // Nuovo char viene preso da "doc" (JsonObject ricevuto) e indice relativo a _file
    QJsonObject newChar = doc["content"].toObject();
    // NewChar viene parsato e trasformato in Char obj
    Char c = crdtFile.getChar(newChar);

    // Find correct index with crdt structure
    int index = crdtFile.findInsertIndex(c);
    // Keep crdt updated
    crdtFile.insertChar(c, index);

    // inserzione al posto giusto nel JsonArray da updatare per il file conservato sul server
    cteContent.insert(index, newChar);

    // Update data structures
    cteData["content"] = cteContent;
    cteFile.setObject(cteData);
    _openedFiles.insert(filename, crdtFile);

    // Write Json file to disk
    file.open(QIODevice::WriteOnly);
    file.write(cteFile.toJson());
    file.close();

    broadcastOnlyOpenedFile(filename, doc, sender);
}

void Server::deletionHandler(const QJsonObject &doc, WorkerServer &sender){

    //Open file from database - cteFile
    QString filename = doc["fileName"].toString();
    bool isPublic= doc["access"].toBool();

    checkPublic(filename, sender.userName(), isPublic);


    //Open Json file
    QFile file(filename);
    file.open(QIODevice::ReadWrite);
    QJsonDocument cteFile = QJsonDocument::fromJson(file.readAll());
    file.close();

    if(filename.split("/").count() == 2) {
        filename = filename.split("/")[1];
    }

    Crdt crdtFile = _openedFiles.value(filename);

    //Estrazione campi del json
    QJsonObject cteData = cteFile.object();
    QJsonArray cteContent = cteData["content"].toArray(); //Array di Char da parsare

    // Char da eliminare viene preso da "doc" (JsonObject ricevuto) insieme all'indice
    QJsonObject delChar = doc["content"].toObject();

    Char c = crdtFile.getChar(delChar);
    int index = crdtFile.findIndexByPosition(c);

    // Update data structures (remote delete)
    cteContent.removeAt(index);
    crdtFile.deleteChar(c, index);
    cteData["content"] = cteContent;
    cteFile.setObject(cteData);
    _openedFiles.insert(filename, crdtFile);

    // Write Json file to disk
    file.open(QIODevice::WriteOnly);
    file.write(cteFile.toJson());
    file.close();

    broadcastOnlyOpenedFile(filename, doc, sender);
}

void Server::formatHandler(const QJsonObject &doc, WorkerServer &sender) {
    QString filename = doc["fileName"].toString();
    bool isPublic= doc["access"].toBool();

    checkPublic(filename, sender.userName(), isPublic);


    //Open Json file
    QFile file(filename);
    file.open(QIODevice::ReadWrite);
    QJsonDocument cteFile = QJsonDocument::fromJson(file.readAll());
    file.close();

    if(filename.split("/").count() == 2) {
        filename = filename.split("/")[1];
    }

    Crdt crdtFile = _openedFiles.value(filename);

    //Estrazione campi del json
    QJsonObject cteData = cteFile.object();
    QJsonArray cteContent = cteData["content"].toArray(); //Array di Char da parsare
    QJsonObject formatChar = doc["content"].toObject();

    Char c = crdtFile.getChar(formatChar);
    int index = crdtFile.findIndexByPosition(c);

    cteContent.replace(index, formatChar);
    crdtFile.replaceChar(c, index);

    cteData["content"] = cteContent;
    cteFile.setObject(cteData);
    _openedFiles.insert(filename, crdtFile);

    // Write Json file to disk
    file.open(QIODevice::WriteOnly);
    file.write(cteFile.toJson());
    file.close();

    broadcastOnlyOpenedFile(filename, doc, sender);
}

void Server::broadcastOnlyOpenedFile(QString fileName, const QJsonObject& qjo, WorkerServer& sender) {

    for(WorkerServer* worker : m_clients) {

        if(worker == &sender)
            continue;

        QList<QString> openedFile = worker->openedFileList();        
        if(openedFile.contains(fileName))
            sendJson(*worker, qjo);
    }
}

void Server::inviteHandler(WorkerServer &sender, const QJsonObject &doc) {

    QString linksPath = _defaultAbsoluteFilesLocation + "Links";
    QDir linksDir = QDir(linksPath);

    // Check directory existence and create it if it doesn't exist
    if(!linksDir.exists()){
        // does this work?
        QDir().mkdir(linksDir.path());
    }

    QDir::setCurrent(linksPath);

    QFile file("invite_links");
    file.open(QIODevice::ReadWrite);
    QJsonDocument invitesDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject invitesObj = invitesDoc.object();
    QJsonArray invites = invitesObj["invites"].toArray();

    // Extracted fields
    QString linkStr = doc["link"].toString();
    EditType operation = static_cast<EditType>(doc["operation"].toInt());

    // Insert new invite link in invite links json file
    if(operation == EditType::insertion){

        //Append invite to list of valid invites
        invites.append(linkStr);
        invitesObj["invites"] = invites;
        invitesDoc.setObject(invitesObj);

        // Write Json file to disk
        file.open(QIODevice::WriteOnly);
        file.write(invitesDoc.toJson());
        file.close();
    }

    bool found = false;

    // Check if link is present means that the link is valid
    if(operation == EditType::check){        

        // Find in links
        for (QJsonValue jsonLink : invites){
            if(jsonLink.toString() == linkStr){
                found = true;
                break;
            }
        }

        // Send response: link valid or not
        QJsonObject linkValidation;
        if(found) {
            QStringList parts = linkStr.split("/");
            QString fileName = parts[1];
            QString userName = parts[2];
            linkValidation["type"] = messageType::openFile;
            linkValidation["fileName"] = userName+"/"+fileName;
            linkValidation["sharing"] = true;
            sendJson(sender, linkValidation);
        } else {
            linkValidation["type"] = messageType::invalid; //da cambiare in openFile, ma lascio questo ommento per i posteri
            linkValidation["reason"] = "Invalid link";
            // Send response to client
            sender.sendJson(linkValidation);
        }


    }

    return;
}

void Server::deleteFileHandler(WorkerServer &sender, const QJsonObject &doc) {

    bool isPublic = doc["isPublic"].toBool();
    QString fileName = doc["fileName"].toString();

    checkPublic(fileName, sender.userName(), isPublic);

    bool notPresent = checkFilenameInDirectory(fileName, QDir::current(), isPublic);
    if(!notPresent) {
        QFile::remove(fileName);
        _openedFiles.remove(fileName); //anche se non serve perchè fa solo cache però è meglio perchè libero memoria per il server (idea del recupero)
        broadcastOnlyOpenedFile(fileName, doc, sender);
    }
    //Refresh file list for everyone
    for(WorkerServer * s : m_clients) {
        sendListFile(*s, true);
        sendListFile(*s, false);
    }
}

void Server::checkPublic(QString fileName, QString userName, bool isPublic) {
    if(!isPublic){
        if(fileName.split("/").size() == 2) {
            QDir::setCurrent(_defaultAbsoluteFilesLocation); //shared file: fileName is {user}/{file} => QFile will work
            fileName.split("/")[0] + "/";
        } else {
            QDir::setCurrent(_defaultAbsoluteFilesLocation + userName);
        }
    }
    else{
        QDir::setCurrent(_defaultAbsolutePublicFilesLocation);
    }
}

#include "server.h"
#include "workerserver.h"

Server::Server(QObject *parent) : QTcpServer (parent) {

    // Check public directory existance and it if it doesn't exist
    QDir dir = QDir(_defaultFilesLocation);
    if(!dir.exists())
        QDir().mkdir(dir.path());

    dir = QDir(_defaultPublicFilesLocation);
    if(!dir.exists())
        QDir().mkdir(dir.path());

    dir = QDir(_defaultIconPath);
    if(!dir.exists())
        QDir().mkdir(dir.path());
}


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

    connect(worker, &WorkerServer::jsonReceived, this, &Server::jsonReceived);
    connect(worker, &WorkerServer::logMessage, this, &Server::logMessage);
    connect(worker, &WorkerServer::userDisconnected, this, &Server::userDisconnected);

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

void Server::notifyServerDown() {
    QJsonObject serverDown;
    serverDown["type"] = messageType::serverDown;
    broadcastAll(serverDown);
}


void Server::sendListFile(WorkerServer &sender, bool isPublic) {

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    QDir dir;

    if(isPublic)
        dir = QDir(_defaultPublicFilesLocation);
    else
        dir = QDir(_defaultFilesLocation + sender.userName());

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
    m_clients.clear();
    this->close();
}

void Server::userDisconnected(WorkerServer& sender) {

    m_clients.removeAll(&sender);
    const QString userName = sender.userName();
    const QList<QString> senderOpenedFile = sender.openedFileList();

    if(!userName.isEmpty()) {
        QJsonObject userDel;
        userDel["type"] = messageType::userList;
        userDel["action"] = action::del;
        userDel["username"] = userName;

        for(QString fileName : senderOpenedFile) {
            broadcastOnlyOpenedFile(fileName,userDel,sender);
        }
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
    qSignup.prepare("INSERT INTO users (username, password, email, icon) VALUES (:USERNAME, :PASSWORD, :EMAIL, :ICON)");
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

    //get default icon in signup
    if(typeVal.toString().compare("saveicon", Qt::CaseInsensitive) == 0){
        saveIcon(doc);
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
        successmsg["email"] = doc.value("email").toString();
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
            QSqlQuery q;
            QString email;
            q.prepare("SELECT email FROM users WHERE username = :USER");
            q.bindValue(":USER", doc.value("username").toString().simplified());
            q.exec();
            while (q.next()) {
                email = q.value(0).toString();
            }
            msg["email"] = email;
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
    const QString email = doc.value("email").toString();
    QString icon = doc.value("icon").toString();
    q.bindValue(":USERNAME", simplifiedUser);
    q.bindValue(":PASSWORD", password);
    q.bindValue(":EMAIL", email);
    if(icon == "default")
        icon = _defaultIcon;
    else
        icon = _defaultIconPath+icon;
    q.bindValue(":ICON", icon);
}


void Server::jsonFromLoggedIn(WorkerServer &sender, const QJsonObject &doc) {

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
            userListHandler(sender, doc);
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

        case messageType::getCurrentUserIcon:
            currentIconHandler(sender, doc);
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

    QJsonObject msgF;
    msgF["type"] = messageType::filesRequest;
    msgF["requestedFiles"] = fileName;

    if(fileName.split("/").size() == 2) { //without this server sees two copies of the same file opened
        fileName = fileName.split("/")[1];
    }

    if(_openedFiles.contains(fileName)) {
        Crdt crdtF = _openedFiles.value(fileName);
        QJsonObject fileContent = crdtF.crdtToJson();

        msgF["fileContent"] = QString(QJsonDocument(fileContent).toJson());
    }
    else {
        // Carica e parsa file se non gia' aperto
        QFile f(fileName);
        if(!f.open(QIODevice::ReadWrite))
            return; //handle error, if it is deleted or else

        QString buf = f.readAll();
        msgF["fileContent"] = buf;
        f.close();

        Crdt file;
        file.parseCteFile(QJsonDocument(msgF));
        _openedFiles.insert(fileName, file);
    }

//    if(!_openedFiles.contains(fileName)){

//    }

    sender.addOpenFile(fileName);

    sendJson(sender, msgF);       // Manda file in formato Json, unparsed

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

    if(isPublic){
        return checkFilenameInDirectory(filename, QDir(_defaultPublicFilesLocation), isPublic);
    }
    else{        
        // Create private directory path
        QString privateDirectoryPath = _defaultFilesLocation + username;
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
            int opened = 0;
            for(WorkerServer* worker : m_clients) {

                QList<QString> openedFile = worker->openedFileList();
                if(openedFile.contains(fileName) || openedFile.contains(effectiveFileName)) {
                    sender.delOpenFile(fileName);
                    sendJson(*worker, userDel);
                    opened++;
                }
            }
            if(opened == 1) { //only one people had the file opened => save file in cte
                saveFile(fileName);
            }
            break;
        }
}

void Server::saveFile(QString filename) {

    QJsonObject content;
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    Crdt crdtF = _openedFiles.value(filename);
    content = crdtF.crdtToJson();
    file.write(QJsonDocument(content).toJson());
    file.close();
    _openedFiles.remove(filename);
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

        case EditType::propic:
            propicHandler(doc);
            break;

        case EditType::username:
            userHandler(doc, sender);
            break;

        case EditType::password:
            passwordHandler(doc, sender);
            break;

        case EditType::email:
            emailHandler(doc, sender);
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
//    QFile file(filename);
//    file.open(QIODevice::ReadWrite);
//    QJsonDocument cteFile = QJsonDocument::fromJson(file.readAll());
//    file.close();
    if(filename.split("/").count() == 2) {
        filename = filename.split("/")[1];
    }
    Crdt crdtFile = _openedFiles.value(filename);

//    //Estrazione campi del json
//    QJsonObject cteData = cteFile.object();
//    QJsonArray cteContent = cteData["content"].toArray(); //Array di Char da parsare

    // Nuovo char viene preso da "doc" (JsonObject ricevuto) e indice relativo a _file

    QPair<int,int> rowCh = crdtFile.handleRemoteInsert(doc);
//    QJsonObject newChar = doc["content"].toObject();
// /*   NewChar viene parsato e trasformato in Char obj
//    Char c = crdtFile.getChar(newChar);

// //    // Find correct index with crdt structure
// //    QPair<int,int> rowCh = crdtFile.findInsertPosition(c);
// //    // Keep crdt updated
// //    crdtFile.insertChar(c, rowCh);*/
//    int index = crdtFile.calcIndex(rowCh);
//    // inserzione al posto giusto nel JsonArray da updatare per il file conservato sul server
//    cteContent.insert(index, newChar);

//    // Update data structures
//    cteData["content"] = cteContent;
//    cteFile.setObject(cteData);
    _openedFiles.insert(filename, crdtFile);

    // Write Json file to disk
//    file.open(QIODevice::WriteOnly);
//    file.write(cteFile.toJson());
//    file.close();

    broadcastOnlyOpenedFile(filename, doc, sender);
}

void Server::deletionHandler(const QJsonObject &doc, WorkerServer &sender){

    //Open file from database - cteFile
    QString filename = doc["fileName"].toString();
    bool isPublic = doc["access"].toBool();

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
    QPair<int,int> position = crdtFile.handleRemoteDelete(doc);

    // Update data structures (remote delete)
    cteContent.removeAt(crdtFile.calcIndex(position));
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

    QPair<int,int> position = crdtFile.handleRemoteFormat(doc);
    QJsonObject formatChar = doc["content"].toObject();

    cteContent.replace(crdtFile.calcIndex(position), formatChar);

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

    QString linksPath = _defaultFilesLocation + "Links";
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
            linkValidation["type"] = messageType::invalid;
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
        _openedFiles.remove(fileName);
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
            QDir::setCurrent(_defaultFilesLocation); //shared file: fileName is {user}/{file} => QFile will work
            fileName.split("/")[0] + "/";
        } else {
            QDir::setCurrent(_defaultFilesLocation + userName);
        }
    }
    else{
        QDir::setCurrent(_defaultPublicFilesLocation);
    }
}

void Server::propicHandler(const QJsonObject &doc){
    // get img
    auto encoded = doc["image"].toString().toLatin1();
    QPixmap p;
    p.loadFromData(QByteArray::fromBase64(encoded));
    QImage img = p.toImage();


    img.save(_defaultIconPath+doc["filename"].toString());

    if(encoded.isNull() || encoded.isEmpty()) {
        // TODO: json che invii messaggio di immagine non supportata
    } else {
        // eseguo update nel db
        QSqlQuery q;
        q.prepare("UPDATE users SET icon = :ICON WHERE username = :USER");
        q.bindValue(":USER", doc["username"]);
        q.bindValue(":ICON", _defaultIconPath+doc["filename"].toString());
        queryDatabase(q);
    }
}

void Server::userHandler(const QJsonObject &doc, WorkerServer &sender){
    const QString user = doc["username"].toString().simplified();

    const QString new_one = doc["new_usn"].toString().simplified();
    bool ok = checkUsernameAvailability(new_one);

    QJsonObject qjo;
    if (ok){
        QSqlQuery q;
        q.prepare("UPDATE users SET username = :NEWUSER WHERE username = :USERNAME");
        q.bindValue(":USERNAME", user);
        q.bindValue(":NEWUSER", new_one);

        queryDatabase(q);
        qjo["username"] = new_one;

        sender.setUserName(new_one);

        // change directory name if it exists
        QDir currentDir(_defaultFilesLocation);
        QDir oldDir(_defaultFilesLocation+"/"+user);
        if (oldDir.exists())
            currentDir.rename(oldDir.dirName(), new_one);

    } else qjo["username"] = user;

    qjo["type"] = messageType::edit;
    qjo["editType"] = EditType::username;
    qjo["success"] = ok;

    sendJson(sender, qjo);
}

bool Server::checkUsernameAvailability(QString n_usn){
    QSqlQuery q;
    q.prepare("SELECT username FROM users WHERE username = :USER");
    q.bindValue(":USER", n_usn);

    if(queryDatabase(q) && !q.next())
        return true;
    else return false;
}

QString Server::getIcon(QString user){ //metodo usato in login
    QSqlQuery q;
    QString icn;
    q.prepare("SELECT icon FROM users WHERE username = :USER");
    q.bindValue(":USER", user);
    q.exec();
    while (q.next()) {
        icn = q.value(0).toString();
    }
    return icn;
}

void Server::saveIcon(const QJsonObject &qj){
    // get img
    auto encoded = qj["image"].toString().toLatin1();
    QPixmap p;
    p.loadFromData(QByteArray::fromBase64(encoded));
    QImage img = p.toImage();


    img.save(_defaultIconPath+qj["filename"].toString());
}

void Server::currentIconHandler(WorkerServer &sender, const QJsonObject& qj){
    QString path = getIcon(qj["username"].toString());

    QByteArray ba = getLatinStringFromImg(path);
    QLatin1String img = QLatin1String(ba);

    QJsonObject qjo;
    qjo["type"] = messageType::getCurrentUserIcon;
    qjo["image"] = img;

    sendJson(sender, qjo);
}

void Server::passwordHandler(const QJsonObject &doc, WorkerServer &sender){
    QSqlQuery q;
    QString user = doc["username"].toString();
    QString pwd = doc["password"].toString();
    q.prepare("UPDATE users SET password = :PASSWORD WHERE username = :USERNAME");
    q.bindValue(":USERNAME", user);
    q.bindValue(":PASSWORD", pwd);

    QJsonObject response;
    response["username"] = user;
    response["type"] = messageType::edit;
    response["editType"] = EditType::password;


    if(queryDatabase(q))
        response["success"] = true;
    else response["success"] = false;

    sendJson(sender, response);
}

void Server::emailHandler(const QJsonObject &doc, WorkerServer &sender){
    QSqlQuery q;
    QString user = doc["username"].toString();
    QString email = doc["email"].toString();
    q.prepare("UPDATE users SET email = :EMAIL WHERE username = :USERNAME");
    q.bindValue(":USERNAME", user);
    q.bindValue(":EMAIL", email);

    queryDatabase(q);

    QJsonObject response;
    response["username"] = user;
    response["type"] = messageType::edit;
    response["editType"] = EditType::email;
    response["email"] = email;


    if(queryDatabase(q))
        response["success"] = true;
    else response["success"] = false;

    sendJson(sender, response);
}

QByteArray Server::getLatinStringFromImg(QString path){
    QPixmap pm(path);
    QString form = path.split(".").last().toUpper();
    QByteArray buf_form = form.toLocal8Bit();
    const char * format = buf_form.data();

    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    pm.save(&buf, format);
    auto ba = buf.data().toBase64();
    QLatin1String img = QLatin1String(ba);

    return ba;
}

#include "editor.h"
#include "ui_editor.h"

#include <exception>

Editor::Editor(QWidget *parent, WorkerClient *worker, QString fileName, bool isPublic) :
    QMainWindow(parent),
    ui(new Ui::Editor),
    _workerClient(worker)
{
    ui->setupUi(this);
    this->setWindowTitle(fileName);
    connect(_workerClient, &WorkerClient::handleFile, this, &Editor::handleFile);
    connect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    connect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    connect(ui->editorController, &EditorController::broadcastEditWorker, _workerClient, &WorkerClient::broadcastEditWorker);
    connect(_workerClient, &WorkerClient::handleRemoteEdit, ui->editorController, &EditorController::handleRemoteEdit);

    _workerClient->requestFile(fileName, ui->editorController->getSiteID(), isPublic);

    //Prende lista degli utenti attivi su quel file
    _workerClient->requestUserList(fileName); // DEVE SAPERE SE PRIVATO O PUBBLICO?

    //Notifica il server che l'utente si e' connesso a quel file
    _workerClient->userJoined(fileName, _workerClient->getUser());    
    ui->editorController->setAccess(isPublic);
}

Editor::~Editor()
{
    disconnect(_workerClient, &WorkerClient::handleFile, this, &Editor::handleFile);
    disconnect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    disconnect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    disconnect(ui->editorController, &EditorController::broadcastEditWorker, _workerClient, &WorkerClient::broadcastEditWorker);
    disconnect(_workerClient, &WorkerClient::handleRemoteEdit, ui->editorController, &EditorController::handleRemoteEdit);
    delete ui;
}

//Al posto di QString avremo il JSonDocument ricevuto, che corrisponde al file
void Editor::handleFile(QJsonDocument unparsedFile) {
    //Il document ricevuto viene passato a editorcontroller che lo converte il data structures
    //c++ e lo visualizza
    if(!ui->editorController->parseCteFile(unparsedFile)){
        //throw exception;
    }
    ui->editorController->write();
}

void Editor::showUser(QString user) {
    QListWidgetItem *newUser = new QListWidgetItem(user);
    newUser->setBackground(_colors.at(_colorNumber)); //creare una palette
    _colorNumber++;
    ui->listWidget->addItem(newUser);
}

QString Editor::deleteUser(QString user) {
    for(int i=0; i < ui->listWidget->count(); i++) {
        if(ui->listWidget->item(i)->text()==user) {
            _colorNumber--;
            return ui->listWidget->takeItem(i)->text();
        }
    }
    //TODO: see how to manage exceptions, how to distinguish them
    throw ("Users isn't active on file, user not found");
}

void Editor::closeEvent(QCloseEvent *event) {
    QString user;
    user = _workerClient->getUser();
    _workerClient->userLeft(ui->editorController->getFileName(), user);
    this->deleteLater();
}



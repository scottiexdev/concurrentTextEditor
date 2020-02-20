#include "editor.h"
#include "ui_editor.h"

#include <exception>

Editor::Editor(QWidget *parent, WorkerClient *worker, QString fileName) :
    QMainWindow(parent),    
    ui(new Ui::Editor),
    _workerClient(worker)
{
    ui->setupUi(this);
    this->setWindowTitle(fileName);
    connect(_workerClient, &WorkerClient::showFileLine, this, &Editor::showFileLine);
    connect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    connect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    _workerClient->requestFile(fileName);
    _workerClient->requestUserList(fileName);
    _workerClient->userJoined(fileName, _workerClient->getUser());
}

Editor::~Editor()
{
    delete ui;
}

void Editor::showFileLine(QString buf) {
    ui->textEdit->append(buf);
}

void Editor::showUser(QString user) {
    ui->listWidget->addItem(user);
}

QString Editor::deleteUser(QString user) {
    for(int i=0; i < ui->listWidget->count(); i++) {
        if(ui->listWidget->item(i)->text()==user)
            return ui->listWidget->takeItem(i)->text();
    }
    //TODO: see how to manage exceptions, how to distinguish them
    throw ("Users isn't active on file, user not found");
}

void Editor::closeEvent(QCloseEvent *event) {
    QString user;
    user = _workerClient->getUser();
    _workerClient->userLeft(this->windowTitle(), user);
}

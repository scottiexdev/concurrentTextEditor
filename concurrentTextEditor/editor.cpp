#include "editor.h"
#include "ui_editor.h"

Editor::Editor(QWidget *parent, WorkerClient *worker, QString fileName) :
    QMainWindow(parent),
    _workerClient(worker),
    ui(new Ui::Editor)
{
    ui->setupUi(this);
    this->setWindowTitle(fileName);
    connect(_workerClient, &WorkerClient::showFileLine, this, &Editor::showFileLine);
    _workerClient->requestFile(fileName);
}

Editor::~Editor()
{
    delete ui;
}

void Editor::showFileLine(QString buf) {
    ui->textEdit->append(buf);
}

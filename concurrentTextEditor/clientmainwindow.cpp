#include "clientmainwindow.h"
#include "ui_clientmainwindow.h"

clientmainwindow::clientmainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::clientmainwindow)
{
    ui->setupUi(this);
    _workerClient = new WorkerClient(this);
    connect(_workerClient, &WorkerClient::myLoggedIn, this, &clientmainwindow::myLoggedIn);
    connect(_workerClient, &WorkerClient::mySignupOk, this, &clientmainwindow::mySignupOk);
    connect(_workerClient, &WorkerClient::disconnectClient, this, &clientmainwindow::disconnectClient);

}

clientmainwindow::~clientmainwindow()
{
    delete ui;
}

void clientmainwindow::disconnectClient() {
    QMessageBox::warning(this, "Connection Error", "There was a problem with the server, click OK to return to the login window");
    hli->deleteLater();
    _workerClient->disconnectFromServer();
    this->show();
}

void clientmainwindow::on_pushButtonLogin_clicked()
{
    ui->pushButtonLogin->setEnabled(false);
    ui->pushButtonLogin->setText("Connecting...");
    ui->pushButtonLogin->repaint();

    if(!_workerClient->connectToServer(QHostAddress::LocalHost, 1967)) {
        QMessageBox::warning(this, "Error", "Server is not responding");
        ui->pushButtonLogin->setEnabled(true);
        ui->pushButtonLogin->setText("Login");
        return;
    }

    //get login credentials to make query to db
    QString usr = ui->lineEditUsr->text();
    QString pwd = ui->lineEditPwd->text();

    //create a json message with credentials for login
    QJsonObject cred;

    cred["type"] = "login";
    cred["username"] = usr;
    cred["password"] = pwd;

    _workerClient->sendLoginCred(cred);
    ui->pushButtonLogin->setEnabled(true);
    ui->pushButtonLogin->setText("Login");
}

void clientmainwindow::on_pushButtonSignup_clicked()
{
    d = new dialogsignup(this, _workerClient);
    d->show();
}

void clientmainwindow::myLoggedIn() {
    hli = new loggedinmainwindow(this, _workerClient);
    connect(_workerClient, &WorkerClient::showFiles, hli, &loggedinmainwindow::showFiles);void showFiles(QStringList list);
    hide();
    hli->show();
    hli->requestFileList();
}

void clientmainwindow::mySignupOk(){
    hli = new loggedinmainwindow(this, _workerClient);
    connect(_workerClient, &WorkerClient::showFiles, hli, &loggedinmainwindow::showFiles);void showFiles(QStringList list);
    d->close();
    hide();
    hli->show();
    hli->requestFileList();
}

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
}

clientmainwindow::~clientmainwindow()
{
    delete ui;
}


void clientmainwindow::on_pushButtonLogin_clicked()
{
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

//    connect(_workerClient, &WorkerClient::myLoggedIn, this, &clientmainwindow::myLoggedIn);

    //get login credentials to make query to db
    QString usr = ui->lineEditUsr->text();
    QString pwd = ui->lineEditPwd->text();

    //create a json message with credentials for login
    QJsonObject cred;

    cred["type"] = "login";
    cred["username"] = usr;
    cred["password"] = pwd;

    _workerClient->sendLoginCred(cred);
//    _workerClient->getFileList();
}

void clientmainwindow::on_pushButtonSignup_clicked()
{
    d = new dialogsignup(this, _workerClient);
    d->show();
}

void clientmainwindow::myLoggedIn() {
    hli = new homeLoggedIn(this, _workerClient);
    hide();
    hli->show();
    hli->requestFileList();
}

void clientmainwindow::mySignupOk(){
    hli = new homeLoggedIn(this, _workerClient);
    d->close();
    hide();
    hli->show();
    hli->requestFileList();
}

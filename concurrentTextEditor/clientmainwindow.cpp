#include "clientmainwindow.h"
#include "ui_clientmainwindow.h"

clientmainwindow::clientmainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::clientmainwindow)
{
    ui->setupUi(this);
}

clientmainwindow::~clientmainwindow()
{
    delete ui;
}


void clientmainwindow::on_pushButtonLogin_clicked()
{
    _workerClient = new WorkerClient(this);
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

    connect(_workerClient, &WorkerClient::myLoggedIn, this, &clientmainwindow::myLoggedIn);

    //get login credentials to make query to db
    QString usr = ui->lineEditUsr->text();
    QString pwd = ui->lineEditPwd->text();

    _loggedUser = usr;

    //create a json message with credentials for login
    QJsonObject cred;

    cred["type"] = "login";
    cred["username"] = usr;
    cred["password"] = pwd;

    _workerClient->sendLoginCred(cred);
    _workerClient->getFileList();
}

void clientmainwindow::on_pushButtonSignup_clicked()
{
    d = new dialogsignup(this);
    d->exec();
}

void clientmainwindow::myLoggedIn(QString loggedUser) {
    hli = new homeLoggedIn(this, loggedUser,  _workerClient);
    hide();
    hli->exec();
}

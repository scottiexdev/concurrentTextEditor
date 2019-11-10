#include "clientmainwindow.h"
#include "ui_clientmainwindow.h"

QString user;

clientmainwindow::clientmainwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::clientmainwindow)
{
    ui->setupUi(this);
<<<<<<< Updated upstream
    connect(_workerClient, &WorkerClient::myLoggedIn, this, &clientmainwindow::myLoggedIn);
=======
    _workerClient = new WorkerClient(this);
>>>>>>> Stashed changes
}

clientmainwindow::~clientmainwindow()
{
    delete ui;
}


void clientmainwindow::on_pushButtonLogin_clicked()
{
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

<<<<<<< Updated upstream
=======
    //connecting signals and slots
    connect(_workerClient, &WorkerClient::myLoggedIn, this, &clientmainwindow::myLoggedIn);
>>>>>>> Stashed changes
    //get login credentials to make query to db
    QString usr = ui->lineEditUsr->text();
    QString pwd = ui->lineEditPwd->text();

    user = usr;

    //create a json message with credentials for login
    QJsonObject cred;

    cred["type"] = "login";
    cred["username"] = usr;
    cred["password"] = pwd;


    //make query and update bool accordingly
    _workerClient->SendLoginCred(cred);

//    credentialok = workerClient->receiveLoginResult();

//    if(!credentialok){
//        QMessageBox nok;
//        nok.setText("Invalid Username or Password.");
//        nok.exec();
//    }

//    if(credentialok) {
//        hli = new homeLoggedIn(this, usr, _workerClient);
//        hide();
//        hli->exec();
//    }
}

void clientmainwindow::on_pushButtonSignup_clicked()
{
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

    d = new dialogsignup(this, _workerClient);

    connect(d, &dialogsignup::logIn, this, &clientmainwindow::myLoggedIn);

    d->exec();
}

void clientmainwindow::myLoggedIn() {
    hli = new homeLoggedIn(this, user, _workerClient);
    hide();
    hli->exec();
}

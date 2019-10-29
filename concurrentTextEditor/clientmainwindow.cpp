#include "clientmainwindow.h"
#include "ui_clientmainwindow.h"
#include "workerclient.h"
#include <QDataStream>

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
    //Socket to server - workerClient
        WorkerClient* workerClient = new WorkerClient(this);
        workerClient->connectToServer(QHostAddress::LocalHost, 1967);

        //get login credentials to make query to db
        QString usr = ui->lineEditUsr->text();
        QString pwd = ui->lineEditPwd->text();

        bool credentialok = false;


        //create a json message with credentials for login
        QJsonObject cred;

        cred["type"] = "login";
        cred["username"] = usr;
        cred["password"] = pwd;

        workerClient->SendLoginCred(cred);
        credentialok = workerClient->receiveLoginResult();

        if(!credentialok){
            QMessageBox nok;
            nok.setText("Invalid Username or Password.");
            nok.exec();
        }

        if(credentialok) {
            hli = new homeLoggedIn(this, usr, workerClient);
            hide();
            hli->exec();
        }
}

void clientmainwindow::on_pushButtonSignup_clicked()
{
    d = new dialogsignup(this);
    d->exec();
}

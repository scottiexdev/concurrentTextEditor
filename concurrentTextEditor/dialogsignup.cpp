#include "dialogsignup.h"
#include "ui_dialogsignup.h"
#include "workerclient.h"
#include <QHostAddress>

QString usr_temp;

dialogsignup::dialogsignup(QWidget *parent, WorkerClient *worker) :
    QDialog(parent),
    ui(new Ui::dialogsignup),
    _workerClient(worker)
{
    ui->setupUi(this);
}

dialogsignup::~dialogsignup()
{
    delete ui;
}

void dialogsignup::on_pushButton_clicked()
{
    connect(_workerClient, &WorkerClient::mySignupOk, this, &dialogsignup::mySignupOk);
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

    QString usr = ui->lineEdit_Usr->text();
    QString pwd1 = ui->lineEdit_PwdIns->text();
    QString pwd2 = ui->lineEdit_PwdConf->text();

    if(pwd1 != pwd2){
        QMessageBox pwd_not_eq;
        pwd_not_eq.setText("Passwords do not match.");
        pwd_not_eq.exec();
    }

    QJsonObject signup;

    signup["type"] = "signup";
    signup["username"] = usr;
    signup["password"] = pwd1;

    usr_temp = usr;

    _workerClient->sendLoginCred(signup);

    //TODO signup corretto
}

//used usr_temp since signedUser is a void string - TO FIX
void dialogsignup::mySignupOk(QString signedUser){
    emit logIn(usr_temp);
}

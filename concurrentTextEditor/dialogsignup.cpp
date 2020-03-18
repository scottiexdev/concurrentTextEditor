#include "dialogsignup.h"
#include "ui_dialogsignup.h"
#include "workerclient.h"
#include <QHostAddress>

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
    bool ok=false; //variable needed to handle different pwd
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

    QString usr = ui->lineEdit_Usr->text();
    QString pwd1 = ui->lineEdit_PwdIns->text();
    QString pwd2 = ui->lineEdit_PwdConf->text();

    if(pwd1 != pwd2){
        QMessageBox pwd_not_eq;
        pwd_not_eq.setText("Passwords do not match.");
        pwd_not_eq.exec();
    } else ok = true;

    QJsonObject signup;

    signup["type"] = "signup";
    signup["username"] = usr;
    signup["password"] = pwd1;

    if(ok)
        _workerClient->sendLoginCred(signup);
}



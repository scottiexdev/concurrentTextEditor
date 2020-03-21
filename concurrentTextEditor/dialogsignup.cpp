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

void dialogsignup::on_pushButton_Signup_clicked()
{
    bool ok=false, ok1=false; //variable needed to handle different pwd
    // Regex ok ma accetta anche robe del tipo abc@def@pippo.it => to fix
    QRegularExpression regex("(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    _workerClient->connectToServer(QHostAddress::LocalHost, 1967);

    QString usr = ui->lineEdit_Usr->text();
    QString pwd1 = ui->lineEdit_PwdIns->text();
    QString pwd2 = ui->lineEdit_PwdConf->text();
    QString email = ui->lineEdit_email->text();

    if(!regex.match(email).hasMatch()){
        QMessageBox email_nok;
        email_nok.setText("The email you enetered has not a valid email format");
        email_nok.exec();
    } else ok1=true;

    if(pwd1 != pwd2 || (pwd1.isEmpty()&&pwd2.isEmpty())){
        QMessageBox pwd_not_eq;
        pwd_not_eq.setText("Passwords do not match.");
        pwd_not_eq.exec();
    } else ok = true;

    QJsonObject signup;

    signup["type"] = "signup";
    signup["username"] = usr;
    signup["password"] = pwd1;
    signup["email"] = email;
    if(this->icn.isNull() || this->icn.isEmpty())
        signup["icon"] = this->_defaultIcon;
    else signup["icon"] = this->icn;

    if(ok && ok1) {
        _workerClient->sendLoginCred(signup);
        this->close();
    }
}



void dialogsignup::on_pushButton_Pic_clicked()
{
    QString iconpath = QFileDialog::getOpenFileName(this, tr("Profile Picture"), this->_defaultIconPath);
    this->icn = iconpath;
}

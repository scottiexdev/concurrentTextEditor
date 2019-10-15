#include "dialogsignup.h"
#include "ui_dialogsignup.h"

dialogsignup::dialogsignup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialogsignup)
{
    ui->setupUi(this);
}

dialogsignup::~dialogsignup()
{
    delete ui;
}

void dialogsignup::on_pushButton_clicked()
{
    std::string usr = ui->lineEdit_Usr->text().toUtf8().constData();
    std::string pwd1 = ui->lineEdit_PwdIns->text().toUtf8().constData();
    std::string pwd2 = ui->lineEdit_PwdConf->text().toUtf8().constData();

    if(pwd1 != pwd2){
        QMessageBox pwd_not_eq;
        pwd_not_eq.setText("Passwords do not match.");
        pwd_not_eq.exec();
    }
}

#include "homeloggedin.h"
#include "ui_homeloggedin.h"

homeLoggedIn::homeLoggedIn(QWidget *parent, std::string usrname) :
    QDialog(parent),
    ui(new Ui::homeLoggedIn)
{
    ui->setupUi(this);
    QString usr_to_show = QString::fromStdString(usrname);
    ui->welcomeLabel->setText("Welcome, "+usr_to_show); //used to show Username in home window
}

homeLoggedIn::~homeLoggedIn()
{
    delete ui;
}

void homeLoggedIn::on_pushButtonSettings_clicked()
{
    ac_st = new accountSettings(this);
    ac_st->exec();
}

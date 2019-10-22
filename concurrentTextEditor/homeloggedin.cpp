#include "homeloggedin.h"
#include "ui_homeloggedin.h"

homeLoggedIn::homeLoggedIn(QWidget *parent, QString usrname) :
    QDialog(parent),
    ui(new Ui::homeLoggedIn)
{
    ui->setupUi(this);
    ui->welcomeLabel->setText("Welcome, "+usrname); //used to show Username in home window
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

void homeLoggedIn::on_pushButtonLogout_clicked()
{
    QApplication::quit();
}

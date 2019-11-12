#include "homeloggedin.h"
#include "ui_homeloggedin.h"
#include "workerclient.h"

homeLoggedIn::homeLoggedIn(QWidget *parent, WorkerClient* worker) :
    QDialog(parent),
    ui(new Ui::homeLoggedIn),
    _workerClient(worker)    
{
    ui->setupUi(this);    
    _workerClient->getFileList();
    ui->welcomeLabel->setText("Welcome, "+ _workerClient->getUser()); //used to show Username in home window
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

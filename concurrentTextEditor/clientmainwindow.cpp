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
    //get login credentials to make query to db
    std::string usr = ui->lineEditUsr->text().toUtf8().constData();
    std::string pwd = ui->lineEditPwd->text().toUtf8().constData();

    bool credentialok = false;
    credentialok = true;
    //make query and update bool accordingly

    if(!credentialok){
        QMessageBox nok;
        nok.setText("Invalid Username or Password.");
        nok.exec();
    }

    if(credentialok) {
        hli = new homeLoggedIn(this, usr);
        hide();
        hli->exec();
    }
}

void clientmainwindow::on_pushButtonSignup_clicked()
{
    d = new dialogsignup(this);
    d->exec();
}

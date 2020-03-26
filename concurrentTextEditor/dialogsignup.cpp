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
    else {

        signup["icon"] = _defaultIconPath+this->icn.split("/").last();
    }

    if(ok && ok1) {
        _workerClient->sendLoginCred(signup);
        // prendo pixmap
        QPixmap pm(this->icn);
        QString form = this->icn.split(".").last().toUpper();
        QByteArray buf = form.toLocal8Bit();
        const char * format = buf.data(); //formato necessario pr salvare immagine

        // inserisco immagine in json
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        pm.save(&buffer, format);
        auto ba = buffer.data().toBase64();
        QLatin1String img = QLatin1String(ba);


        // popolo json
        QJsonObject propic;
        propic["username"] = usr;
        propic["type"] = messageType::edit;
        propic["editType"] = EditType::propic;
        propic["image"] = img;
        propic["filename"] = this->icn.split("/").last();

        _workerClient->setIcon(pm);
        _workerClient->changeProPic(propic); // la cosa che deve fare è la stessa di quando cambi

        //TODO: fix problema che se faccio signup e poi account settings non fa vedere propric, se slogghi e rilogghi si
        this->close();
    }
}


void dialogsignup::on_pushButton_Pic_clicked()
{
    QString iconpath = QFileDialog::getOpenFileName(this, tr("Profile Picture"), this->_defaultIconPath);
    this->icn = iconpath;
}

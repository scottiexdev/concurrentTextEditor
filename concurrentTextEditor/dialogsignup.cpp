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
    QRegularExpression regex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);

    bool success = false;
    if(LOCAL_HOST)
        success = _workerClient->connectToServer(QHostAddress::LocalHost, 8888);
    else
        success = _workerClient->connectToServer(QHostAddress("109.115.20.249"), 8888);

    if(!success) {
        QMessageBox::warning(this, "Error", "Server is not responding");
        return;
    }

    QString usr = ui->lineEdit_Usr->text();
    QString pwd1 = ui->lineEdit_PwdIns->text();
    QString pwd2 = ui->lineEdit_PwdConf->text();
    QString email = ui->lineEdit_email->text();

    if(!regex.match(email).hasMatch()){
        QMessageBox email_nok;
        email_nok.setText("The email you entered has not a valid email format");
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
    if(this->icn.isNull() || this->icn.isEmpty()){
        QMessageBox::information(this, "Notice", "It seems you did not choose any valid profile picture, in this way the default one will be assigned to you.");
        signup["icon"] = "default";
    }
    else {
        QPixmap qpm(icn);
        _workerClient->setIcon(qpm);
        signup["icon"] = icn.split("/").last();

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
        propic["type"] = QString("saveicon");
        propic["image"] = img;
        propic["filename"] = this->icn.split("/").last();

         _workerClient->saveIcon(propic);
    }

    if(ok && ok1) {
        _workerClient->sendLoginCred(signup);
        this->close();
    }
}


void dialogsignup::on_pushButton_Pic_clicked()
{
    QString iconpath = QFileDialog::getOpenFileName(this, tr("Profile Picture"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    //i briefly check if the file chosen is valid & supported
    QPixmap pm (iconpath);
    QString form = iconpath.split(".").last().toUpper();
    QByteArray buf = form.toLocal8Bit();
    const char * format = buf.data();

    // inserisco immagine in json
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    pm.save(&buffer, format);
    auto ba = buffer.data().toBase64();
    QLatin1String img = QLatin1String(ba);

    if (img.isNull() || img.isEmpty() || img == "") {
        QMessageBox::information(this, "Error", "Something is wrong with the image. Please try to use a different one.");
    } else {
        this->icn = iconpath;
        QMessageBox::information(this, "Success", "Picture set!");
    }
}

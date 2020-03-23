#include "accountsettings.h"
#include "ui_accountsettings.h"



accountSettings::accountSettings(QWidget *parent, WorkerClient *worker) :
    QDialog(parent),
    ui(new Ui::accountSettings),
    _worker(worker)
{
    _worker->connectToServer(QHostAddress::LocalHost, 1967);
    ui->setupUi(this);
    ui->label_usr->setText("Username: "+worker->getUser());
    //la QPixMap deve prendere il path salvato sul server per l'user speicfico (necessaria query al db)
    QPixmap pm(_defaultIcon);
    ui->img_label->setPixmap(pm);
    ui->img_label->setScaledContents(true);
}

accountSettings::~accountSettings()
{
    // _worker.disconnectFromServer();
    delete ui;
}

void accountSettings::on_pushButton_U_clicked()
{
    // Open Q Dialog con QlineEdit - OK
    // Get username - OK
    // Query per scoprire se quello nuovo è già esistente - OK
    // Risposta OK v NOK -
    // Update -

    QString new_usn = QInputDialog::getText(this, tr("Change Username"),
                                             tr("New username:"), QLineEdit::Normal);
    if(new_usn.isNull() || new_usn.isEmpty())
        new_usn = _worker->getUser();
    QJsonObject user;
    user["username"] = _worker->getUser();
    user["type"] = messageType::edit;
    user["editType"] = EditType::username;
    user["new_usn"] = new_usn;

    _worker->newUsername(user);

}

void accountSettings::on_pushButton_EA_clicked()
{
    QRegularExpression regex("(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    bool ok;
    QString new_email = QInputDialog::getText(this, tr("Change Email"), tr("New Email"), QLineEdit::Normal, tr("email@domain.com"), &ok);

    if(ok){
        if(!regex.match(new_email).hasMatch()) {
            QMessageBox::information(this, tr("Error"),tr("new email has not a valid format"));
            return;
        }
    }
}

void accountSettings::on_pushButton_PP_clicked()
{
    // connect

    QString newicon_filepath = QFileDialog::getOpenFileName(this, tr("New Profile Picture"), this->_defaultIconPath);
    QPixmap pm(newicon_filepath);
    ui->img_label->setPixmap(pm); //TODO: update questo con query al db appena funziona tutto
    ui->img_label->setScaledContents(true);

    // inserisco immagine in json
    QBuffer buffer;
    QByteArray arr;
    QImage image = pm.toImage();
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer);
    QLatin1String img = QLatin1String(buffer.data().toBase64());

    // TODO: QImage dentro un json e duale Propichandler su server.cpp


    // popolo json
    QJsonObject propic;
    propic["username"] = _worker->getUser();
    propic["type"] = messageType::edit;
    propic["editType"] = EditType::propic;
    propic["image"] = img;
    propic["filename"] = newicon_filepath.split("/").last();

    _worker->changeProPic(propic);

    // TODO: prendere immagine X, ficcarla in un json X, mandarla al server X, fargliela salvare nel suo path di Icons. Crea un nuovo messageType in Enums.h X

}

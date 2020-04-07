#include "accountsettings.h"
#include "ui_accountsettings.h"



accountSettings::accountSettings(QWidget *parent, WorkerClient *worker) :
    QDialog(parent),
    ui(new Ui::accountSettings),
    _worker(worker)
{
    ui->setupUi(this);
    ui->label_usr->setText("Username: "+worker->getUser());
    ui->img_label->setPixmap(_worker->getUserIcon());
    ui->img_label->setScaledContents(true);

    connect(_worker, &WorkerClient::newUsernameOk, this, &accountSettings::newUsernameOk);
    connect(_worker, &WorkerClient::newUsernameNok, this, &accountSettings::newUsernameNok);
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
    // Risposta OK v NOK basata su availability new user - OK
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
    QString newicon_filepath = QFileDialog::getOpenFileName(this, tr("New Profile Picture"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    //entra nell'if solo nel caso l'utente scelga un'immagine
    if(!newicon_filepath.isNull() || !newicon_filepath.isEmpty()){
        QPixmap pm(newicon_filepath);
        _worker->setIcon(_defaultIconPath+newicon_filepath.split("/").last());

        QString form = newicon_filepath.split(".").last().toUpper();
        QByteArray buf = form.toLocal8Bit();
        const char * format = buf.data();

        // inserisco immagine in json
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        pm.save(&buffer, format);
        auto ba = buffer.data().toBase64();
        QLatin1String img = QLatin1String(ba);


        // popolo json
        QJsonObject propic;
        propic["username"] = _worker->getUser();
        propic["type"] = messageType::edit;
        propic["editType"] = EditType::propic;
        propic["image"] = img;
        propic["filename"] = newicon_filepath.split("/").last();

        if (img.isNull() || img.isEmpty() || img == "") {
            QMessageBox::information(this, "Error", "Something is wrong with the image. Please try to use a different one.");
        } else {
            _worker->changeProPic(propic);

            ui->img_label->setPixmap(pm);
            ui->img_label->setScaledContents(true);
        }
    }
}

void accountSettings::on_pushButton_PWD_clicked()
{
    //TODO: aprire un Qdialog
    // inserire password precedente, nuova password e conferma nuova password
    // sul server: query ad DB, check e json in risposta

}

void accountSettings::newUsernameNok(){
    QMessageBox::information(this, "Error", "The username you chose is already taken. Please choose another.");
}

void accountSettings::newUsernameOk(){
    this->ui->label_usr->setText("Username: "+_worker->getUser());
    QMessageBox::information(this, "Success", "Username changed successfully!");
}

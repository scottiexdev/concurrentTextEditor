#include "accountsettings.h"
#include "ui_accountsettings.h"



accountSettings::accountSettings(QWidget *parent, WorkerClient *worker) :
    QDialog(parent),
    ui(new Ui::accountSettings)
{
    ui->setupUi(this);
    ui->label_usr->setText("Username: "+worker->getUser());
    //la QPixMap deve prendere il path salvato sul server per l'user speicfico (necessaria query al db)
    QPixmap pm(_defaultIcon);
    ui->img_label->setPixmap(pm);
    ui->img_label->setScaledContents(true);
}

accountSettings::~accountSettings()
{
    delete ui;
}

void accountSettings::on_pushButton_U_clicked()
{
    // Open Q Dialog con QlineEdit - OK
    // Get username - OK
    // Query per scoprire se quello nuovo è già esistente -
    // Risposta OK v NOK -
    // Update -

    QString new_usn = QInputDialog::getText(this, tr("Change Username"),
                                             tr("New username:"), QLineEdit::Normal);

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
    QString newicon_filepath = QFileDialog::getOpenFileName(this, tr("New Profile Picture"), this->_defaultIconPath);
    QPixmap pm(newicon_filepath);
    ui->img_label->setPixmap(pm);
    ui->img_label->setScaledContents(true);
    // TODO: prendere immagine, ficcarla in un json, mandarla al server, fargliela salvare nel suo path di Icons. Crea un nuovo messageType in Enums.h

}

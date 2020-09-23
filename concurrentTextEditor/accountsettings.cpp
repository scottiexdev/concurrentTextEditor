#include "accountsettings.h"
#include "ui_accountsettings.h"



accountSettings::accountSettings(QWidget *parent, WorkerClient *worker) :
    QDialog(parent),
    ui(new Ui::accountSettings),
    _worker(worker)
{
    ui->setupUi(this);
    ui->label_usr->setText(worker->getUser());
    ui->label_email->setText(worker->getEmail());
    ui->img_label->setPixmap(_worker->getUserIcon());
    ui->img_label->setScaledContents(true);

    connect(_worker, &WorkerClient::newEmailOk, this, &accountSettings::emailChanged);
    connect(_worker, &WorkerClient::newPwdOk, this, &accountSettings::newPWdOk);
    connect(_worker, &WorkerClient::newUsernameOk, this, &accountSettings::newUsernameOk);
    connect(_worker, &WorkerClient::newUsernameNok, this, &accountSettings::newUsernameNok);
    connect(_worker, &WorkerClient::iconSent, this, &accountSettings::iconArrived);
}

accountSettings::~accountSettings()
{
    disconnect(_worker, &WorkerClient::newUsernameOk, this, &accountSettings::newUsernameOk);
    disconnect(_worker, &WorkerClient::newUsernameNok, this, &accountSettings::newUsernameNok);
    disconnect(_worker, &WorkerClient::iconSent, this, &accountSettings::iconArrived);
    delete ui;
}

void accountSettings::on_pushButton_U_clicked()
{

    QString new_usn = QInputDialog::getText(this, tr("Change Username"),
                                             tr("New username:"), QLineEdit::Normal);
    if(new_usn.isNull() || new_usn.isEmpty())
        return;
    QJsonObject user;
    user["username"] = _worker->getUser();
    user["type"] = messageType::edit;
    user["editType"] = EditType::username;
    user["new_usn"] = new_usn;

    _worker->newUsername(user);

}

void accountSettings::on_pushButton_EA_clicked()
{
    QRegularExpression regex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
    bool ok;
    QString new_email = QInputDialog::getText(this, tr("Change Email"), tr("New Email"), QLineEdit::Normal, tr("email@domain.com"), &ok);

    if(ok){
        if(!regex.match(new_email).hasMatch()) {
            QMessageBox::information(this, tr("Error"),tr("new email has not a valid format"));
            return;
        } else _worker->setNewEmail(new_email);
    }
}

void accountSettings::on_pushButton_PP_clicked()
{
    QString newicon_filepath = QFileDialog::getOpenFileName(this, tr("New Profile Picture"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    //entra nell'if solo nel caso l'utente scelga un'immagine
    if(!newicon_filepath.isNull() || !newicon_filepath.isEmpty()){
        QPixmap pm(newicon_filepath);

        QByteArray ba = _worker->getLatinStringFromImg(newicon_filepath);
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

    QDialog dialog(this);
    QFormLayout form(&dialog);

    QString p1, p2;

    form.addRow(new QLabel("Please enter your new password"));
    QLineEdit *pwd1 = new QLineEdit(&dialog);
    pwd1->setEchoMode(QLineEdit::Password);
    form.addRow(pwd1);

    form.addRow(new QLabel("Please Re-enter your new password"));

    QLineEdit *pwd2 = new QLineEdit(&dialog);
    pwd2->setEchoMode(QLineEdit::Password);
    form.addRow(pwd2);

    p1 = pwd1->text();
    p2 = pwd2->text();

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    dialog.exec();

    p1 = pwd1->text();
    p2 = pwd2->text();

    if(p1 != p2 || p1.isNull() || p2.isNull() || p1.isEmpty() || p2.isEmpty()) {
        QMessageBox::information(this, "Error", "Passwords do not match or are empty");
    } else _worker->setNewPassowrd(p1);
}

void accountSettings::newUsernameNok(){
    QMessageBox::information(this, "Error", "The username you chose is already taken. Please choose another.");
}

void accountSettings::newUsernameOk(){
    this->ui->label_usr->setText("Username: "+_worker->getUser());
    QMessageBox::information(this, "Success", "Username changed successfully!");
}

void accountSettings::newPWdOk(){
    QMessageBox::information(this, "Success", "Password changed successfully!");
}

void accountSettings::closeEvent(QCloseEvent *event){
    this->deleteLater();
}

void accountSettings::iconArrived(QPixmap icon){
    QPixmap target = QPixmap(size());
    target.fill(Qt::transparent);
    //QPixmap icon_scaled = QPixmap(icon).scaled(270, 270, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path = QPainterPath();
    path.addRoundedRect(270, 270, 270, 270, 100, 100);
    painter.setClipPath(path);
    painter.drawPixmap(270, 270, icon);

    ui->img_label->setPixmap(icon);
    ui->img_label->setScaledContents(true);
}

void accountSettings::emailChanged(){
    this->ui->label_email->setText("Email: "+_worker->getEmail());
    QMessageBox::information(this, "Success", "Email changed successfully!");
}

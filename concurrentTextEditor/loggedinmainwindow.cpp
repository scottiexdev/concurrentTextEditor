#include "loggedinmainwindow.h"
#include "ui_loggedinmainwindow.h"

loggedinmainwindow::loggedinmainwindow(QWidget *parent, WorkerClient* worker) :
    QMainWindow(parent),
    ui(new Ui::loggedinmainwindow),
    _workerClient(worker)
{
    ui->setupUi(this);
    ui->welcomeLabel->setText("Welcome, "+ _workerClient->getUser()); //used to show Username in home window
    connect(_workerClient, &WorkerClient::genericError, this, &loggedinmainwindow::errorDisplay);
    connect(_workerClient, &WorkerClient::ifFileOpenOk, this, &loggedinmainwindow::isFileOpenOkay);
}

loggedinmainwindow::~loggedinmainwindow()
{
    delete ui;
}

void loggedinmainwindow::requestFileList(){
    _workerClient->getFileList("all");
}

void loggedinmainwindow::showFiles(QStringList filesList, QStringList createdList, QStringList ownerList, bool isPublic){

    QTableWidget* table = isPublic ? ui->PublicFileListTable : ui->PrivatefileListTable;

    // Empty list
    table->clear();

    // Repopulate list
    table->setRowCount(filesList.count());
    table->setColumnCount(2);

    QStringList headers;
    headers.push_back("Filename");
    headers.push_back("Created");
    //headers.push_back("Owner"); #da risolvere la questione dell'owner
    table->setHorizontalHeaderLabels(headers);

    int cnt = 0;

    foreach(auto file, filesList){
        table->setItem(cnt, 0, new QTableWidgetItem(filesList.at(cnt)));
        // ^ changed filesList.first() into filesList.at(cnt) to get right item and not always the first one repeated
        table->setItem(cnt, 1, new QTableWidgetItem(createdList.at(cnt)));
        //ui->fileListTable->setItem(cnt, 2, new QTableWidgetItem(ownerList.at(cnt)));
        cnt++;
    }

}

void loggedinmainwindow::errorDisplay(QString str){
    QMessageBox::information(this, tr("Error"), str);
}

void loggedinmainwindow::on_pushButtonLogout_2_clicked()
{
    // TO FIX THIS: disconnect socket
    //this->_workerClient->disconnectHost();

    this->close();
    this->parentWidget()->show();
    this->deleteLater();
    //QApplication::quit();
}

void loggedinmainwindow::on_pushButtonOpenFile_2_clicked()
{
    //Da rivedere: interfaccia per aprire va cambiata con FILE CONDIVISI e FILE PRIVATI
    //Controllo che un file sia selezionato
    //Cambiare anche come viene preso il nome del file: click su data deve selezionare tutta la riga
    //E prendere il primo campo (filename)
    if(ui->PublicFileListTable->selectedItems().isEmpty()) {
        errorDisplay("Please select a file by clicking on it.");
        return;
    }

    QString fileName = ui->PublicFileListTable->selectedItems().first()->text();
    //_workerClient->requestFile(fileName);

    // Detect if private or public
    _e = new Editor(this, _workerClient, fileName, true);
    //hide();
    _e->show();
}

void loggedinmainwindow::on_pushButtonUpdate_2_clicked()
{
    this->
    _workerClient->getFileList("all");
}

// New PUBLIC file button
void loggedinmainwindow::on_pushButtonNewFile_2_clicked()
{
   newFile(true);
}

// New PUBLIC file button
void loggedinmainwindow::on_pushButtonNewFile_3_clicked()
{
   newFile(false);
}

void loggedinmainwindow::newFile(bool isPublic){

    bool ok = false;
    QJsonObject filename_req;

    // get filename from input dialog
    QString fileName = QInputDialog::getText(this, "New File", "Please insert new filename: ", QLineEdit::Normal, QString("FileName"), &ok);

    if(!fileName.isEmpty() && ok) {
        filename_req["type"] = messageType::newFile;
        filename_req["filename"] = fileName.append(".cte");
        filename_req["access"] = isPublic;
        _workerClient->newFileRequest(filename_req);

        // Spawn editor -WRONG spawn editor on new file received
        _e = new Editor(this, _workerClient, fileName, isPublic);
        _e->show();
    }
    else if(ok){
        errorDisplay("Insert a name for the file");


    }
}


void loggedinmainwindow::on_PublicFileListTable_cellDoubleClicked(int row, int column)
{
    QString fileName = ui->PublicFileListTable->item(row, column)->text();
    //_workerClient->requestFile(fileName);

    // Detect if private or public
    _e = new Editor(this, _workerClient, fileName, true);
    //hide();
    _e->show();
}



void loggedinmainwindow::on_PrivatefileListTable_cellDoubleClicked(int row, int column)
{
    QString fileName = ui->PrivatefileListTable->item(row, column)->text();

    // ADD HERE MESSAGE TO ASK SERVER IF FILE IS AVAILABLE

    // Detect if private or public
    _e = new Editor(this, _workerClient, fileName, false);
    //hide();
    _e->show();
}

void loggedinmainwindow::on_pushButtonInvite_2_clicked()
{
    // Invite makes sense only if a selected file in PrivateFilesListTable is selected
    if(ui->PrivatefileListTable->selectedItems().isEmpty()) {
        errorDisplay("Can't generate invite link. Please select a file by clicking on it.");
        return;
    }

    // Generate link for selected file - TODO: change this
    QString fileName = ui->PrivatefileListTable->selectedItems().first()->text();

    QString link = generateInviteLink(fileName, _workerClient->getUser());

    // Change this into a window that displays the link and has a button "copy link to clipboard"
    // that copies the link to clipboard for the user and notifies it that the operation was successful
    QClipboard* clip = QGuiApplication::clipboard();
    clip->setText(link);

    QInputDialog::getText(this, "Sharing link", "Generated link for file: " + fileName + " and copied to clipboard.", QLineEdit::Normal, link);
}

QString loggedinmainwindow::generateInviteLink(QString fileName, QString username){

    QJsonObject inviteLink;

    QString link = QUuid::createUuid().toString() + "/" + fileName + "/" + username;
    inviteLink["type"] = messageType::invite;
    inviteLink["user"] = username;
    inviteLink["link"] = link;
    inviteLink["operation"] = EditType::insertion;

    _workerClient->saveLinkToServer(inviteLink);

    return link;
}

void loggedinmainwindow::isFileOpenOkay(const QJsonObject& qjo){


}



void loggedinmainwindow::on_pushButtonDeleteFile_3_clicked()
{

    bool isPublic = false;
    QString fileName;

    if(ui->PublicFileListTable->selectedItems().isEmpty() && ui->PrivatefileListTable->selectedItems().isEmpty()) {
        errorDisplay("Please select a file by clicking on it.");
        return;
    }
    int ret = QMessageBox::warning(this, tr("Delete File"),
                                   tr("The file will be permanentely deleted, do you want to proceed?"),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret==QMessageBox::Yes)
    {
        if(!ui->PublicFileListTable->selectedItems().isEmpty()) {
            fileName = ui->PublicFileListTable->selectedItems().first()->text();
            isPublic = true;
        }
        else if (!ui->PrivatefileListTable->selectedItems().isEmpty()){
            fileName = ui->PrivatefileListTable->selectedItems().first()->text();
            isPublic = false;
        }
        _workerClient->deleteFile(fileName, isPublic);
        this->on_pushButtonUpdate_2_clicked();
    }
}

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
}

loggedinmainwindow::~loggedinmainwindow()
{
    delete ui;
}

void loggedinmainwindow::requestFileList(){
    _workerClient->getFileList();
}

void loggedinmainwindow::showFiles(QStringList filesList, QStringList createdList, QStringList ownerList){

    ui->fileListTable->setRowCount(filesList.count());
    ui->fileListTable->setColumnCount(2);

    QStringList headers;
    headers.push_back("Filename");
    headers.push_back("Created");
    //headers.push_back("Owner"); #da risolvere la questione dell'owner
    ui->fileListTable->setHorizontalHeaderLabels(headers);

    int cnt = 0;

    foreach(auto file, filesList){
        ui->fileListTable->setItem(cnt, 0, new QTableWidgetItem(filesList.at(cnt)));
        // ^ changed filesList.first() into filesList.at(cnt) to get right item and not always the first one repeated
        ui->fileListTable->setItem(cnt, 1, new QTableWidgetItem(createdList.at(cnt)));
        //ui->fileListTable->setItem(cnt, 2, new QTableWidgetItem(ownerList.at(cnt)));
        cnt++;
    }

}

void loggedinmainwindow::errorDisplay(QString str){
    QMessageBox::information(this, tr("Error"), str);
}


void loggedinmainwindow::on_pushButtonNewFile_2_clicked()
{
    bool ok;
    QString fileName = QInputDialog::getText(this, "New File", "Please insert new filename: ", QLineEdit::Normal, QString("FileName"), &ok);

    QJsonObject filename_req;

    if(!fileName.isEmpty() && ok) {
        filename_req["type"] = messageType::newFile;
        filename_req["filename"] = fileName.append(".cte");
        _workerClient->newFileRequest(filename_req);
        _e = new Editor(this, _workerClient, fileName);
        _e->show();
    }
    else if(ok){
        errorDisplay("Insert a name for the file");
    }
}

void loggedinmainwindow::on_pushButtonLogout_2_clicked()
{
    QApplication::quit();
}

void loggedinmainwindow::on_pushButtonOpenFile_2_clicked()
{
    //Da rivedere: interfaccia per aprire va cambiata con FILE CONDIVISI e FILE PRIVATI
    //Controllo che un file sia selezionato
    //Cambiare anche come viene preso il nome del file: click su data deve selezionare tutta la riga
    //E prendere il primo campo (filename)
    if(ui->fileListTable->selectedItems().isEmpty()) {
        errorDisplay("Please select a file by clicking on it.");
        return;
    }
    QString fileName = ui->fileListTable->selectedItems().first()->text();
    //_workerClient->requestFile(fileName);
    _e = new Editor(this, _workerClient, fileName);
    //hide();
    _e->show();
}

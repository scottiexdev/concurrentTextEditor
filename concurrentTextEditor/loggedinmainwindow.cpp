#include "loggedinmainwindow.h"
#include "ui_loggedinmainwindow.h"

loggedinmainwindow::loggedinmainwindow(QWidget *parent, WorkerClient* worker) :
    QMainWindow(parent),
    ui(new Ui::loggedinmainwindow),
    _workerClient(worker)
{
    ui->setupUi(this);    
    ui->welcomeLabel->setText("Welcome, "+ _workerClient->getUser()); //used to show Username in home window
}

loggedinmainwindow::~loggedinmainwindow()
{
    delete ui;
}

void loggedinmainwindow::requestFileList(){
    _workerClient->getFileList();
}

void loggedinmainwindow::showFiles(QStringList filesList){

    ui->fileListTable->setRowCount(filesList.count());
    ui->fileListTable->setColumnCount(3);

    QStringList headers;
    headers.push_back("Filename");
    headers.push_back("Created");
    headers.push_back("Owner");
    ui->fileListTable->setHorizontalHeaderLabels(headers);

    int cnt = 0;

    foreach(auto file, filesList){
        ui->fileListTable->setItem(cnt, 0, new QTableWidgetItem(filesList.at(cnt)));
        // ^ changed filesList.first() into filesList.at(cnt) to get right item and not always the first one repeated
        cnt++;
    }

}


//void loggedinmainwindow::on_pushButtonOpenFile_2_clicked()
//{
//    auto fileName = ui->fileListTable->selectedItems().first();
//    _workerClient->requestFile(fileName);
//}


void loggedinmainwindow::on_pushButtonNewFile_2_clicked()
{
    QString fn = QInputDialog::getText(this, "New File", "Please insert new filename: ", QLineEdit::Normal);

    QJsonObject filename_req ;

    if(!fn.isEmpty()) {
        filename_req["type"] = "newFile";
        filename_req["filename"] = fn;
    }

    _workerClient->newFileRequest(filename_req);
}

void loggedinmainwindow::on_pushButtonLogout_2_clicked()
{
    QApplication::quit();
}

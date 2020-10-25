#include "editor.h"
#include "ui_editor.h"

#include <exception>

Editor::Editor(QWidget *parent, WorkerClient *worker, QString fileName, bool isPublic, bool shared) :
    QMainWindow(parent),
    ui(new Ui::Editor),
    _workerClient(worker)
{
    ui->setupUi(this);
    ui->editorController->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setWindowTitle(fileName);
    connect(_workerClient, &WorkerClient::handleFile, this, &Editor::handleFile);
    connect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    connect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    connect(ui->editorController, &EditorController::broadcastEditWorker, _workerClient, &WorkerClient::broadcastEditWorker);
    connect(_workerClient, &WorkerClient::handleRemoteEdit, ui->editorController, &EditorController::handleRemoteEdit);
    connect(_workerClient, &WorkerClient::fileDeleted, this, &Editor::fileDeleted);
    connect(ui->editorController, &EditorController::customContextMenuRequested, this, &Editor::customContextMenuRequested);

    _workerClient->requestFile(fileName, ui->editorController->getSiteID(), isPublic);

    //Prende lista degli utenti attivi su quel file
    _workerClient->requestUserList(fileName);
    if(shared){
        ui->editorController->setOwner(fileName.split("/")[0]);
        ui->editorController->setShared(shared);
    }
    //Notifica il server che l'utente si e' connesso a quel file
    _workerClient->userJoined(fileName, _workerClient->getUser());    
    ui->editorController->setAccess(isPublic);

    // Get Icons to show in GUI
    ui->actionCut->setIcon(_workerClient->getIcon(UiEditor::cut));
    ui->actionCopy->setIcon(_workerClient->getIcon(UiEditor::copy));
    ui->actionPaste->setIcon(_workerClient->getIcon(UiEditor::paste));
    ui->actionBold->setIcon(_workerClient->getIcon(UiEditor::bold1));
    ui->actionExport_PDF->setIcon(_workerClient->getIcon(UiEditor::pdf));
    ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined));
    ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1));

    _loadMsg.setText("Loading file, please wait...");
    _loadMsg.setStandardButtons(0);
    _loadMsg.show();
}

Editor::~Editor()
{
    disconnect(_workerClient, &WorkerClient::handleFile, this, &Editor::handleFile);
    disconnect(_workerClient, &WorkerClient::showUser, this, &Editor::showUser);
    disconnect(_workerClient, &WorkerClient::deleteUser, this, &Editor::deleteUser);
    disconnect(ui->editorController, &EditorController::broadcastEditWorker, _workerClient, &WorkerClient::broadcastEditWorker);
    disconnect(_workerClient, &WorkerClient::handleRemoteEdit, ui->editorController, &EditorController::handleRemoteEdit);
    disconnect(_workerClient, &WorkerClient::fileDeleted, this, &Editor::fileDeleted);
    disconnect(ui->editorController, &EditorController::customContextMenuRequested, this, &Editor::customContextMenuRequested);
    delete ui;
}

//Al posto di QString avremo il JSonDocument ricevuto, che corrisponde al file
void Editor::handleFile(QJsonDocument unparsedFile) {
    //Il document ricevuto viene passato a editorcontroller che lo converte il data structure
    //c++ e lo visualizza

    ui->editorController->parseCteFile(unparsedFile);

    ui->editorController->write();

    // delete here msg
    _loadMsg.hide();
}

void Editor::showUser(QString user) {
    QListWidgetItem *newUser = new QListWidgetItem(user);
    if(user == _workerClient->getUser()) {
        newUser->setBackground(QColor(Qt::white));
        ui->editorController->setUserColor(user, Qt::white);
    } else {
        QColor color = _colors.at(_colorNumber%_colors.size());
        newUser->setBackground(_colors.at(_colorNumber%_colors.size())); //creare una palette
        _colorNumber++;
        ui->editorController->setUserColor(user, color);
    }
    ui->listWidget->addItem(newUser);
    ui->activeUsers->setText("Active users: " + QString::number(ui->listWidget->count()));
}

QString Editor::deleteUser(QString user) {
    for(int i=0; i < ui->listWidget->count(); i++) {
        if(ui->listWidget->item(i)->text()==user) {
            ui->activeUsers->setText("Active users: " + QString::number(ui->listWidget->count()-1));
            return ui->listWidget->takeItem(i)->text();
        }
    }
}

void Editor::closeEvent(QCloseEvent *event) {
    QString user;
    user = _workerClient->getUser();
    _workerClient->userLeft(ui->editorController->getFileName(), user);
    this->deleteLater();
}

void Editor::fileDeleted() {
    QMessageBox::warning(this, tr("Warning"), tr("The file has been deleted"));
    this->close();
}


void Editor::on_actionExport_PDF_triggered()
{
    QString fileName = ui->editorController->getFileName().remove(".cte");

    QFileDialog *savePDF = new QFileDialog(parentWidget(),tr("Export PDF"), QDir::current().path(), tr("PDF files (*.pdf)"));
    savePDF->setFileMode(QFileDialog::AnyFile);
    savePDF->setAcceptMode(QFileDialog::AcceptSave);
    if(savePDF->exec() != QDialog::Accepted)
        return;
    QStringList pathList = savePDF->selectedFiles();
    QString path(pathList.join("\\"));
    if(path.isNull() || path.isEmpty()) {
        QMessageBox::warning(this, tr("Export PDF"), tr("Path not valid"));
        return;
    }
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(path);

    QTextDocument doc;
    ui->editorController->selectAll();
    ui->editorController->setTextBackgroundColor(Qt::white);
    doc.setHtml(ui->editorController->toHtml());
    doc.setPageSize(printer.pageRect().size()); // This is necessary if you want to hide the page number
    doc.print(&printer);
    QMessageBox::information(this, tr("Export PDF"), tr("PDF exported successfully!"));
}

void Editor::on_actionPaste_triggered(){

    QKeyEvent* pasteEvent  = new QKeyEvent(QEvent::KeyPress, Qt::Key_Paste, Qt::NoModifier);
    QCoreApplication::sendEvent(ui->editorController, pasteEvent);

}

void Editor::on_actionBold_triggered()
{
    setFormatUi(UiEditor::bold1);
    changeFormat(Format::bold);
}

void Editor::on_actionItalics_triggered()
{
    setFormatUi(UiEditor::italics1);
    changeFormat(Format::italics);
}

void Editor::on_actionUnderline_triggered()
{
    setFormatUi(UiEditor::underlined);
    changeFormat(Format::underline);
}

void Editor::changeFormat(Format format) {
    int anchor = ui->editorController->textCursor().anchor();
    QTextCursor temp = ui->editorController->textCursor();
    temp.setPosition(anchor);
    QPair<int, int> anchorPosition = QPair<int,int>(temp.blockNumber(),temp.positionInBlock());
    QPair<int,int> cursorPosition = QPair<int,int>(ui->editorController->textCursor().blockNumber(),
                                                   ui->editorController->textCursor().positionInBlock());
    ui->editorController->changeFormat(cursorPosition, anchorPosition, format);
}

void Editor::on_actionCopy_triggered()
{
    ui->editorController->copy();
}

void Editor::on_actionCut_triggered()
{
    QKeyEvent* cutEvent  = new QKeyEvent(QEvent::KeyPress, Qt::Key_Cut, Qt::NoModifier);
    QCoreApplication::sendEvent(ui->editorController, cutEvent);
    ui->editorController->cut();
}

void Editor::setFormatUi(UiEditor tag){
    switch (tag) {
        case UiEditor::bold1:
            if(!b) {ui->actionBold->setIcon(_workerClient->getIcon(UiEditor::boldSelected)); b = true;}
            else {ui->actionBold ->setIcon(_workerClient->getIcon(UiEditor::bold1)); b = false; }
            ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1)); i=false;
            ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined)); u=false;
            break;
        case UiEditor::italics1:
            if(!i) {ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italicsSelected)); i =true;}
            else {ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1)); i = false;}
            ui->actionBold ->setIcon(_workerClient->getIcon(UiEditor::bold1)); b=false;
            ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined)); u=false;
            break;
        case UiEditor::underlined:
            if(!u) {ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlinedSelected)); u=true;}
            else {ui->actionUnderline->setIcon(_workerClient->getIcon(UiEditor::underlined)); u = false;}
            ui->actionBold ->setIcon(_workerClient->getIcon(UiEditor::bold1)); b=false;
            ui->actionItalics->setIcon(_workerClient->getIcon(UiEditor::italics1)); i=false;
            break;
    }
}

void Editor::on_editorController_customContextMenuRequested(const QPoint &pos)
{
    QMenu editMenu;
    QAction *cut = editMenu.addAction("Cut");
    QAction *copy = editMenu.addAction("Copy");
    QAction *paste = editMenu.addAction("Paste");
    editMenu.addSeparator();
    QAction *exPDF = editMenu.addAction("Export PDF");

    QAction *selected = editMenu.exec(QCursor::pos());

    if(selected == cut) {
        on_actionCut_triggered();
    }
    if(selected == copy) {
        on_actionCopy_triggered();
    }
    if(selected == paste) {
        on_actionPaste_triggered();
    }

    if(selected == exPDF) {
        on_actionExport_PDF_triggered();
    }
}

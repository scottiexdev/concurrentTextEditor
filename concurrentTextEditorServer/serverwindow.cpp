#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QMessageBox>
#include "server.h"
#include <QEvent>
#include <QKeyEvent>

ServerWindow::ServerWindow(QWidget *parent, bool autoStart) :
    QWidget(parent),
    ui(new Ui::ServerWindow),
    m_server(new Server(this))
{
    ui->setupUi(this);
    connect(ui->startStopButton, &QPushButton::clicked, this, &ServerWindow::toggleStartServer);
    connect(m_server, &Server::logMessage, this, &ServerWindow::logMessage);
    connect(ui->serverConsole, &ServerConsole::executeCommand, m_server, &Server::executeCommand);

    if(autoStart)
        this->toggleStartServer();
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::toggleStartServer() {
    if(m_server->isListening()) {
        m_server->notifyServerDown();
        m_server->stopServer();
        ui->startStopButton->setText(tr("Start Server"));
        logMessage("Server Stopped");
    } else {
        if(!m_server->listen(QHostAddress::LocalHost, 1967)) {
            QMessageBox::critical(this, tr("Error"), tr("Unable to connect"));
            return;
        }
        logMessage("Server Started");
        ui->startStopButton->setText(tr("Stop Server"));
    }
}

void ServerWindow::logMessage(const QString &msg)  {
    ui->logEditor->appendPlainText(msg + '\n');
}


#ifndef CLIENTMAINWINDOW_H
#define CLIENTMAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>

//custom includes
#include <QDataStream>
#include "dialogsignup.h"
#include "homeloggedin.h"
#include "workerclient.h"
#include "loggedinmainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class clientmainwindow; }
QT_END_NAMESPACE

class clientmainwindow : public QMainWindow
{
    Q_OBJECT

public:
    clientmainwindow(QWidget *parent = nullptr);
    ~clientmainwindow();

private slots:
    void on_pushButtonLogin_clicked();
    void on_pushButtonSignup_clicked();
    void myLoggedIn();
    void mySignupOk();

private:
    Ui::clientmainwindow *ui;
    dialogsignup *d;
    loggedinmainwindow *hli;
    WorkerClient *_workerClient;    
};
#endif // CLIENTMAINWINDOW_H

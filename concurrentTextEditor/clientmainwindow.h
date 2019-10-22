#ifndef CLIENTMAINWINDOW_H
#define CLIENTMAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

//custom includes
#include <QDataStream>
#include "dialogsignup.h"
#include "homeloggedin.h"
#include "workerclient.h"

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

private:
    Ui::clientmainwindow *ui;
    dialogsignup *d;
    homeLoggedIn *hli;
    WorkerClient * _workerClient;
};
#endif // CLIENTMAINWINDOW_H

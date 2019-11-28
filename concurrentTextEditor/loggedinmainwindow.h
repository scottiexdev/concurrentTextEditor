#ifndef LOGGEDINMAINWINDOW_H
#define LOGGEDINMAINWINDOW_H

#include <QMainWindow>
#include "workerclient.h"
#include <QInputDialog>

namespace Ui {
class loggedinmainwindow;
}

class loggedinmainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit loggedinmainwindow(QWidget *parent, WorkerClient* worker);
    ~loggedinmainwindow();
    void requestFileList();

public slots:
    void showFiles(QStringList list);
    void errorDisplay(QString str="Error");

private slots:

    void on_pushButtonNewFile_2_clicked();

    void on_pushButtonLogout_2_clicked();

public:
    void on_pushButtonOpenFile_2_clicked();

private:
    Ui::loggedinmainwindow *ui;
    WorkerClient* _workerClient;
};

#endif // LOGGEDINMAINWINDOW_H

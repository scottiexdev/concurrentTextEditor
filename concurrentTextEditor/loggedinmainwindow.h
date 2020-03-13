#ifndef LOGGEDINMAINWINDOW_H
#define LOGGEDINMAINWINDOW_H

#include <QMainWindow>
#include "workerclient.h"

#include <QInputDialog>
#include "editor.h"

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
    void showFiles(QStringList list, QStringList list2, QStringList list3, bool isPublic);
    void errorDisplay(QString str="Error");

private slots:

    void on_pushButtonNewFile_2_clicked();

    void on_pushButtonLogout_2_clicked();

    void on_pushButtonOpenFile_2_clicked();

    void on_pushButtonUpdate_2_clicked();

    void on_pushButtonNewFile_3_clicked();

    void on_pushButtonNewPrivateFile_clicked();

public:


private:
    Ui::loggedinmainwindow *ui;
    Editor *_e;
    WorkerClient* _workerClient;
    void newFile(bool isPublic);
};

#endif // LOGGEDINMAINWINDOW_H

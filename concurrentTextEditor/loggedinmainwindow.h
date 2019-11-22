#ifndef LOGGEDINMAINWINDOW_H
#define LOGGEDINMAINWINDOW_H

#include <QMainWindow>
#include "workerclient.h"

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

private slots:
//    void on_pushButtonSettings_clicked();
    void on_pushButtonLogout_clicked();

private:
    Ui::loggedinmainwindow *ui;
    WorkerClient* _workerClient;
};

#endif // LOGGEDINMAINWINDOW_H

#ifndef HOMELOGGEDIN_H
#define HOMELOGGEDIN_H

#include <QtWidgets/QMainWindow>
#include "accountsettings.h"
#include "workerclient.h"

namespace Ui {
class homeLoggedIn;
}

class homeLoggedIn : public QMainWindow
{
    Q_OBJECT

public:
    explicit homeLoggedIn(QWidget *parent, WorkerClient* worker);
    ~homeLoggedIn();
    void requestFileList();

private slots:
    void on_pushButtonSettings_clicked();

    void on_pushButtonLogout_clicked();

private:
    Ui::homeLoggedIn *ui;
    accountSettings *ac_st;
    WorkerClient* _workerClient;
};

#endif // HOMELOGGEDIN_H

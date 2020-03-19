#ifndef LOGGEDINMAINWINDOW_H
#define LOGGEDINMAINWINDOW_H

#include <QMainWindow>
#include <QInputDialog>
#include <QClipboard>

#include "workerclient.h"
#include "editor.h"
#include "accountsettings.h"


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
    void isFileOpenOkay(const QJsonObject& qjo);

private slots:

    void on_pushButtonNewFile_2_clicked();

    void on_pushButtonLogout_2_clicked();

    void on_pushButtonOpenFile_2_clicked();

    void on_pushButtonUpdate_2_clicked();

    void on_pushButtonNewFile_3_clicked();

    void on_PublicFileListTable_cellDoubleClicked(int row, int column);

    void on_PrivatefileListTable_cellDoubleClicked(int row, int column);

    void on_pushButtonInvite_2_clicked();    

    void on_pushButtonDeleteFile_3_clicked();

    void on_pushButtonSettings_2_clicked();

    void on_pushButtonOpenSharedFile_3_clicked();


    void on_PublicFileListTable_cellClicked(int row, int column);

    void on_PrivatefileListTable_cellClicked(int row, int column);

public:


private:
    QString generateInviteLink(QString fileName, QString username);
    Ui::loggedinmainwindow *ui;
    Editor *_e;
    WorkerClient* _workerClient;
    accountSettings *_ac;
    void newFile(bool isPublic);
};

#endif // LOGGEDINMAINWINDOW_H

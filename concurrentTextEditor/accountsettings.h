#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include <QDialog>
#include <QInputDialog>
#include <QRegularExpression>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QHostAddress>
#include <QFormLayout>
#include <QDialogButtonBox>

#include "workerclient.h"

namespace Ui {
class accountSettings;
}

class accountSettings : public QDialog
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent *event);

public:
    explicit accountSettings(QWidget *parent = nullptr, WorkerClient *worker = nullptr);
    ~accountSettings();

public slots:
    void newUsernameNok();
    void newUsernameOk();
    void iconArrived(QPixmap icon);

private slots:
    void on_pushButton_U_clicked();

    void on_pushButton_EA_clicked();

    void on_pushButton_PP_clicked();

    void on_pushButton_PWD_clicked();

private:
    Ui::accountSettings *ui;

    // Icons path
    //QString _defaultIconPath = "/home/albo/Documents/repos/master/concurrentTextEditor/concurrentTextEditorServer/Icons/";
    QString _defaultIconPath = "C:/Users/giorg/Documents/GitHub/concurrentTextEditor/concurrentTextEditorServer/Icons/";
    //QString _defaultIconPath = "C:/Users/silvi/Google Drive/Politecnico/Magistrale/ProgettoDefinitivo/concurrentTextEditor/concurrentTextEditorServer/Icons/";

    // EDIT THIS -> prende path direttmente dal db con una query - inutile?
    // QString _defaultIcon=  _defaultIconPath+ "male_icon.png";
    WorkerClient *_worker;
};

#endif // ACCOUNTSETTINGS_H

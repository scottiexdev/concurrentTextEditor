#ifndef DIALOGSIGNUP_H
#define DIALOGSIGNUP_H

#include <QDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QDataStream>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QRegularExpression>
#include <QFileDialog>
#include <QBuffer>
#include "workerclient.h"

namespace Ui {
class dialogsignup;
}

class dialogsignup : public QDialog
{
    Q_OBJECT

public:
    explicit dialogsignup(QWidget *parent, WorkerClient *worker);
    ~dialogsignup();

private slots:
    void on_pushButton_Signup_clicked();

    void on_pushButton_Pic_clicked();

private:
    Ui::dialogsignup *ui;
    WorkerClient *_workerClient;

    // Icons path
    //QString _defaultIconPath = "/home/albo/Documents/repos/master/concurrentTextEditor/concurrentTextEditorServer/Icons/";
    QString _defaultIconPath = "C:/Users/giorg/Documents/GitHub/concurrentTextEditor/concurrentTextEditorServer/Icons/";
    //QString _defaultIcon = "C:/Users/silvi/Google Drive/Politecnico/Magistrale/ProgettoDefinitivo/concurrentTextEditor/concurrentTextEditorServer/Icons/";

    // EDIT THIS -> prende path direttmente dal db con una query
    QString _defaultIcon=  _defaultIconPath+ "male_icon.png";

    QString icn;
};

#endif // DIALOGSIGNUP_H

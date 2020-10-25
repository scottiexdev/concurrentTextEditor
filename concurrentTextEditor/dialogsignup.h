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
#include  <QStandardPaths>
#include "workerclient.h"
#define LOCAL_HOST 0

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

    QString icn;
};

#endif // DIALOGSIGNUP_H

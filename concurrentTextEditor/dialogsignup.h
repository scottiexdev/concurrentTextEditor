#ifndef DIALOGSIGNUP_H
#define DIALOGSIGNUP_H

#include <QDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QDataStream>
#include <QJsonDocument>
#include <QTcpSocket>
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
    void on_pushButton_clicked();

private:
    Ui::dialogsignup *ui;
    WorkerClient *_workerClient;
};

#endif // DIALOGSIGNUP_H

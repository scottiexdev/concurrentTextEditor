#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include "workerclient.h"
#include "char.h"
#include "Enums.h"

namespace Ui {
class Editor;
}

class Editor : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);

public:
    explicit Editor(QWidget *parent, WorkerClient *worker, QString fileName, bool isPublic);
    ~Editor();    

public slots:
    void handleFile(QJsonDocument buf);
    void showUser(QString user);
    QString deleteUser(QString user);    

private:
    Ui::Editor *ui;
    WorkerClient *_workerClient;
};

#endif // EDITOR_H

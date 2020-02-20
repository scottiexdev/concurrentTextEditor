#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include "workerclient.h"

namespace Ui {
class Editor;
}

class Editor : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);

public:
    explicit Editor(QWidget *parent, WorkerClient *worker, QString fileName);
    ~Editor();

public slots:
    void showFileLine(QString buf);
    void showUser(QString user);
    QString deleteUser(QString user);

private:
    Ui::Editor *ui;
    WorkerClient *_workerClient;
};

#endif // EDITOR_H

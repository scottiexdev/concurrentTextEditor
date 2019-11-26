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

public:
    explicit Editor(QWidget *parent, WorkerClient *worker, QString fileName);
    ~Editor();

public slots:
    void showFileLine(QString buf);

private:
    Ui::Editor *ui;
    WorkerClient *_workerClient;
};

#endif // EDITOR_H

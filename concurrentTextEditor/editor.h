#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QtPrintSupport/QPrinter>
#include <QFileDialog>
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

private slots:
    void on_actionExport_PDF_triggered();

private:
    Ui::Editor *ui;
    WorkerClient *_workerClient;
    QList<QColor> _colors = {
        QColor(255, 221, 89),
        QColor(75, 207, 250),
        QColor(239, 87, 119),
        QColor(186, 220, 88),
        QColor(126, 214, 223),
        QColor(199, 236, 238),
        QColor(149, 175, 192)
    };
    int _colorNumber=0;
};

#endif // EDITOR_H

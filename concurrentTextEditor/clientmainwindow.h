#ifndef CLIENTMAINWINDOW_H
#define CLIENTMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class clientmainwindow; }
QT_END_NAMESPACE

class clientmainwindow : public QMainWindow
{
    Q_OBJECT

public:
    clientmainwindow(QWidget *parent = nullptr);
    ~clientmainwindow();

private:
    Ui::clientmainwindow *ui;
};
#endif // CLIENTMAINWINDOW_H

#ifndef DIALOGSIGNUP_H
#define DIALOGSIGNUP_H

#include <QDialog>

//custom includes
#include <QMessageBox>
#include <QJsonObject>

namespace Ui {
class dialogsignup;
}

class dialogsignup : public QDialog
{
    Q_OBJECT

public:
    explicit dialogsignup(QWidget *parent = nullptr);
    ~dialogsignup();

private slots:
    void on_pushButton_clicked();

private:
    Ui::dialogsignup *ui;
};

#endif // DIALOGSIGNUP_H

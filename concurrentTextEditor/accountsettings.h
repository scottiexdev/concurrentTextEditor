#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include <QDialog>

namespace Ui {
class accountSettings;
}

class accountSettings : public QDialog
{
    Q_OBJECT

public:
    explicit accountSettings(QWidget *parent = nullptr);
    ~accountSettings();

private:
    Ui::accountSettings *ui;
};

#endif // ACCOUNTSETTINGS_H

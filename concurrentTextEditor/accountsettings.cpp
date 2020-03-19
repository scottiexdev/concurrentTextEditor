#include "accountsettings.h"
#include "ui_accountsettings.h"

accountSettings::accountSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::accountSettings)
{
    ui->setupUi(this);

    QPixmap pm (_defaultIcon);
    ui->img_label->setPixmap(pm);
    ui->img_label->setScaledContents(true);
}

accountSettings::~accountSettings()
{
    delete ui;
}

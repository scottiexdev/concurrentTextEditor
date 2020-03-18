#include "accountsettings.h"
#include "ui_accountsettings.h"

accountSettings::accountSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::accountSettings)
{
    ui->setupUi(this);

    QPixmap pm ("C:/Users/giorg/Documents/GitHub/concurrentTextEditor/concurrentTextEditorServer/Icons/male_icon.png");
    ui->img_label->setPixmap(pm);
    ui->img_label->setScaledContents(true);
}

accountSettings::~accountSettings()
{
    delete ui;
}

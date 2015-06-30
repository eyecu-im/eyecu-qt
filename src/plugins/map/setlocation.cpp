#include "setlocation.h"
#include "ui_setlocation.h"

SetLocation::SetLocation(const MercatorCoordinates &ACoordinates, const QIcon &AIcon, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetLocation)
{
    ui->setupUi(this);
    setWindowIcon(AIcon);
    ui->dspbLatitude->setValue(ACoordinates.latitude());
    ui->dspbLongitude->setValue(ACoordinates.longitude());
}

SetLocation::~SetLocation()
{
    delete ui;
}

MercatorCoordinates SetLocation::coordinates() const
{
    return MercatorCoordinates(ui->dspbLatitude->value(), ui->dspbLongitude->value());
}

void SetLocation::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

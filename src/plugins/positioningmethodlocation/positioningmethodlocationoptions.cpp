#include "positioningmethodlocationoptions.h"

PositioningMethodLocationOptions::PositioningMethodLocationOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PositioningMethodLocationOptions)
{
    ui->setupUi(this);
    connect(this->ui->boxInterval,SIGNAL(activated(int)),SIGNAL(modified()));
    reset();
}

PositioningMethodLocationOptions::~PositioningMethodLocationOptions()
{
    delete ui;
}

void PositioningMethodLocationOptions::apply()
{
    Options::node(OPV_POSITIONING_METHOD_LOCATION_INTERVAL).setValue(ui->boxInterval->itemData(ui->boxInterval->currentIndex()));
    emit childApply();
}

void PositioningMethodLocationOptions::reset()
{
    ui->boxInterval->setCurrentIndex(ui->boxInterval->findData(Options::node(OPV_POSITIONING_METHOD_LOCATION_INTERVAL).value().toString()));
    emit childReset();
}

void PositioningMethodLocationOptions::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

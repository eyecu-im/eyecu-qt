#include "positioningmethodmanualoptions.h"

PositioningMethodManualOptions::PositioningMethodManualOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PositioningMethodManualOptions)
{
    ui->setupUi(this);
	connect(ui->spbAutoSendInterval,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    reset();

}

PositioningMethodManualOptions::~PositioningMethodManualOptions()
{
    delete ui;
}

void PositioningMethodManualOptions::apply()
{
	Options::node(OPV_POSITIONING_METHOD_MANUAL_INTERVAL).setValue(ui->spbAutoSendInterval->value());
    emit childApply();
}

void PositioningMethodManualOptions::reset()
{
	ui->spbAutoSendInterval->setValue(Options::node(OPV_POSITIONING_METHOD_MANUAL_INTERVAL).value().toInt());
    emit childReset();
}

void PositioningMethodManualOptions::changeEvent(QEvent *e)
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

void PositioningMethodManualOptions::onSpinValueChanged(int AValue)
{
	ui->spbAutoSendInterval->setSuffix(" "+tr("second(s)", "Send interval units", AValue));
}

#include "positioningmethodmanualoptions.h"

PositioningMethodManualOptions::PositioningMethodManualOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PositioningMethodManualOptions)
{
    ui->setupUi(this);
    connect(ui->boxInterval,SIGNAL(activated(int)),SIGNAL(modified()));
    reset();

}

PositioningMethodManualOptions::~PositioningMethodManualOptions()
{
    delete ui;
}

void PositioningMethodManualOptions::apply()
{
    Options::node(OPV_POSITIONING_METHOD_MANUAL_INTERVAL).setValue(ui->boxInterval->currentText());
    emit childApply();
}

void PositioningMethodManualOptions::reset()
{
    ui->boxInterval->setCurrentIndex(ui->boxInterval->findText(Options::node(OPV_POSITIONING_METHOD_MANUAL_INTERVAL).value().toString()));
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

void PositioningMethodManualOptions::onEditTextChanged(const QString &AText)
{
	ui->lblIntervalUnits->setText(tr("second(s)", "Send interval units", AText.toInt()));
}

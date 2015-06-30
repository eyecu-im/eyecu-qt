#include "positioningmethodipoptions.h"

PositioningMethodIpOptions::PositioningMethodIpOptions(QHash<QUuid, IPositioningMethodIpProvider *> AProviders, QWidget *parent) :
    QWidget(parent),
	ui(new Ui::PositioningMethodIpOptions)
{
    ui->setupUi(this);
	for (QHash<QUuid, IPositioningMethodIpProvider *>::ConstIterator it = AProviders.constBegin(); it != AProviders.constEnd(); it++)
		ui->cmbProvider->addItem((*it)->icon(), (*it)->name(), it.key().toString());
	connect(ui->spbUpdateRate,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    reset();
}

PositioningMethodIpOptions::~PositioningMethodIpOptions()
{
    delete ui;
}

void PositioningMethodIpOptions::apply()
{
	Options::node(OPV_POSITIONING_METHOD_IP_UPDATERATE).setValue(ui->spbUpdateRate->value());
	Options::node(OPV_POSITIONING_METHOD_IP_PROVIDER).setValue(ui->cmbProvider->itemData(ui->cmbProvider->currentIndex()));
    emit childApply();
}

void PositioningMethodIpOptions::reset()
{
	ui->spbUpdateRate->setValue(Options::node(OPV_POSITIONING_METHOD_IP_UPDATERATE).value().toInt());
	ui->cmbProvider->setCurrentIndex(ui->cmbProvider->findData(Options::node(OPV_POSITIONING_METHOD_IP_PROVIDER).value().toString()));
	emit childReset();
}

void PositioningMethodIpOptions::onValueChanged(int AValue)
{
	ui->spbUpdateRate->setSuffix(" "+tr("second(s)", "", AValue));
}

void PositioningMethodIpOptions::changeEvent(QEvent *e)
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

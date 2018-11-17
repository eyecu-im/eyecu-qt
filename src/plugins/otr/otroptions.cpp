#include <definitions/optionvalues.h>
#include "otroptions.h"
#include "ui_otroptions.h"

OtrOptions::OtrOptions(QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OtrOptions)
{
	ui->setupUi(this);	
	reset();
	connect(ui->cmbPolicy, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->cbEndWhenOffline, SIGNAL(toggled(bool)), SIGNAL(modified()));
}

OtrOptions::~OtrOptions()
{
	delete ui;
}

QWidget *OtrOptions::instance()
{
	return this;
}

void OtrOptions::apply()
{
	Options::node(OPV_OTR_POLICY).setValue(ui->cmbPolicy->currentIndex());
	Options::node(OPV_OTR_ENDWHENOFFLINE).setValue(ui->cbEndWhenOffline->isChecked());
	emit childApply();
}

void OtrOptions::reset()
{
	ui->cmbPolicy->setCurrentIndex(Options::node(OPV_OTR_POLICY).value().toInt());
	ui->cbEndWhenOffline->setChecked(Options::node(OPV_OTR_ENDWHENOFFLINE).value().toBool());
	emit childReset();
}

#include <definitions/optionvalues.h>
#include "receiptsoptions.h"
#include "ui_receiptsoptions.h"

ReceiptsOptions::ReceiptsOptions(QWidget *AParent):
	QWidget(AParent),
	ui(new Ui::ReceiptsOptions)
{
	ui->setupUi(this);

        connect(ui->chkShow, SIGNAL(toggled(bool)), SIGNAL(modified()));
        connect(ui->chkSend, SIGNAL(toggled(bool)), SIGNAL(modified()));

	reset();
}

ReceiptsOptions::~ReceiptsOptions()
{
	delete ui;
}

QWidget *ReceiptsOptions::instance()
{
	return this;
}

void ReceiptsOptions::apply()
{
	Options::node(OPV_MARKERS_SHOW_LEVEL).setValue(ui->chkShow->isChecked());
	Options::node(OPV_MARKERS_SEND_RECEIVED).setValue(ui->chkSend->isChecked());

	emit childApply();
}

void ReceiptsOptions::reset()
{
	ui->chkShow->setChecked(Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt());
	ui->chkSend->setChecked(Options::node(OPV_MARKERS_SEND_RECEIVED).value().toBool());

	emit childReset();
}

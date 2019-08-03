#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include <utils/pluginhelper.h>
#include "otroptions.h"
#include "ui_otroptions.h"

OtrOptions::OtrOptions(Otr *AOtr, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OtrOptions),
	FOtr(AOtr),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>()),
	FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>())
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

void OtrOptions::onKeysClicked()
{
	FOptionsManager->showOptionsDialog(OPN_P2P_OTR, OPN_P2P, this);
}

#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <utils/options.h>
#include <utils/pluginhelper.h>
#include "omemooptions.h"
#include "ui_omemooptions.h"

OmemoOptions::OmemoOptions(Omemo *AOmemo, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OmemoOptions),
	FOmemo(AOmemo),
	FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>())
{
	ui->setupUi(this);

	reset();
}

OmemoOptions::~OmemoOptions()
{
	delete ui;
}

QWidget *OmemoOptions::instance()
{
	return this;
}

void OmemoOptions::apply()
{
	Options::node(OPV_OMEMO_FALLBACKMESSAGE).setValue(ui->tedFallback->toPlainText());
	Options::node(OPV_OMEMO_OPTOUTMESSAGE).setValue(ui->tedOptOut->toPlainText());
	Options::node(OPV_OMEMO_OPTOUTCONFIRM).setValue(ui->chkOptOutConfirm->isChecked());
	Options::node(OPV_OMEMO_SIMULATEERROR).setValue(ui->chkSimulateError->isChecked());
	emit childApply();
}

void OmemoOptions::reset()
{
	ui->tedFallback->setPlainText(Options::node(OPV_OMEMO_FALLBACKMESSAGE).value().toString());
	ui->tedOptOut->setPlainText(Options::node(OPV_OMEMO_OPTOUTMESSAGE).value().toString());
	ui->chkOptOutConfirm->setChecked(Options::node(OPV_OMEMO_OPTOUTCONFIRM).value().toBool());
	ui->chkSimulateError->setChecked(Options::node(OPV_OMEMO_SIMULATEERROR).value().toBool());
	emit childReset();
}

void OmemoOptions::onKeysClicked()
{
	FOptionsManager->showOptionsDialog(OPN_P2P_OMEMO, OPN_P2P, this);
}

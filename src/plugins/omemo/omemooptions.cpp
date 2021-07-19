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
#ifndef NO_OMEMO_OLD
	QPushButton *pbKeysOld = new QPushButton(tr("Keys (old OMEMO)"), this);
	ui->hlKeys->addWidget(pbKeysOld);
	connect(pbKeysOld, SIGNAL(clicked()), SLOT(onKeysClicked()));
#endif
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
	emit childApply();
}

void OmemoOptions::reset()
{
	ui->tedFallback->setPlainText(Options::node(OPV_OMEMO_FALLBACKMESSAGE).value().toString());
	ui->tedOptOut->setPlainText(Options::node(OPV_OMEMO_OPTOUTMESSAGE).value().toString());
	ui->chkOptOutConfirm->setChecked(Options::node(OPV_OMEMO_OPTOUTCONFIRM).value().toBool());
	emit childReset();
}

void OmemoOptions::onKeysClicked()
{
	FOptionsManager->showOptionsDialog(
#ifndef NO_OMEMO_OLD
		sender() != ui->pbKeys?OPN_P2P_OMEMO_OLD:
#endif
							   OPN_P2P_OMEMO, OPN_P2P, this);
}

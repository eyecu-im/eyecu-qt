#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
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
	emit childApply();
}

void OmemoOptions::reset()
{
	emit childReset();
}

void OmemoOptions::onKeysClicked()
{
	FOptionsManager->showOptionsDialog(OPN_P2P_OMEMO, OPN_P2P, this);
}

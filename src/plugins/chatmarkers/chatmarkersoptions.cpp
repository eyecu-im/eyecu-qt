#include <utils/options.h>
#include <definitions/optionvalues.h>
#include "chatmarkersoptions.h"
#include "ui_chatmarkersoptions.h"

ChatMarkersOptions::ChatMarkersOptions(QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::ChatMarkersOptions)
{
	ui->setupUi(this);

	connect(ui->chkShowReceived, SIGNAL(toggled(bool)), SIGNAL(modified()));
	connect(ui->chkShowAckOwn, SIGNAL(toggled(bool)), SIGNAL(modified()));

	connect(ui->chkSendReceived, SIGNAL(toggled(bool)), SIGNAL(modified()));
	connect(ui->chkSendDisplayed, SIGNAL(toggled(bool)), SIGNAL(modified()));
	connect(ui->chkSendAck, SIGNAL(toggled(bool)), SIGNAL(modified()));

	reset();
}

ChatMarkersOptions::~ChatMarkersOptions()
{
	delete ui;
}

QWidget *ChatMarkersOptions::instance()
{
	return this;
}

void ChatMarkersOptions::apply()
{
	Options::node(OPV_MARKERS_SHOW_LEVEL).setValue(
				ui->chkShowAck->isChecked()?3:
				ui->chkShowDisplayed->isChecked()?2:
				ui->chkShowReceived->isChecked()?1:0);

	Options::node(OPV_MARKERS_SHOW_ACKOWN)
			.setValue(ui->chkShowAckOwn->isChecked());

	Options::node(OPV_MARKERS_SEND_RECEIVED)
			.setValue(ui->chkSendReceived->isChecked());

	Options::node(OPV_MARKERS_SEND_DISPLAYED)
			.setValue(ui->chkSendDisplayed->isChecked());

	Options::node(OPV_MARKERS_SEND_ACK)
			.setValue(ui->chkSendAck->isChecked());

	emit childApply();
}

void ChatMarkersOptions::reset()
{
	switch (Options::node(OPV_MARKERS_SHOW_LEVEL).value().toInt())
	{
		case 3:
			ui->chkShowAck->setChecked(true);
		case 2:
			ui->chkShowDisplayed->setChecked(true);
		case 1:
			ui->chkShowReceived->setChecked(true);
	}

	ui->chkShowAckOwn->setChecked(
				Options::node(OPV_MARKERS_SHOW_ACKOWN).value().toBool());

	ui->chkSendReceived->setChecked(
				Options::node(OPV_MARKERS_SEND_RECEIVED).value().toBool());
	ui->chkSendDisplayed->setChecked(
				Options::node(OPV_MARKERS_SEND_DISPLAYED).value().toBool());
	ui->chkSendAck->setChecked(
				Options::node(OPV_MARKERS_SEND_ACK).value().toBool());

	emit childReset();
}

void ChatMarkersOptions::onShowDisplayedToggled(bool AChecked)
{
	if (AChecked)
	{
		ui->chkShowReceived->setDisabled(true);
		ui->chkShowReceived->setChecked(true);
	}
	else
		ui->chkShowReceived->setEnabled(true);

	emit modified();
}

void ChatMarkersOptions::onShowAckToggled(bool AChecked)
{
	if (AChecked)
	{
		ui->chkShowDisplayed->setDisabled(true);
		ui->chkShowDisplayed->setChecked(true);
	}
	else
		ui->chkShowDisplayed->setEnabled(true);

	emit modified();
}

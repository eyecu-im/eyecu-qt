#include <QDebug>
#include <QLabel>

#include <definitions/optionvalues.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

#include "iceoptions.h"
#include "addserver.h"
#include "ui_iceoptions.h"

IceOptions::IceOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::IceOptions)
{
	ui->setupUi(this);

	ui->twServers->setColumnWidth(0, 40);
	ui->twServers->setColumnWidth(1, 128);
	ui->twServers->setColumnWidth(2, 40);
	ui->twServers->setColumnWidth(3, 128);
	ui->twServers->sortByColumn(0, Qt::AscendingOrder);

	connect(ui->twServers->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(modified()));
	connect(ui->twServers->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(modified()));
	connect(ui->chkAggressive, SIGNAL(stateChanged(int)), SIGNAL(modified()));
	connect(ui->spbRto, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	connect(ui->spbNominationDelay, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	connect(ui->spbNominationTimeout, SIGNAL(valueChanged(int)), SIGNAL(modified()));

	reset();
}

IceOptions::~IceOptions()
{
	delete ui;
}

void IceOptions::apply()
{
	Options::node(OPV_JINGLE_TRANSPORT_ICE_NOMINATION_AGGRESSIVE)
			.setValue(ui->chkAggressive->isChecked());

	Options::node(OPV_JINGLE_TRANSPORT_ICE_NOMINATION_DELAY)
			.setValue(ui->spbNominationDelay->value());

	Options::node(OPV_JINGLE_TRANSPORT_ICE_NOMINATION_WAIT)
			.setValue(ui->spbNominationTimeout->value());

	Options::node(OPV_JINGLE_TRANSPORT_ICE_STUN_RTO)
			.setValue(ui->spbRto->value());

	QStringList turn, stun;

	int count = ui->twServers->topLevelItemCount();
	for (int i = 0; i < count; ++i)
	{
		QTreeWidgetItem *item = ui->twServers->topLevelItem(i);
		bool isTurn = item->text(0)=="TURN";
		int count = isTurn?5:3;
		QStringList parts;
		for (int c=1; c< count; ++c)
			parts.append(item->text(c));
		QString string(parts.join(":"));
		if (isTurn)
			turn.append(string);
		else
			stun.append(string);
	}

	Options::node(OPV_JINGLE_TRANSPORT_ICE_SERVERS_STUN).setValue(stun);
	Options::node(OPV_JINGLE_TRANSPORT_ICE_SERVERS_TURN).setValue(turn);

	emit childApply();
}

void IceOptions::reset()
{
	ui->chkAggressive->setChecked(Options::node(OPV_JINGLE_TRANSPORT_ICE_NOMINATION_AGGRESSIVE)
								  .value().toBool());

	ui->spbNominationDelay->setValue(Options::node(OPV_JINGLE_TRANSPORT_ICE_NOMINATION_DELAY)
								  .value().toInt());

	ui->spbNominationTimeout->setValue(Options::node(OPV_JINGLE_TRANSPORT_ICE_NOMINATION_WAIT)
								  .value().toInt());

	ui->spbRto->setValue(Options::node(OPV_JINGLE_TRANSPORT_ICE_STUN_RTO)
								  .value().toInt());

	QStringList stun(Options::node(OPV_JINGLE_TRANSPORT_ICE_SERVERS_STUN)
					 .value().toStringList());

	for (QStringList::ConstIterator it = stun.constBegin();
		 it != stun.constEnd(); ++it) {
		QStringList parts = (*it).split(':');
		if (parts.size() != 2)
			LOG_ERROR(QString("Invalid server description: %1").arg(*it));
		else
			ui->twServers->addTopLevelItem(
						new QTreeWidgetItem(QStringList(QStringList() << "STUN" << parts)));
	}

	QStringList turn(Options::node(OPV_JINGLE_TRANSPORT_ICE_SERVERS_TURN)
					 .value().toStringList());

	for (QStringList::ConstIterator it = turn.constBegin();
		 it != turn.constEnd(); ++it) {
		QStringList parts = (*it).split(':');
		if (parts.size() != 4)
			LOG_ERROR(QString("Invalid server description: %1").arg(*it));
		else
			ui->twServers->addTopLevelItem(
						new QTreeWidgetItem(QStringList(QStringList() << "TURN" << parts)));
	}

	emit childReset();
}

void IceOptions::onAdd()
{
	bool turn(sender() == ui->pbAddTurn);
	AddServer *dialog = new AddServer(turn, this);
	if (dialog->exec())
	{
		QStringList strings;
		if (turn)
			strings << "TURN";
		else
			strings << "STUN";
		strings.append(dialog->host());
		strings.append(dialog->port()?QString::number(dialog->port()):QString());
		if (turn)
		{
			strings.append(dialog->username());
			strings.append(dialog->password());
		}

		ui->twServers->addTopLevelItem(new QTreeWidgetItem(strings));
	}
	dialog->deleteLater();
}

void IceOptions::onRemove()
{
	QTreeWidgetItem *item = ui->twServers->currentItem();
	if (item)
		ui->twServers->takeTopLevelItem(ui->twServers->indexOfTopLevelItem(item));
}

void IceOptions::onCurrentItemChanged(QTreeWidgetItem *ACurrent, QTreeWidgetItem *APrevious)
{
	Q_UNUSED(APrevious)

	ui->pbRemove->setEnabled(ACurrent);
}

void IceOptions::onAggressiveNominationToggled(bool AState)
{
	ui->spbNominationDelay->setDisabled(AState);
}

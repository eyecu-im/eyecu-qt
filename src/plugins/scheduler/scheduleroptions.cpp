#include <QDebug>
#include <utils/pluginhelper.h>
#include "scheduleroptions.h"
#include "scheduleritemdialog.h"
#include "ui_scheduleroptions.h"

SchedulerOptions::SchedulerOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SchedulerOptions),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>()),
	FRosterManager(PluginHelper::pluginInstance<IRosterManager>())
{
	ui->setupUi(this);
}

SchedulerOptions::~SchedulerOptions()
{
	delete ui;
}

void SchedulerOptions::apply()
{
	emit childApply();
}

void SchedulerOptions::reset()
{
	emit childReset();
}

void SchedulerOptions::onItemAdd()
{
	SchedulerItemDialog *itemDialog = new SchedulerItemDialog(SchedulerItem(), FAccountManager, FRosterManager, this);
	if (itemDialog->exec())
	{
		qDebug() << "Ok";
		SchedulerItem schedulerItem = itemDialog->getItem();
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText(0, FAccountManager->findAccountByStream(schedulerItem.streamJid)->name());
		item->setData(0, Qt::UserRole, schedulerItem.streamJid.full());
		item->setText(1, schedulerItem.contactJid.full());
		item->setData(1, Qt::UserRole, schedulerItem.streamJid.full());
		item->setText(2, QString::number(schedulerItem.timeout));
		item->setText(3, schedulerItem.message);
		ui->twSchedule->addTopLevelItem(item);
	}
	itemDialog->deleteLater();
}

void SchedulerOptions::onItemEdit()
{
	QTreeWidgetItem *item = ui->twSchedule->currentItem();
	if (item)
	{
		SchedulerItem schedulerItem;
		schedulerItem.streamJid	= item->data(0, Qt::UserRole).toString();
		schedulerItem.contactJid = item->data(1, Qt::UserRole).toString();
		schedulerItem.timeout	= item->text(2).toInt();
		schedulerItem.message	= item->text(3);
		SchedulerItemDialog *itemDialog = new SchedulerItemDialog(schedulerItem, FAccountManager, FRosterManager, this);
		if (itemDialog->exec())
		{
			qDebug() << "Ok";
			SchedulerItem schedulerItem = itemDialog->getItem();
			item->setText(0, FAccountManager->findAccountByStream(schedulerItem.streamJid)->name());
			item->setData(0, Qt::UserRole, schedulerItem.streamJid.full());
			item->setText(1, schedulerItem.contactJid.full());
			item->setData(1, Qt::UserRole, schedulerItem.streamJid.full());
			item->setText(2, QString::number(schedulerItem.timeout));
			item->setText(3, schedulerItem.message);
		}
		itemDialog->deleteLater();
	}
}

void SchedulerOptions::onItemDelete()
{
	QTreeWidgetItem *item = ui->twSchedule->currentItem();
	if (item)
		delete ui->twSchedule->takeTopLevelItem(ui->twSchedule->indexOfTopLevelItem(item));
}

#include <utils/pluginhelper.h>
#include "scheduleroptions.h"
#include "scheduleritemdialog.h"
#include "ui_scheduleroptions.h"

SchedulerOptions::SchedulerOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SchedulerOptions),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>())
{
	ui->setupUi(this);
	connect(ui->cbActive, SIGNAL(toggled(bool)), SIGNAL(modified()));
	onCurrentItemChanged(NULL,NULL);
	reset();
}

SchedulerOptions::~SchedulerOptions()
{
	delete ui;
}

void SchedulerOptions::apply()
{
	Options::node(OPV_SCHEDULER_ACTIVE).setValue(ui->cbActive->isChecked());
	QStringList itemList;
	int items = ui->twSchedule->topLevelItemCount();
	for (int i=0; i<items; ++i)
	{
		QTreeWidgetItem *item = ui->twSchedule->topLevelItem(i);
		SchedulerItem schedulerItem;
		schedulerItem.streamJid = item->data(0, Qt::UserRole).toString();
		schedulerItem.contactJid = item->data(1, Qt::UserRole).toString();
		schedulerItem.timeout = item->text(2).toInt();
		schedulerItem.message = item->text(3);
		itemList.append(schedulerItem);
	}
	Options::node(OPV_SCHEDULER_ITEMS).setValue(itemList);
	emit childApply();
}

void SchedulerOptions::reset()
{
	ui->cbActive->setChecked(Options::node(OPV_SCHEDULER_ACTIVE).value().toBool());
	QStringList items = Options::node(OPV_SCHEDULER_ITEMS).value().toStringList();
	ui->twSchedule->clear();
	for (QStringList::ConstIterator it = items.constBegin(); it!=items.constEnd(); ++it)
		addItem(*it);
	emit childReset();
}

QTreeWidgetItem *SchedulerOptions::addItem(const SchedulerItem &AItem)
{
	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, FAccountManager->findAccountByStream(AItem.streamJid)->name());
	item->setData(0, Qt::UserRole, AItem.streamJid.full());
	item->setText(1, AItem.contactJid.full());
	item->setData(1, Qt::UserRole, AItem.contactJid.full());
	item->setText(2, QString::number(AItem.timeout));
	item->setText(3, AItem.message);
	ui->twSchedule->addTopLevelItem(item);
	return item;
}

void SchedulerOptions::onCurrentItemChanged(QTreeWidgetItem *ACurrent, QTreeWidgetItem *APrevious)
{
	Q_UNUSED(APrevious)

	if (ACurrent)
	{
		ui->pbDelete->setEnabled(true);
		ui->pbEdit->setEnabled(true);
	}
	else
	{
		ui->pbDelete->setEnabled(false);
		ui->pbEdit->setEnabled(false);
	}
}

void SchedulerOptions::onItemAdd()
{
	SchedulerItemDialog *itemDialog = new SchedulerItemDialog(SchedulerItem(), FAccountManager, this);
	if (itemDialog->exec())
	{
		SchedulerItem schedulerItem = itemDialog->getItem();
		QTreeWidgetItem *item = addItem(schedulerItem);
		ui->twSchedule->setCurrentItem(item);
		emit modified();
	}
	itemDialog->deleteLater();
}

void SchedulerOptions::onItemEdit()
{
	QTreeWidgetItem *item = ui->twSchedule->currentItem();
	if (item)
	{
		SchedulerItem existingItem;
		existingItem.streamJid	= item->data(0, Qt::UserRole).toString();
		existingItem.contactJid = item->data(1, Qt::UserRole).toString();
		existingItem.timeout	= item->text(2).toInt();
		existingItem.message	= item->text(3);
		SchedulerItemDialog *itemDialog = new SchedulerItemDialog(existingItem, FAccountManager, this);
		if (itemDialog->exec())
		{
			SchedulerItem newItem = itemDialog->getItem();
			if (newItem!=existingItem)
			{
				item->setText(0, FAccountManager->findAccountByStream(newItem.streamJid)->name());
				item->setData(0, Qt::UserRole, newItem.streamJid.full());
				item->setText(1, newItem.contactJid.full());
				item->setData(1, Qt::UserRole, newItem.contactJid.full());
				item->setText(2, QString::number(newItem.timeout));
				item->setText(3, newItem.message);
				emit modified();
			}
		}
		itemDialog->deleteLater();
	}
}

void SchedulerOptions::onItemDelete()
{
	QTreeWidgetItem *item = ui->twSchedule->currentItem();
	if (item)
	{
		delete ui->twSchedule->takeTopLevelItem(ui->twSchedule->indexOfTopLevelItem(item));
		emit modified();
	}
}

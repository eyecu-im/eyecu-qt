#include <QTextEdit>
#include <QPushButton>
#include <utils/pluginhelper.h>
#include "scheduleritemdialog.h"
#include "ui_scheduleritemdialog.h"

SchedulerItemDialog::SchedulerItemDialog(const SchedulerItem &AItem, IAccountManager *AAccountManager, QWidget *AParent):
	QDialog(AParent),
	ui(new Ui::SchedulerItemDialog),
	FAccountManager(AAccountManager),
	FRosterManager(PluginHelper::pluginInstance<IRosterManager>()),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FItem(AItem)
{
	ui->setupUi(this);
	if (FItem.streamJid.isValid() && FItem.contactJid.isValid() && FItem.timeout && !FItem.message.isNull())
		setWindowTitle(tr("Edit schedule record"));
	else
		setWindowTitle(tr("Add schedule record"));
	QList<IPresence*> presences = FPresenceManager->presences();
	for (QList<IPresence*>::ConstIterator it=presences.constBegin(); it!=presences.constEnd(); ++it)
	{
		IAccount *account = FAccountManager->findAccountByStream((*it)->streamJid());
		ui->cmbAccount->addItem(account->name(), account->accountJid().full());
	}

	if (AItem.streamJid.isValid())
	{
		int index = ui->cmbAccount->findData(AItem.streamJid.full());
		if (index>-1)
			ui->cmbAccount->setCurrentIndex(index);
		else
			ui->cmbAccount->setEditText(AItem.streamJid.full());
		if (AItem.contactJid.isValid())
		{
			int index = ui->cmbContact->findData(AItem.contactJid.full());
			if (index>-1)
				ui->cmbContact->setCurrentIndex(index);
			else
				ui->cmbContact->setEditText(AItem.contactJid.full());
		}
	}
	ui->spbTimeout->setValue(AItem.timeout);
	ui->tedMessage->setPlainText(AItem.message);
}

SchedulerItemDialog::~SchedulerItemDialog()
{
	delete ui;
}

SchedulerItem SchedulerItemDialog::getItem() const
{
	SchedulerItem item;
	int index = ui->cmbAccount->currentIndex();
	if (index>-1)
		item.streamJid = ui->cmbAccount->itemData(index).toString();
	else
		item.streamJid = ui->cmbAccount->currentText();
	index = ui->cmbContact->currentIndex();
	if (index>-1)
		item.contactJid = ui->cmbContact->itemData(index).toString();
	else
		item.contactJid = ui->cmbContact->currentText();
	item.timeout = ui->spbTimeout->value();
	item.message = ui->tedMessage->toPlainText();
	return item;
}

void SchedulerItemDialog::onTimeoutSpinboxValueChanged(int AValue)
{
	ui->spbTimeout->setSuffix(tr(" second(s)", "Keep space in front of unit if your language uses space to separate units from numbers", AValue));
}

void SchedulerItemDialog::onAccountSelected(int AIndex)
{
	ui->cmbContact->clear();
	Jid streamJid(ui->cmbAccount->itemData(AIndex).toString());	
	IRoster *roster = FRosterManager->findRoster(streamJid);
	if (roster)
	{
		IPresence *presence = FPresenceManager->findPresence(streamJid);
		if (presence)
		{
			QHash<Jid, QString> contacts;
			QList<Jid> items = presence->itemsJid();
			for (QList<Jid>::ConstIterator it=items.constBegin(); it!=items.constEnd(); ++it)
			{
				IRosterItem rosterItem = roster->findItem(*it);
				QString name = (rosterItem.name.isEmpty()?(*it).full():QString("%1 <%2>").arg(rosterItem.name).arg((*it).full()));
				contacts.insert(*it, name);
			}

			for (QHash<Jid, QString>::ConstIterator it=contacts.constBegin(); it!=contacts.constEnd(); ++it)
				ui->cmbContact->addItem(it.value(), it.key().full());
		}
	}
}

void SchedulerItemDialog::validate()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->cmbAccount->currentIndex()>-1 &&
															ui->cmbContact->currentIndex()>-1 &&
															ui->spbTimeout->value()>0 &&
															!ui->tedMessage->toPlainText().isEmpty() &&
															(ui->cmbAccount->itemData(ui->cmbAccount->currentIndex()).toString()!=FItem.streamJid.full() ||
															 ui->cmbContact->itemData(ui->cmbContact->currentIndex()).toString()!=FItem.contactJid.full() ||
															 ui->spbTimeout->value()!=FItem.timeout ||
															 ui->tedMessage->toPlainText()!=FItem.message));
}

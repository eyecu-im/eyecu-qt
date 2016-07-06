#include <QTextEdit>
#include <QPushButton>
#include "scheduleritemdialog.h"
#include "ui_scheduleritemdialog.h"

SchedulerItemDialog::SchedulerItemDialog(const SchedulerItem &AItem, IAccountManager *AAccountManager, IRosterManager *ARosterManager, QWidget *AParent) :
	QDialog(AParent),
	ui(new Ui::SchedulerItemDialog),
	FAccountManager(AAccountManager),
	FRosterManager(ARosterManager),
	FItem(AItem)
{
	ui->setupUi(this);
	QList<IAccount*> accounts = FAccountManager->accounts();
	for (QList<IAccount*>::ConstIterator it=accounts.constBegin(); it!=accounts.constEnd(); ++it)
		if ((*it)->isActive() && (*it)->xmppStream()->isConnected())
			ui->cmbAccount->addItem((*it)->name(), (*it)->accountJid().full());
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
	ui->tedMessage->setText(AItem.message);
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
	index = ui->cmbContact->currentIndex();
	if (index>-1)
		item.contactJid = ui->cmbContact->itemData(index).toString();
	item.timeout = ui->spbTimeout->value();
	item.message = ui->tedMessage->toPlainText();
	return item;
}

void SchedulerItemDialog::onAccountSelected(int AIndex)
{
	ui->cmbContact->clear();
	Jid streamJid(ui->cmbAccount->itemData(AIndex).toString());
	IRoster *roster = FRosterManager->findRoster(streamJid);
	if (roster)
	{
		QHash<Jid, QString> contacts;
		QList<IRosterItem> items = roster->items();
		for (QList<IRosterItem>::ConstIterator it = items.constBegin();  it!=items.constEnd(); ++it)
			if ((*it).itemJid.isValid())
			{
				QString name = ((*it).name.isEmpty()?(*it).itemJid.full():QString("%1 <%2>").arg((*it).name).arg((*it).itemJid.full()));
				contacts.insert((*it).itemJid, name);
			}

		for (QHash<Jid, QString>::ConstIterator it=contacts.constBegin(); it!=contacts.constEnd(); ++it)
			ui->cmbContact->addItem(it.value(), it.key().full());
	}
}

void SchedulerItemDialog::validate()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->cmbAccount->currentIndex()>-1 &&
															ui->cmbContact->currentIndex()>-1 &&
															ui->spbTimeout->value()>0 &&
															!ui->tedMessage->toPlainText().isEmpty());
}

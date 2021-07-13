#include "addlegacycontactdialog.h"

#include <QTextDocument>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>
#include <utils/logger.h>

AddLegacyContactDialog::AddLegacyContactDialog(IGateways *AGateways, IRosterChanger *ARosterChanger, const Jid &AStreamJid, const Jid &AServiceJid, QWidget *AParent) : QDialog(AParent)
{
	REPORT_VIEW;
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	setWindowTitle(tr("Add Legacy User to %1").arg(AServiceJid.uFull()));
	IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->insertAutoIcon(this,MNI_GATEWAYS_ADD_CONTACT,0,0,"windowIcon");

	FGateways = AGateways;
	FRosterChanger = ARosterChanger;
	FStreamJid = AStreamJid;
	FServiceJid = AServiceJid;

	connect(FGateways->instance(),SIGNAL(promptReceived(const QString &,const QString &,const QString &)),
		SLOT(onPromptReceived(const QString &,const QString &,const QString &)));
	connect(FGateways->instance(),SIGNAL(userJidReceived(const QString &, const Jid &)),
		SLOT(onUserJidReceived(const QString &, const Jid &)));
	connect(FGateways->instance(),SIGNAL(errorReceived(const QString &, const XmppError &)),
		SLOT(onErrorReceived(const QString &, const XmppError &)));
	connect(ui.dbbButtons,SIGNAL(clicked(QAbstractButton *)),SLOT(onDialogButtonsClicked(QAbstractButton *)));

	requestPrompt();
}

AddLegacyContactDialog::~AddLegacyContactDialog()
{

}

Jid AddLegacyContactDialog::streamJid() const
{
	return FStreamJid;
}

Jid AddLegacyContactDialog::serviceJid() const
{
	return FServiceJid;
}

void AddLegacyContactDialog::resetDialog()
{
	ui.lblPrompt->setVisible(false);
	ui.lnePrompt->setVisible(false);
}

void AddLegacyContactDialog::requestPrompt()
{
	FRequestId = FGateways->sendPromptRequest(FStreamJid,FServiceJid);
	resetDialog();

	if (!FRequestId.isEmpty())
		ui.lblDescription->setText(tr("Waiting for host response ..."));
	else
		ui.lblDescription->setText(tr("Error: Can't send request to host."));
	ui.dbbButtons->setStandardButtons(QDialogButtonBox::Cancel);
}

void AddLegacyContactDialog::requestUserJid()
{
	FContactId = ui.lnePrompt->text();
	if (!FContactId.isEmpty())
	{
		FRequestId = FGateways->sendUserJidRequest(FStreamJid,FServiceJid,FContactId);
		resetDialog();
		if (!FRequestId.isEmpty())
			ui.lblDescription->setText(tr("Waiting for host response ..."));
		else
			ui.lblDescription->setText(tr("Error: Can't send request to host."));
		ui.dbbButtons->setStandardButtons(QDialogButtonBox::Cancel);
	}
}

void AddLegacyContactDialog::onPromptReceived(const QString &AId, const QString &ADesc, const QString &APrompt)
{
	if (FRequestId == AId)
	{
		ui.lblDescription->setText(ADesc);
		ui.lblPrompt->setVisible(true);
		ui.lblPrompt->setText(!APrompt.isEmpty() ? APrompt : tr("Contact ID:"));
		ui.lnePrompt->setVisible(true);
		ui.lnePrompt->setText(QString());
		ui.dbbButtons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	}
}

void AddLegacyContactDialog::onUserJidReceived(const QString &AId, const Jid &AUserJid)
{
	if (FRequestId == AId)
	{
		ui.lblDescription->setText(tr("Jabber ID for %1 is %2").arg(FContactId).arg(AUserJid.uFull()));
		if (FRosterChanger)
		{
			IAddContactDialog *dialog = FRosterChanger!=NULL ? FRosterChanger->showAddContactDialog(FStreamJid) : NULL;
			if (dialog)
			{
				dialog->setContactJid(AUserJid);
				dialog->setNickName(FContactId);
				accept();
			}
		}
		ui.dbbButtons->setStandardButtons(QDialogButtonBox::Retry|QDialogButtonBox::Cancel);
	}
}

void AddLegacyContactDialog::onErrorReceived(const QString &AId, const XmppError &AError)
{
	if (FRequestId == AId)
	{
		resetDialog();
		ui.lblDescription->setText(tr("Requested operation failed: %1").arg(AError.errorMessage()));
		ui.dbbButtons->setStandardButtons(QDialogButtonBox::Retry|QDialogButtonBox::Cancel);
	}
}

void AddLegacyContactDialog::onDialogButtonsClicked(QAbstractButton *AButton)
{
	QDialogButtonBox::StandardButton button = ui.dbbButtons->standardButton(AButton);
	if (button == QDialogButtonBox::Ok)
		requestUserJid();
	else if (button == QDialogButtonBox::Retry)
		requestPrompt();
	else if (button == QDialogButtonBox::Cancel)
		reject();
}

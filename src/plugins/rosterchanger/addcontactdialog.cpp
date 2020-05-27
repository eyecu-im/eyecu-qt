#include "addcontactdialog.h"

#include <QSet>
#include <QMessageBox>
#include <definitions/toolbargroups.h>
#include <definitions/vcardvaluenames.h>
#include <definitions/optionvalues.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/pluginhelper.h>
#include <utils/action.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

AddContactDialog::AddContactDialog(IRosterChanger *ARosterChanger, const Jid &AStreamJid, QWidget *AParent) : QDialog(AParent)
{
	REPORT_VIEW;
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	setWindowTitle(tr("Add contact - %1").arg(AStreamJid.uBare()));
	IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->insertAutoIcon(this,MNI_RCHANGER_ADD_CONTACT,0,0,"windowIcon");

	FResolving = false;

	FStreamJid = AStreamJid;
	FRosterChanger = ARosterChanger;

	QToolBar *toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(16,16));
	ui.lytMainLayout->setMenuBar(toolBar);
	FToolBarChanger = new ToolBarChanger(toolBar);

	setSubscriptionMessage(tr("Please, authorize me to your presence."));

	connect(ui.dbbButtons,SIGNAL(accepted()),SLOT(onDialogAccepted()));
	connect(ui.dbbButtons,SIGNAL(rejected()),SLOT(reject()));

	initialize();
}

AddContactDialog::~AddContactDialog()
{
	emit dialogDestroyed();
}

Jid AddContactDialog::streamJid() const
{
	return FStreamJid;
}

Jid AddContactDialog::contactJid() const
{
	return Jid::fromUserInput(ui.lneContact->text());
}

void AddContactDialog::setContactJid(const Jid &AContactJid)
{
	ui.lneContact->setText(AContactJid.uBare());
}

QString AddContactDialog::nickName() const
{
	return ui.lneNickName->text();
}

void AddContactDialog::setNickName(const QString &ANick)
{
	ui.lneNickName->setText(ANick);
}

QString AddContactDialog::group() const
{
	return ui.cmbGroup->currentText();
}

void AddContactDialog::setGroup(const QString &AGroup)
{
	ui.cmbGroup->setEditText(AGroup);
}

QString AddContactDialog::subscriptionMessage() const
{
	return ui.tedMessage->toPlainText();
}

void AddContactDialog::setSubscriptionMessage(const QString &AText)
{
	ui.tedMessage->setPlainText(AText);
}

bool AddContactDialog::subscribeContact() const
{
	return ui.chbSubscribe->isChecked();
}

void AddContactDialog::setSubscribeContact(bool ASubscribe)
{
	ui.chbSubscribe->setCheckState(ASubscribe ? Qt::Checked : Qt::Unchecked);
}

ToolBarChanger *AddContactDialog::toolBarChanger() const
{
	return FToolBarChanger;
}

void AddContactDialog::initialize()
{
	IRosterManager *rosterManager = PluginHelper::pluginInstance<IRosterManager>();
	FRoster = rosterManager!=NULL ? rosterManager->findRoster(FStreamJid) : NULL;
	if (FRoster)
	{
		ui.cmbGroup->addItems(FRoster->groups().toList());
		ui.cmbGroup->model()->sort(0,Qt::AscendingOrder);
		ui.cmbGroup->setCurrentIndex(-1);
		ui.lblGroupDelim->setText(tr("* nested group delimiter - '%1'").arg(ROSTER_GROUP_DELIMITER));
	}

	FMessageProcessor = PluginHelper::pluginInstance<IMessageProcessor>();
	if (FMessageProcessor)
	{
		FShowChat = new Action(FToolBarChanger->toolBar());
		FShowChat->setText(tr("Chat"));
		FShowChat->setToolTip(tr("Open chat window"));
		FShowChat->setIcon(RSR_STORAGE_MENUICONS,MNI_CHATMHANDLER_MESSAGE);
		FToolBarChanger->insertAction(FShowChat,TBG_RCACD_ROSTERCHANGER);
		connect(FShowChat,SIGNAL(triggered(bool)),SLOT(onToolBarActionTriggered(bool)));

		FSendMessage = new Action(FToolBarChanger->toolBar());
		FSendMessage->setText(tr("Message"));
		FSendMessage->setToolTip(tr("Send Message"));
		FSendMessage->setIcon(RSR_STORAGE_MENUICONS,MNI_NORMALMHANDLER_MESSAGE);
		FToolBarChanger->insertAction(FSendMessage,TBG_RCACD_ROSTERCHANGER);
		connect(FSendMessage,SIGNAL(triggered(bool)),SLOT(onToolBarActionTriggered(bool)));
	}

	FVCardManager = PluginHelper::pluginInstance<IVCardManager>();
	if (FVCardManager)
	{
		FShowVCard = new Action(FToolBarChanger->toolBar());
		FShowVCard->setText(tr("VCard"));
		FShowVCard->setToolTip(tr("Show VCard"));
		FShowVCard->setIcon(RSR_STORAGE_MENUICONS,MNI_VCARD);
		FToolBarChanger->insertAction(FShowVCard,TBG_RCACD_ROSTERCHANGER);
		connect(FShowVCard,SIGNAL(triggered(bool)),SLOT(onToolBarActionTriggered(bool)));

		FResolve = new Action(FToolBarChanger->toolBar());
		FResolve->setText(tr("Nick"));
		FResolve->setToolTip(tr("Resolve nick name"));
		FResolve->setIcon(RSR_STORAGE_MENUICONS,MNI_GATEWAYS_RESOLVE);
		FToolBarChanger->insertAction(FResolve,TBG_RCACD_ROSTERCHANGER);
		connect(FResolve,SIGNAL(triggered(bool)),SLOT(onToolBarActionTriggered(bool)));

		connect(FVCardManager->instance(),SIGNAL(vcardReceived(const Jid &)),SLOT(onVCardReceived(const Jid &)));
	}
}

void AddContactDialog::onDialogAccepted()
{
	if (contactJid().isValid())
	{
		if (!FRoster->hasItem(contactJid()))
		{
			QSet<QString> groups;
			if (!group().isEmpty())
				groups += group();
			FRoster->setItem(contactJid().bare(),nickName(),groups);
			if (subscribeContact())
				FRosterChanger->subscribeContact(FStreamJid,contactJid(),subscriptionMessage());
			accept();
		}
		else
		{
			QMessageBox::information(NULL,FStreamJid.uBare(),tr("Contact <b>%1</b> already exists.").arg(HTML_ESCAPE(contactJid().uBare())));
		}
	}
	else if (!contactJid().isEmpty())
	{
		QMessageBox::warning(this,FStreamJid.uBare(),tr("Can't add contact '<b>%1</b>' because it is not a valid Jabber ID").arg(HTML_ESCAPE(contactJid().uBare())));
	}
}

void AddContactDialog::onToolBarActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action!=NULL && contactJid().isValid())
	{
		if (action == FShowChat)
		{
			FMessageProcessor->getMessageWindow(FStreamJid,contactJid(),Message::Chat,IMessageProcessor::ActionShowNormal);
		}
		else if (action == FSendMessage)
		{
			FMessageProcessor->getMessageWindow(FStreamJid,contactJid(),Message::Normal,IMessageProcessor::ActionShowNormal);
		}
		else if (action == FShowVCard)
		{
			FVCardManager->showVCardDialog(FStreamJid,contactJid().bare());
		}
		else if (action == FResolve)
		{
			FResolving = true;
			if (FVCardManager->hasVCard(contactJid().bare()))
				onVCardReceived(contactJid());
			else
				FVCardManager->requestVCard(FStreamJid,contactJid());
		}
	}
}

void AddContactDialog::onVCardReceived(const Jid &AContactJid)
{
	if (FResolving && AContactJid.pBare()== contactJid().pBare())
	{
		IVCard *vcard = FVCardManager->getVCard(AContactJid.bare());
		if (vcard)
		{
			QString nick = !vcard->value(VVN_NICKNAME).isEmpty() ? vcard->value(VVN_NICKNAME) : contactJid().node();
			setNickName(nick);
			vcard->unlock();
			if (!nick.isEmpty())
			{
				updateSubscriptionMessage(nick);
			}
		}
		FResolving = false;
	}
}

void AddContactDialog::updateSubscriptionMessage(const QString &ANick)
{
	QString nick;
	IAccountManager *accountManager = PluginHelper::pluginInstance<IAccountManager>();
	if (accountManager)
	{
		nick = accountManager->findAccountByStream(streamJid())->optionsNode().value(OPV_NICKNAME).toString();
	}
	if (nick.isEmpty())
	{
		IVCard *vcard = FVCardManager->getVCard(FStreamJid.bare());
		if (vcard)
		{
			if (!vcard->value(VVN_NICKNAME).isEmpty())
			{
				nick = vcard->value(VVN_NICKNAME);
				vcard->unlock();
			}
		}
	}
	if (nick.isEmpty())
	{
		nick = FStreamJid.node();
	}
	setSubscriptionMessage(Options::node(OPV_ROSTER_SUBSCRIPTION_MESSAGE).value().toString()
						   .replace("%(nick)",ANick)
						   .replace("%(whoami)",nick));
}

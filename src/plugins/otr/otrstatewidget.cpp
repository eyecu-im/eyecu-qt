#include "otrstatewidget.h"

#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include <utils/menu.h>

OtrStateWidget::OtrStateWidget(IOtr* AOtr, OtrMessaging* AOtrMessaging, IMessageWindow *AWindow,
							   const QString &AAccount, const QString &AContact, QWidget *AParent)
	: QToolButton(AParent),
	  FOtr(AOtr),
	  FOtrMessaging(AOtrMessaging),
	  FAccount(AAccount),
	  FContact(AContact)
{
	FWindow = AWindow;

	FMenu = new Menu(this);
	QActionGroup *actionGroup = new QActionGroup(FMenu);
	setMenu(FMenu);

	FStartSessionAction = new Action(FMenu);
	connect(FStartSessionAction, SIGNAL(triggered(bool)),
            this, SLOT(initiateSession(bool)));
	FStartSessionAction->setActionGroup(actionGroup);
	FMenu->addAction(FStartSessionAction);

	FEndSessionAction = new Action(FMenu);
	FEndSessionAction->setText(tr("&End private conversation"));
	connect(FEndSessionAction, SIGNAL(triggered(bool)),
            this, SLOT(endSession(bool)));
	FEndSessionAction->setActionGroup(actionGroup);
	FMenu->addAction(FEndSessionAction);

	FMenu->insertSeparator(NULL);

	FAuthenticateAction = new Action(FMenu);
	FAuthenticateAction->setText(tr("&Authenticate contact"));
	connect(FAuthenticateAction, SIGNAL(triggered(bool)),
            this, SLOT(authenticateContact(bool)));
	FAuthenticateAction->setActionGroup(actionGroup);
	FMenu->addAction(FAuthenticateAction);

	FSessionIdAction = new Action(FMenu);
	FSessionIdAction->setText(tr("Show secure session &ID"));
	connect(FSessionIdAction, SIGNAL(triggered(bool)),
            this, SLOT(sessionID(bool)));
	FSessionIdAction->setActionGroup(actionGroup);
	FMenu->addAction(FSessionIdAction);

	FFingerprintAction = new Action(FMenu);
	FFingerprintAction->setText(tr("Show own &fingerprint"));
	connect(FFingerprintAction, SIGNAL(triggered(bool)),
            this, SLOT(fingerprint(bool)));
	FFingerprintAction->setActionGroup(actionGroup);
	FMenu->addAction(FFingerprintAction);

    setToolTip(tr("OTR Messaging"));

	connect(FWindow->address()->instance(),SIGNAL(addressChanged(const Jid &, const Jid &)),SLOT(onWindowAddressChanged(const Jid &, const Jid &)));
	connect(FOtr->instance(),SIGNAL(otrStateChanged(const Jid &, const Jid &)),SLOT(onUpdateMessageState(const Jid &, const Jid &)));

	onUpdateMessageState(FWindow->streamJid(),FWindow->contactJid());
}

OtrStateWidget::~OtrStateWidget()
{

}

void OtrStateWidget::onWindowAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore); Q_UNUSED(AContactBefore);
	onUpdateMessageState(FWindow->streamJid(),FWindow->contactJid());
}

void OtrStateWidget::onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid)
{
    if (FWindow->streamJid()==AStreamJid && FWindow->contactJid()==AContactJid.full())
    {
        QString iconKey;
		IOtr::MessageState state = FOtrMessaging->getMessageState(FAccount, FContact);

		QString stateString(FOtrMessaging->getMessageStateString(FAccount,
														 FContact));

		if (state == IOtr::MsgStateEncrypted)
        {
			if (FOtrMessaging->isVerified(FAccount, FContact))
            {
            //    m_chatDlgAction->setIcon(QIcon(":/otrplugin/otr_yes.png"));
                iconKey = MNI_OTR_ENCRYPTED;
            }
            else
            {
            //    m_chatDlgAction->setIcon(QIcon(":/otrplugin/otr_unverified.png"));
                iconKey = MNI_OTR_UNVERFIFIED;
                stateString += ", " + tr("unverified");
            }
        }
        else
        {
            iconKey = MNI_OTR_NO;
        //    m_chatDlgAction->setIcon(QIcon(":/otrplugin/otr_no.png"));
        }

        setText(tr("OTR Messaging [%1]").arg(stateString));
        IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->insertAutoIcon(this,iconKey);

		if (state == IOtr::MsgStateEncrypted)
        {
			FStartSessionAction->setText(tr("Refre&sh private conversation"));
			FAuthenticateAction->setEnabled(true);
			FSessionIdAction->setEnabled(true);
			FEndSessionAction->setEnabled(true);
        }
        else
        {
			FStartSessionAction->setText(tr("&Start private conversation"));
			if (state == IOtr::MsgStatePlaintext)
            {
				FAuthenticateAction->setEnabled(false);
				FSessionIdAction->setEnabled(false);
				FEndSessionAction->setEnabled(false);
            }
            else // finished, unknown
            {
				FEndSessionAction->setEnabled(true);
				FAuthenticateAction->setEnabled(false);
				FSessionIdAction->setEnabled(false);
            }
        }

		if (FOtrMessaging->getPolicy() < IOtr::PolicyEnabled)
        {
			FStartSessionAction->setEnabled(false);
			FEndSessionAction->setEnabled(false);
        }
    }
}

//-----------------------------------------------------------------------------

void OtrStateWidget::initiateSession(bool b)
{
    Q_UNUSED(b);
	FOtrMessaging->startSession(FAccount, FContact);
}

//-----------------------------------------------------------------------------

void OtrStateWidget::authenticateContact(bool)
{
	FOtr->authenticateContact(FAccount, FContact);
}

//-----------------------------------------------------------------------------

void OtrStateWidget::sessionID(bool)
{
	QString sId = FOtrMessaging->getSessionId(FAccount, FContact);
    QString msg;

    if (sId.isEmpty())
    {
        msg = tr("No active encrypted session");
    }
    else
    {
        msg = tr("Session ID between account \"%1\" and %2: %3")
				.arg(FOtrMessaging->humanAccount(FAccount))
				.arg(FContact)
                .arg(sId);
    }

	FOtrMessaging->displayOtrMessage(FAccount, FContact, msg);
}

//-----------------------------------------------------------------------------

void OtrStateWidget::endSession(bool b)
{
    Q_UNUSED(b);
	FOtrMessaging->endSession(FAccount, FContact);
    onUpdateMessageState(FWindow->streamJid(),FWindow->contactJid());
}

//-----------------------------------------------------------------------------

void OtrStateWidget::fingerprint(bool)
{
	QString fingerprint = FOtrMessaging->getPrivateKeys()
									.value(FAccount,
                                           tr("No private key for account \"%1\"")
											 .arg(FOtrMessaging->humanAccount(FAccount)));

    QString msg(tr("Fingerprint for account \"%1\": %2")
				   .arg(FOtrMessaging->humanAccount(FAccount))
                   .arg(fingerprint));

	FOtrMessaging->displayOtrMessage(FAccount, FContact, msg);
}

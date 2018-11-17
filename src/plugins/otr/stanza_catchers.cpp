#include <utils/pluginhelper.h>

#include "stanza_catchers.h"
#include <utils/logger.h>

StanzaCatcher::StanzaCatcher(OtrMessaging* AOtr, IAccountManager* AAccountManager, QObject *AParent):
	QObject(AParent),
	FOtrMessaging(AOtr),
	FAccountManager(AAccountManager)
{

}

QObject * StanzaCatcher::instance()
{
	return this;
}

OtrMessaging* StanzaCatcher::otr()
{
	return FOtrMessaging;
}

IAccountManager* StanzaCatcher::accountManager()
{
	return FAccountManager;
}

bool StanzaCatcher::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	Message message(AStanza);
	if (message.type() != Message::Chat)
		return false;

	if (message.body().isEmpty())
		return false;

	if (message.stanza().attribute(SkipOtrCatcherFlag()) != "true")
	{
		return stanzaEditImpl(AHandleId, AStreamJid, AStanza, AAccept);
	}
	else
	{
		message.stanza().element().removeAttribute("skip_otr_processing");
	}
	return false;
}

//------------------------------------------------

InboundStanzaCatcher::InboundStanzaCatcher(OtrMessaging* AOtr, IAccountManager* AAccountManager, QObject* AParent)
	: StanzaCatcher(AOtr, AAccountManager, AParent)
{

}

bool InboundStanzaCatcher::stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	Q_UNUSED(AHandleId);
	Q_UNUSED(AAccept);

	bool ignore = false;
	Message message(AStanza);

	QString contact = message.from();
	QString account = accountManager()->findAccountByStream(AStreamJid)->accountId().toString();
	QString plainBody = message.body();

    QString decrypted;
	IOtr::MessageType messageType = otr()->decryptMessage(account, contact,
															 plainBody, decrypted);
    switch (messageType)
    {
		case IOtr::MsgTypeNone:
            break;
		case IOtr::MsgTypeIgnore:
            ignore = true;
            break;
		case IOtr::MsgTypeOtr:
            QString bodyText;

            bodyText = decrypted;

			message.setBody(bodyText);
			AStanza = message.stanza();
			break;
	}
	return ignore;

}

//------------------------------------------------

OutboundStanzaCatcher::OutboundStanzaCatcher(OtrMessaging* AOtr, IAccountManager* AAccountManager, QObject* AParent)
	: StanzaCatcher(AOtr, AAccountManager, AParent)
{

}

bool OutboundStanzaCatcher::stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	Q_UNUSED(AHandleId);
	Q_UNUSED(AAccept);

	Message message(AStanza);

	QString contact = message.to();
	QString account = accountManager()->findAccountByStream(AStreamJid)->accountId().toString();

	QString encrypted = otr()->encryptMessage(
		account,
		contact,
		message.body());
	message.setBody(encrypted);

    //if there has been an error, drop the message
    if (encrypted.isEmpty())
    {
        return true;
    }

	AStanza = message.stanza();


	/*if (!m_onlineUsers.value(account).contains(contact))
	{
	    m_onlineUsers[account][contact] = new PsiOtrClosure(account, contact,
	                                                        m_otrConnection);
	}*/
	//if (m_onlineUsers[account][contact]->encrypted()) {
	if (otr()->getMessageState(account, contact) == IOtr::MsgStateEncrypted) {
	    if (AStanza.to().contains("/")) {
	        // if not a bare jid
	        AStanza.document().appendChild(AStanza.document().createElementNS("urn:xmpp:hints" ,"no-copy")).toElement();
	    }

	    AStanza.document().appendChild(AStanza.document().createElementNS("urn:xmpp:hints", "no-permanent-store")).toElement();
	    AStanza.document().appendChild(AStanza.document().createElementNS("urn:xmpp:carbons:2", "private")).toElement();
	}

	return false;
}

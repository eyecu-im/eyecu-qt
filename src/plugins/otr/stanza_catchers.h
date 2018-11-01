#ifndef STANZA_CATCHER
#define STANZA_CATCHER

#include <interfaces/iaccountmanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/istanzaprocessor.h>

#include <utils/message.h>

#include "otrmessaging.h"

class IAccountManager;

class StanzaCatcher:
    public QObject,
    public IStanzaHandler
{
    Q_OBJECT
public:
	static const char* SkipOtrCatcherFlag()
	{
		return "skip_otr_processing";
	}

	//StanzaCatcher(psiotr::OtrMessaging* otr, IAccountManager* AAccountJid,QObject* Aparent);
	StanzaCatcher(psiotr::OtrMessaging* otr, IAccountManager* AAccountJid,QObject* Aparent);
	//virtual QObject *instance() { return this; }
	virtual QObject *instance();
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

protected:
	psiotr::OtrMessaging* otr();
	IAccountManager* accountManager();
	virtual bool stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept) = 0;

private:
	psiotr::OtrMessaging* m_otrConnection;
	IAccountManager* m_accountJid;
};

class InboundStanzaCatcher: public StanzaCatcher
{
public:
	InboundStanzaCatcher(psiotr::OtrMessaging* otr, IAccountManager* AAccountJid, QObject* Aparent);
	virtual bool stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
};

class OutboundStanzaCatcher: public StanzaCatcher
{
public:
	OutboundStanzaCatcher(psiotr::OtrMessaging* otr,IAccountManager* AAccountJid, QObject* Aparent);
	virtual bool stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
};


#endif //STANZA_CATCHER
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
	Q_INTERFACES(IStanzaHandler)
public:
	static const char* SkipOtrCatcherFlag()
	{
		return "skip_otr_processing";
	}

	StanzaCatcher(OtrMessaging* AOtr, IAccountManager* AAccountManager, QObject* AParent = nullptr);
	//virtual QObject *instance() { return this; }
	virtual QObject *instance();
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

protected:
	OtrMessaging* otr();
	IAccountManager* accountManager();
	virtual bool stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept) = 0;

private:
	OtrMessaging* FOtrMessaging;
	IAccountManager* FAccountManager;
};

class InboundStanzaCatcher: public StanzaCatcher
{
public:
	InboundStanzaCatcher(OtrMessaging* AOtr, IAccountManager* AAccountManager, QObject* AParent=nullptr);
	virtual bool stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
};

class OutboundStanzaCatcher: public StanzaCatcher
{
public:
	OutboundStanzaCatcher(OtrMessaging* AOtr,IAccountManager* AAccountManager, QObject* AParent=nullptr);
	virtual bool stanzaEditImpl(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
};


#endif //STANZA_CATCHER

#ifndef OTRPLUGIN_H
#define OTRPLUGIN_H

#include <QMultiMap>

#include <interfaces/ipluginmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/ioptionsmanager.h>

#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>

#include <utils/message.h>

#include "otrmessaging.h"
#include "psiotrconfig.h"

#include "stanza_catchers.h"
#include "otrstatewidget.h"

class IAccountManager;
class IMessageProcessor;
class Action;

class OtrClosure;

class Otr:
    public QObject,
	public IPlugin,
	public IOtr,
	public IOptionsDialogHolder,
	public IArchiveHandler,
	public IStanzaHandler
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IOtr IOptionsDialogHolder IArchiveHandler IStanzaHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IOtr")
#endif
public:
	Otr();
	~Otr();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return OTR_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//IArchiveHandler
	virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn);
	//IStanzaHandler
	virtual bool stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

	virtual OtrPolicy policy() const;
	virtual void optionChanged(const QString& AOption);
	static const char* SkipOtrCatcherFlag()
	{
		return "skip_otr_processing";
	}

    // OtrCallback
    virtual QString dataDir();
	virtual void sendMessage(const QString &AAccount, const QString &AContact,
							 const QString& AMessage);
	virtual bool isLoggedIn(const QString &AAccount, const QString &AContact);
	virtual void notifyUser(const QString &AAccount, const QString &AContact,
							const QString& AMessage, const OtrNotifyType& AType);

	virtual bool displayOtrMessage(const QString &AAccount, const QString &AContact,
								   const QString& AMessage);
	virtual void stateChange(const QString &AAccount, const QString &AContact,
							 OtrStateChange AChange);

	virtual void receivedSMP(const QString &AAccount, const QString &AContact,
							 const QString& AQuestion);
	virtual void updateSMP(const QString &AAccount, const QString &AContact,
						   int AProgress);

	virtual QString humanAccount(const QString& AAccountId);
	virtual QString humanAccountPublic(const QString& AAccountId);
	virtual QString humanContact(const QString& AAccountId, const QString &AContactJid);
	virtual void authenticateContact(const QString &AAccount, const QString &AContact);

protected:
	void notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const;

protected slots:
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onMessageWindowCreated(IMessageNormalWindow *AWindow);
	void onMessageWindowDestroyed(IMessageNormalWindow *AWindow);
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onChatWindowDestroyed(IMessageChatWindow *AWindow);
	void onPresenceOpened(IPresence *APresence);
	void onProfileOpened(const QString &AProfile);

signals:
	void otrStateChanged(const Jid &AStreamJid, const Jid &AContactJid) const;

private:
	OtrMessaging* FOtrConnection;
	QHash<QString, QHash<QString, OtrClosure*> > FOnlineUsers;
	IOptionsManager* FOptionsManager;
	IStanzaProcessor *FStanzaProcessor;
	IMessageArchiver *FMessageArchiver;
	IAccountManager* FAccountManager;
	IPresenceManager *FPresenceManager;
	IMessageProcessor* FMessageProcessor;
	InboundStanzaCatcher* FInboundCatcher;
	OutboundStanzaCatcher* FOutboundCatcher;
	QString FHomePath;
	QHash<IMessageToolBarWidget*, Action*> FActions;
	QHash<Action*, QToolButton*> FButtons;
	IMessageWidgets *FMessageWidgets;
	int				FSHIMessage;
	int				FSHIPresence;
	int				FSHOMessage;
	int				FSHOPresence;
};

#endif //OTRPLUGIN_H

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

#include <utils/message.h>

#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>

#include "otrmessaging.h"
#include "psiotrconfig.h"

#include "stanza_catchers.h"
#include "otrstatewidget.h"

class QToolButton;
class QAction;

class IAccountManager;
class IMessageProcessor;
class IMessageWidgets;
class IMessageToolBarWidget;
class Action;
class IMessageWindow;
class IMessageChatWindow;

#define OTRE2E_UUID "{8592e3c3-ef5e-42a9-91c9-faf1ed9a91c5}"

namespace psiotr
{

class PsiOtrClosure;

//-----------------------------------------------------------------------------

class OtrPlugin :
    public QObject,
	public IPlugin,
	public IOptionsDialogHolder,
	public IArchiveHandler,
	public IStanzaHandler,
	public OtrCallback
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IOptionsDialogHolder IArchiveHandler IStanzaHandler)
public:
	OtrPlugin();
	~OtrPlugin();
	//IPlugin
	//virtual QObject *instance() { return this; }
	virtual QObject *instance();
	virtual QUuid pluginUuid() const { return OTRE2E_UUID; }
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
	virtual void optionChanged(const QString& option);
	static const char* SkipOtrCatcherFlag()
	{
		return "skip_otr_processing";
	}

    // OtrCallback
    virtual QString dataDir();
    virtual void sendMessage(const QString &account, const QString &contact,
                             const QString& message);
    virtual bool isLoggedIn(const QString &account, const QString &contact);
    virtual void notifyUser(const QString &account, const QString &contact,
                            const QString& message, const OtrNotifyType& type);

    virtual bool displayOtrMessage(const QString &account, const QString &contact,
                                   const QString& message);
    virtual void stateChange(const QString &account, const QString &contact,
                             OtrStateChange change);

    virtual void receivedSMP(const QString &account, const QString &contact,
                             const QString& question);
    virtual void updateSMP(const QString &account, const QString &contact,
                           int progress);

    virtual QString humanAccount(const QString& accountId);
    virtual QString humanAccountPublic(const QString& accountId);
    virtual QString humanContact(const QString& accountId,
                                 const QString &AContactJid);
    virtual void authenticateContact(const QString &account, const QString &contact);
signals:
	void otrStateChanged(const Jid &AStreamJid, const Jid &AContactJid) const;

private:
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
protected:
	void notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const;

private slots:
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);

	void onMessageWindowCreated(IMessageWindow *AWindow);
	void onMessageWindowDestroyed(IMessageWindow *AWindow);
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onChatWindowDestroyed(IMessageChatWindow *AWindow);
	void onPresenceOpened(IPresence *APresence);
	void onProfileOpened(const QString &AProfile);

private:
	OtrMessaging* m_otrConnection;
	QHash<QString, QHash<QString, PsiOtrClosure*> > m_onlineUsers;
	IOptionsManager* FOptionsManager;
	IStanzaProcessor *FStanzaProcessor;
	IMessageArchiver *FMessageArchiver;
	IAccountManager* FAccountManager;
	IPresenceManager *FPresenceManager;
	IMessageProcessor* FMessageProcessor;
	InboundStanzaCatcher* m_inboundCatcher;
	OutboundStanzaCatcher* m_outboundCatcher;
	QString m_homePath;
	QHash<IMessageToolBarWidget*, Action*> m_actions;
	QHash<Action*, QToolButton*> m_buttons;
	IMessageWidgets *FMessageWidgets;
	int					FSHIMessage;
	int					FSHIPresence;
	int					FSHOMessage;
	int					FSHOPresence;
};

} // namespace psiotr

#endif //OTRPLUGIN_H

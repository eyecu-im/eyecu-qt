#ifndef CHATMARKERS_H
#define CHATMARKERS_H

#include <interfaces/ichatmarkers.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/inotifications.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imultiuserchat.h>
#include <interfaces/ireceipts.h>

#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>

#include <utils/options.h>

class ChatMarkers:
		public QObject,
		public IPlugin,
		public IChatMarkers,
		public IOptionsDialogHolder,
		public IMessageEditor,
		public IMessageWriter,
		public IArchiveHandler
 {
	Q_OBJECT
	Q_INTERFACES(IPlugin IChatMarkers IOptionsDialogHolder IMessageEditor IMessageWriter IArchiveHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IChatMarkers")
#endif
public:
	enum Type {
		Unknown,
		Received,
		Displayed,
		Acknowledged,
		Acknowledge
	};
	ChatMarkers();
	~ChatMarkers();

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return CHATMARKERS_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin(){return true;}
	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//IMessageEditor
	virtual bool messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection);
	//IMessageWriter
	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang);
	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang);
	//IArchiveHandler
	virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn);

	//Incomming
	QList<Jid> getLastMarkableDisplay(const Jid &AStreamJid, const QString &AContactBareJid) const;
	QList<Jid>  getLastMarkableAcknowledge(const Jid &AStreamJid, const QString &AContactBareJid) const;

protected:
	void setMessageMarker(const Jid &AStreamJid, const Jid &AContactJid,
						  const QString &AMessageId, Type AType);
	void setAcknowledgedMarker(const Jid &AStreamJid, const Jid &AContactJid,
							   const QString &AMessageId);
	void showNotification(const Jid &AStreamJid, const Jid &AContactJid, const Type &AType, int IdsNum);
	void markMessage(const Jid &AStreamJid, const Jid &AContactJid, const Type &AType, const QString &AMessageId);
	void markMessages(const Jid &AStreamJid, const Jid &AContactJid, const Type &AType, const QStringList &AMessageIds);
	void sendMessageMarked(const Jid &AStreamJid, const Jid &AContactJid, const Type &AType, const QString &AMessageId);
	bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool isReceiptsSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	void removeNotifiedMessages(IMessageChatWindow *AWindow);
	void updateToolBarAction();
	void updateToolBarAction(IMessageToolBarWidget *AWidget);
	void registerDiscoFeatures(bool ARegister);
	void setImage(IMessageChatWindow *AWindow, const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId, const QString &AImage, const QString &ATitle);

protected slots:
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onWindowActivated();
	void onNotificationActivated(int ANotifyId);
	//Options
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
	void onMarkable(const Jid &AStreamJid, const Jid &AContactJid);
	void onAcknowledgedByAction(bool);
	//Presence
	void onPresenceOpened(IPresence *APresence);
	void onContactStateChanged(const Jid &AStreamJid, const Jid &AContactJid, bool AStateOnline);
	//IReceipts
	void onMessageDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId);

signals:
	void markable(const Jid &AStreamJid, const Jid &AContactJid);

private:
	IMessageProcessor   *FMessageProcessor;
	IMessageArchiver    *FMessageArchiver;
	IServiceDiscovery   *FDiscovery;
	IOptionsManager     *FOptionsManager;
	INotifications      *FNotifications;
	IMessageWidgets     *FMessageWidgets;
	IPresenceManager    *FPresenceManager;
	IReceipts			*FReceipts;
	IconStorage         *FIconStorage;

	QHash<IMessageChatWindow *, int>   FNotifies;

	// Outgoing
	QHash<Jid, QHash<Jid, QStringList> > FRequestHash;

	QHash<Jid, QHash<Jid, QString> > FLastReceived;
	QHash<Jid, QHash<Jid, QString> > FLastDisplayed;
	QHash<Jid, QHash<Jid, QString> > FLastAcknowledged;

	QHash<Jid, QHash<Jid, QSet<QString> > > FDeliveredHash;

	// Incomming
	QHash<Jid, QHash<Jid, QStringList> > FLastMarkableDisplayHash;
	QHash<Jid, QHash<Jid, QStringList> > FLastMarkableAcknowledgeHash;
	QHash<Jid, QHash<Jid, QStringList> > FAcknowledgeHash;

	// Pending
	QHash<Jid, QHash<Jid, QHash<QString, int> > > FPendingMarkers;
};

#endif // CHATMARKERS_H

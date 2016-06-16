#ifndef JINGLERTP_H
#define JINGLERTP_H

#include <QSound>
#include <interfaces/ijingle.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/inotifications.h>
#include <interfaces/iavatars.h>
#include <interfaces/irostersview.h>
#include <interfaces/istatusicons.h>

#include "audio.h"
#include "audiooptions.h"
#include "payloadtypeoptions.h"

#define JINGLERTP_UUID "{3d5702bc-29b9-40f2-88fe-85887cd6d8dc}"

class JingleCallTimer: public QTimer
{
	Q_OBJECT
public:
	JingleCallTimer(QString ASoundFileName, QObject *parent=0);
protected:
	void timerEvent(QTimerEvent *e);
private:
	QSound  FSound;
};

class JingleRtp: public QObject,
				 public IPlugin,
				 public IOptionsDialogHolder,
				 public INotificationHandler,
				 public IJingleApplication
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IOptionsDialogHolder INotificationHandler IJingleApplication)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingleRtp")
#endif
	enum PendingType
	{
		AddContent,
		FillTransport,
		Connect
	};

public:

	enum CallType
	{
		Called,
		Rejected,
		Cancelled,
		Finished,
		Error
	};

	enum Command
	{
		VoiceCall,
		VideoCall,
		Hangup
	};

	enum InfoType
	{
		Active,
		Hold,
		Mute,
		Ringing,
		Unmute
	};

	JingleRtp();
	~JingleRtp();
	//IPlugin
	QObject *instance() { return this; }
	QUuid pluginUuid() const { return JINGLERTP_UUID; }
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin(){return true;}
	//IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//INotificationHandler
	bool showNotification(int AOrder, ushort AKind, int ANotifyId, const INotification &ANotification);
	//IJingleApplication
	QString ns() const;
	bool checkSupported(QDomElement &ADescription);

public:
	static QStringList stringsFromAvps(const QList<QAVP> &AAvps);
	static QList<QAVP> avpsFromStrings(const QStringList &AStrings);

public slots:
	void onSessionInitiated(const Jid &AStreamJid, const QString &ASid);
	void onActionAcknowledged(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus APreviousStatus, const Jid &ARedirect, IJingle::Reason AReason); // To notify, about own initiate request acknowleged
	void onSessionAccepted(const Jid &AStreamJid, const QString &ASid);
	void onSessionConnected(const Jid &AStreamJid, const QString &ASid);
	void onSessionTerminated(const Jid &AStreamJid, const QString &ASid, IJingle::SessionStatus APreviousStatus, IJingle::Reason AReason);
	void onSessionInformed(const QDomElement &AInfoElement);
	void onDataReceived(const Jid &AStreamJid, const QString &ASid);
	void onConnectionEstablished(IJingleContent *AContent);
	void onConnectionFailed(IJingleContent *AContent);

protected:
	bool    isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool    hasVideo(const Jid &AStreamJid, const QString &ASid) const;
	QString getSid(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool    removeSid(const Jid &AStreamJid, const QString &ASid);
	bool    windowNotified(const IMessageChatWindow *window) const;

	bool    sessionInfo(const Jid &AStreamJid, const Jid &AContactJid, InfoType AType, const QString &AName=QString::null);

	void    removeNotification(IMessageChatWindow *AWindow);
	void    removeNotification(const Jid &AStreamJid, const QString &ASid);
	void    putSid(const Jid &AStreamJid, const Jid &AContactJid, const QString &ASid);
	void    callChatMessage(const Jid &AStreamJid, const QString &ASid, CallType AType, IJingle::Reason AReason = IJingle::NoReason);

	IMessageChatWindow *getWindow(const Jid &AStreamJid, const Jid &AContactJid);
//	IMessageWindow *messageShowNotified(int AMessageId);
//	bool    showWindow(int ANotifyId, int AShowMode);

	void    registerDiscoFeatures();
	INotification callNotify(const Jid &AStreamJid, const QString &ASid, CallType ACallType);
	void    updateWindow(IMessageChatWindow *AWindow);
	bool    writeCallMessageIntoChat(IMessageChatWindow *AWindow, CallType AType, IJingle::Reason AReason = IJingle::NoReason);
	bool    updateWindowActions(IMessageChatWindow *AWindow);
	void    updateChatWindowActions(IMessageChatWindow *AChatWindow);

	void    addPendingContent(IJingleContent *AContent, PendingType AType);
	void    removePendingContent(IJingleContent *AContent, PendingType AType);
	bool    hasPendingContents(const QString &AStreamJid, const QString &ASid, PendingType AType);

	void    establishConnection(const Jid &AStreamJid, const QString &ASid);
	void    connectionEstablished(const Jid &AStreamJid, const QString &ASid);
	void    connectionTerminated(const Jid &AStreamJid, const QString &ASid);
	static void addPayloadType(IJingleContent *AContent, const QAVP &APayloadType);

protected slots:
	// Notofications
	void onNotificationActivated(int ANotifyId);
	void onNotificationRemoved(int ANotifyId);
	void onTabPageActivated();
	void onCall();
	void onHangup();
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);

protected slots:
	void onContentAdded(IJingleContent *AContent);
	void onContentAddFailed(IJingleContent *AContent);
	void onIncomingTransportFilled(IJingleContent *AContent);
	void onIncomingTransportFillFailed(IJingleContent *AContent);

private:
	IJingle             *FJingle;
	IServiceDiscovery   *FServiceDiscovery;
	IOptionsManager     *FOptionsManager;
	IMessageWidgets     *FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;
	IMessageProcessor   *FMessageProcessor;
	INotifications      *FNotifications;
	IAvatars            *FAvatars;
	IStatusIcons        *FStatusIcons;
	IconStorage         *FIconStorage;
	AudioOptions    *FJingleRtpOptions;
	QHash<Jid, QHash<Jid, QString> >	FSidHash;
	QHash<QString, IMessageChatWindow *>   FChatWindows;
	QMap<int, IMessageChatWindow *> FNotifies;
	QList<IMessageChatWindow *> FPendingChats;
	QList<int>          FPendingCalls;
	JingleCallTimer     *FCallTimer;
	QMultiHash<int, IJingleContent *> FPendingContents;

	QTimer              FDataSendTimer; //! --------- TEST ---------

	static const QString    types[4];
};

#endif // JINGLERTP_H

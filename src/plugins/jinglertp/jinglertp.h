#ifndef JINGLERTP_H
#define JINGLERTP_H

#include <QSound>
#include <QUdpSocket>
#include <QPair>
#include <MediaPlayer>
#include <MediaStreamer>
#include <QPayloadType>

#include <interfaces/ijingle.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imultiuserchat.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/inotifications.h>
#include <interfaces/iavatars.h>
#include <interfaces/irostersview.h>
#include <interfaces/istatusicons.h>
#include <interfaces/ipresencemanager.h>

#include "audiooptions.h"
#include "codecoptions.h"
#include "rtpiodevice.h"

#define JINGLERTP_UUID "{3d5702bc-29b9-40f2-88fe-85887cd6d8dc}"

class JingleCallTimer: public QTimer
{
	Q_OBJECT
public:
	JingleCallTimer(const QString &ASoundFileName, int ATimeout, QObject *parent=nullptr);

protected:
	void timerEvent(QTimerEvent *e);
private:
	QSound  FSound;
	int		FTimeout;
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
		Ring,
		Rejected,
		Cancelled,
		Connected,
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

public:
	static QString stringFromInts(const QList<int> &FInts);
	static QList<int> intsFromString(const QString &FString);

	//IPlugin
	QObject *instance() override { return this; }
	QUuid pluginUuid() const override { return JINGLERTP_UUID; }
	void pluginInfo(IPluginInfo *APluginInfo) override;
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	bool initObjects() override;
	bool initSettings() override;
	bool startPlugin() override {return true;}
	//IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) override;
	//INotificationHandler
	bool showNotification(int AOrder, ushort AKind, int ANotifyId, const INotification &ANotification) override;
	//IJingleApplication
	QString ns() const override;
	bool checkSupported(QDomElement &ADescription) override;

public slots:
	virtual void onSessionInitiated(const QString &ASid) override;
	virtual void onActionAcknowledged(const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus APreviousStatus, const Jid &ARedirect, IJingle::Reason AReason) override; // To notify, about own initiate request acknowleged
	virtual void onSessionAccepted(const QString &ASid) override;
	virtual void onSessionConnected(const QString &ASid) override;
	virtual void onSessionTerminated(const QString &ASid, IJingle::SessionStatus APreviousStatus, IJingle::Reason AReason) override;
	virtual void onSessionInformed(const QDomElement &AInfoElement) override;
	virtual void onSessionDestroyed(const QString &ASid) override;
	virtual void onConnectionEstablished(const QString &ASid, const QString &AName) override;
	virtual void onConnectionFailed(const QString &ASid, const QString &AName) override;

protected:
	bool    isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool	checkContent(IJingleContent *AContent);
	bool    hasVideo(const QString &ASid) const;
	QString findSid(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool    windowNotified(const IMessageChatWindow *window) const;

	bool    sessionInfo(const Jid &AStreamJid, const Jid &AContactJid, InfoType AType, const QString &AName=QString::null);

	void    removeNotification(IMessageChatWindow *AWindow);
	void    removeNotification(const QString &ASid);
	void    callChatMessage(const QString &ASid, CallType AType, IJingle::Reason AReason = IJingle::NoReason);
	void	checkRtpContent(IJingleContent *AContent, QIODevice *ARtpDevice);
	void	notifyRinging(const QString &ASid);

	IMessageChatWindow *getWindow(const Jid &AStreamJid, const Jid &AContactJid);

	void    registerDiscoFeatures();
	void	callNotify(const QString &ASid, CallType AEventType, IJingle::Reason AReason = IJingle::NoReason);
	void	removeMucNotification(const Jid &AStreamJid, const Jid &AUserJid);
	void    updateWindow(IMessageChatWindow *AWindow);
	QString chatNotification(const QString &AIcon, const QString &AMessage);
	bool    writeCallMessageIntoChat(IMessageChatWindow *AWindow, CallType AType,
									 IJingle::Reason AReason = IJingle::NoReason);
	void    updateChatWindowActions(IMessageChatWindow *AChatWindow);

	void    addPendingContent(const QString &ASid, const QString &AName);
	void    removePendingContent(const QString &ASid, const QString &AName);
	bool    hasPendingContents(const QString &ASid);

	void    establishConnection(const QString &ASid);

	void	checkRunningContents(const QString &ASid);

	MediaStreamer *startStreamMedia(const QPayloadType &APayloadType, QIODevice *ARtpDevice);
	MediaPlayer *startPlayMedia(const QPayloadType &APayloadType, QIODevice *ARtpIODevice);
	void	stopSessionMedia(const QString &ASid);

	IMessageChatWindow *chatWindow(const QString &ASid) const;

	static QHash<int, QPayloadType> payloadTypesFromDescription(const QDomElement &ADescription,
																QList<QPayloadType> *APayloadTypes=nullptr);
	static bool fillDescriptionWithPayloadTypes(QDomElement &ADescription,
												const QList<QPayloadType> &APayloadTypes);
	static void clearDescription(QDomElement &ADescription);

	static QAudioDeviceInfo selectedAudioDevice(QAudio::Mode AMode);
	static void addPayloadType(IJingleContent *AContent, const QPayloadType &APayloadType);

protected slots:
	// Notofications
	void onNotificationActivated(int ANotifyId);
	void onNotificationRemoved(int ANotifyId);
	void onTabPageActivated();
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onMultiChatWindowCreated(IMultiUserChatWindow *AWindow);
	void onMultiChatUserChanged(IMultiUser *AUser, int AData, const QVariant &ABefore);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onStreamerStatusChanged(int AStatus);
	void onPlayerStatusChanged(int AStatusNew, int AStatusOld);
	void onCall();
	void onHangup();

	void onRtpReadyRead();

protected slots:
	void onContactStateChanged(const Jid &AStreamJid, const Jid &AContactJid, bool AStateOnline);

private:
	IJingle             *FJingle;
	IServiceDiscovery   *FServiceDiscovery;
	IOptionsManager     *FOptionsManager;
	IPresenceManager    *FPresenceManager;
	IPluginManager		*FPluginManager;
	IMultiUserChatManager *FMultiChatManager;
	IMessageWidgets     *FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;
	IMessageProcessor   *FMessageProcessor;
	INotifications      *FNotifications;
	IAvatars            *FAvatars;
	IStatusIcons        *FStatusIcons;
	IconStorage         *FIconStorage;
	AudioOptions		*FJingleRtpOptions;
	QHash<QString, IMessageChatWindow *>   FChatWindows;
	QMap<int, QPair<Jid,Jid> > FNotifies;
	QMap<int, int>		FMucNotifies;
	QList<IMessageChatWindow *> FPendingChats;
	QList<int>          FPendingCalls;
	JingleCallTimer     *FCallTimer;
	QAVOutputFormat		FRtp;
	QMultiHash<QString, QString> FPendingContents;

	QHash<QIODevice *, IJingleContent *> FContents;
	QHash<IJingleContent *, MediaStreamer *> FStreamers;
	QHash<IJingleContent *, MediaPlayer *> FPlayers;
	QHash<QString, QThread *> FIOThreads;
	QHash<QString, QIODevice *> FIODevices;

	static const QString FTypes[4];
};

#endif // JINGLERTP_H

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
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/inotifications.h>
#include <interfaces/iavatars.h>
#include <interfaces/irostersview.h>
#include <interfaces/istatusicons.h>

#include "audiooptions.h"
#include "codecoptions.h"

#define JINGLERTP_UUID "{3d5702bc-29b9-40f2-88fe-85887cd6d8dc}"

class JingleCallTimer: public QTimer
{
	Q_OBJECT
public:
	JingleCallTimer(QString ASoundFileName, QObject *parent=nullptr);
	~JingleCallTimer();

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

public:
	static QString stringFromInts(const QList<int> &FInts);
	static QList<int> intsFromString(const QString &FString);

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

public slots:
	virtual void onSessionInitiated(const QString &ASid);
	virtual void onActionAcknowledged(const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus APreviousStatus, const Jid &ARedirect, IJingle::Reason AReason); // To notify, about own initiate request acknowleged
	virtual void onSessionAccepted(const QString &ASid);
	virtual void onSessionConnected(const QString &ASid);
	virtual void onSessionTerminated(const QString &ASid, IJingle::SessionStatus APreviousStatus, IJingle::Reason AReason);
	virtual void onSessionInformed(const QDomElement &AInfoElement);
	virtual void onConnectionEstablished(IJingleContent *AContent);
	virtual void onConnectionFailed(IJingleContent *AContent);

protected:
	bool    isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool	checkContent(IJingleContent *AContent);
	bool    hasVideo(const QString &ASid) const;
	QString getSid(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool    removeSid(const QString &ASid);
	bool    windowNotified(const IMessageChatWindow *window) const;

	bool    sessionInfo(const Jid &AStreamJid, const Jid &AContactJid, InfoType AType, const QString &AName=QString::null);

	void    removeNotification(IMessageChatWindow *AWindow);
	void    removeNotification(const Jid &AStreamJid, const QString &ASid);
	void    putSid(const Jid &AContactJid, const QString &ASid);
	void    callChatMessage(const QString &ASid, CallType AType, IJingle::Reason AReason = IJingle::NoReason);
	void	checkRtpContent(IJingleContent *AContent, QIODevice *ARtpDevice);

	IMessageChatWindow *getWindow(const Jid &AStreamJid, const Jid &AContactJid);

	void    registerDiscoFeatures();
	void	callNotify(const QString &ASid, CallType AEventType);
	void    updateWindow(IMessageChatWindow *AWindow);
	QString chatNotification(const QString &AIcon, const QString &AMessage);
	bool    writeCallMessageIntoChat(IMessageChatWindow *AWindow, CallType AType,
									 IJingle::Reason AReason = IJingle::NoReason);
	bool    updateWindowActions(IMessageChatWindow *AWindow);
	void    updateChatWindowActions(IMessageChatWindow *AChatWindow);

	void    addPendingContent(IJingleContent *AContent, PendingType AType);
	void    removePendingContent(IJingleContent *AContent, PendingType AType);
	bool    hasPendingContents(const QString &ASid, PendingType AType);

	void    establishConnection(const QString &ASid);
	void    connectionEstablished(const QString &ASid);
	void    connectionTerminated(const QString &ASid);

	MediaStreamer *startStreamMedia(const QPayloadType &APayloadType, QIODevice *ARtpDevice);
	MediaPlayer *startPlayMedia(const QPayloadType &APayloadType, QIODevice *ARtpIODevice);

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
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onStreamerStatusChanged(int AStatus);
	void onPlayerStatusChanged(int AStatusNew, int AStatusOld);
	void onCall();
	void onHangup();

	void onRtpReadyRead();

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
	QHash<QString, Jid>	FSidHash;
	QHash<QString, IMessageChatWindow *>   FChatWindows;
	QMap<int, IMessageChatWindow *> FNotifies;
	QList<IMessageChatWindow *> FPendingChats;
	QList<int>          FPendingCalls;
	JingleCallTimer     *FCallTimer;
	QAVOutputFormat		FRtp;
	QMultiHash<int, IJingleContent *> FPendingContents;

	QHash<QIODevice *, IJingleContent *> FContents;
	QHash<IJingleContent *, MediaStreamer *> FStreamers;
	QHash<IJingleContent *, MediaPlayer *> FPlayers;

	static const QString    types[4];	
};

#endif // JINGLERTP_H

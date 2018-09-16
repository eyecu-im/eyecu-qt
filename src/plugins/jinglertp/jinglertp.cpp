#include <QList>
#include <QProcess>

#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/jingleicons.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/notificationtypes.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/notificationdataroles.h>
#include <definitions/notificationhandlerorders.h>
#include <definitions/rosternotifyorders.h>
#include <definitions/tabpagenotifypriorities.h>
#include <definitions/soundfiles.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionvalues.h>

#include <utils/logger.h>
#include <utils/qt4qt5compat.h>
#include <MediaStreamer>
#include <MediaPlayer>
#include <QPayloadType>
#include <QPSocketAddress>

#include "jinglertp.h"

#define ADR_STREAM_JID          Action::DR_StreamJid
#define ADR_CONTACT_JID         Action::DR_Parametr4
#define ADR_COMMAND             Action::DR_Parametr1

#define STREAM_STREAM           100
#define STREAM_CONTACT          102

// JingleCallTimer
JingleCallTimer::JingleCallTimer(QString ASoundFileName, QObject *parent):
	QTimer(parent), FSound(ASoundFileName)
{
	start();
}

void JingleCallTimer::timerEvent(QTimerEvent *e)
{
	Q_UNUSED(e)

	if (!interval())        // Just started
		setInterval(2000);  // Set correct interval

#if QT_VERSION >= 0x050000
	if(!QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty())
#else
	if (QSound::isAvailable())
#endif
		FSound.play();
#ifdef Q_WS_X11
	else
		QProcess::startDetached(Options::node(OPV_NOTIFICATIONS_SOUNDCOMMAND)
								.value().toString(),QStringList()<<soundFile);
#endif
}

// JingleRtp

const QString JingleRtp::FTypes[]={"Active","Hold","Mute","Ringing"};

JingleRtp::JingleRtp():
	FJingle(nullptr),
	FServiceDiscovery(nullptr),
	FOptionsManager(nullptr),
	FPresenceManager(nullptr),
	FPluginManager(nullptr),
	FMessageWidgets(nullptr),
	FMessageStyleManager(nullptr),
	FMessageProcessor(nullptr),
	FNotifications(nullptr),
	FAvatars(nullptr),
	FStatusIcons(nullptr),
	FIconStorage(nullptr),
	FJingleRtpOptions(nullptr),
	FCallTimer(nullptr)
{}

JingleRtp::~JingleRtp()
{}

QString JingleRtp::stringFromInts(const QList<int> &FInts)
{
	QString result;
	for (QList<int>::ConstIterator it=FInts.constBegin(); it!=FInts.constEnd(); ++it)
	{
		if (!result.isEmpty())
			result.append(';');
		result.append(QString::number(*it));
	}
	return result;
}

QList<int> JingleRtp::intsFromString(const QString &FString)
{
	QList<int> result;
	QStringList strings = FString.split(';');
	for (QStringList::ConstIterator it=strings.constBegin(); it!=strings.constEnd(); ++it)
		result.append((*it).toInt());
	return result;
}

void JingleRtp::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Jingle RTP Sessions");
	APluginInfo->description = tr("Allows to perform voice calls");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(JINGLE_UUID);
}

bool JingleRtp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	FPluginManager = APluginManager;

	IPlugin *plugin= APluginManager->pluginInterface("IJingle").value(0, nullptr);
	if (plugin)
		FJingle = qobject_cast<IJingle *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, nullptr);
	if (plugin)
		FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,nullptr);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		connect(FPresenceManager->instance(), SIGNAL(contactStateChanged(Jid,Jid,bool)),
											  SLOT(onContactStateChanged(Jid,Jid,bool)));
	}

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0, nullptr);
	if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0, nullptr);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IAvatars").value(0, nullptr);
	if (plugin)
		FAvatars = qobject_cast<IAvatars *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IStatusIcons").value(0, nullptr);
	if (plugin)
		FStatusIcons = qobject_cast<IStatusIcons *>(plugin->instance());

	plugin = APluginManager->pluginInterface("INotifications").value(0, nullptr);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
		{
			connect(FNotifications->instance(),SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
			connect(FNotifications->instance(),SIGNAL(notificationRemoved(int)), SLOT(onNotificationRemoved(int)));
		}
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0, nullptr);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
			connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
	}

	//AInitOrder = 200;   // This one should be initialized AFTER ...
	return true; //FMessageWidgets!=NULL
}

bool JingleRtp::initObjects()
{
	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_JINGLE);//<---in menuicon--
	if (FServiceDiscovery)
		registerDiscoFeatures();

	if (FNotifications)
	{
		INotificationType notifyType;
		notifyType.order = NTO_JINGLE_RTP_CALL;
		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)
				->getIcon(JNI_RTP_CALL);
		notifyType.title = tr("When incoming voice or video call received");
		notifyType.kindMask = INotification::RosterNotify|INotification::TrayNotify|
							  INotification::TrayAction|INotification::PopupWindow|
							  INotification::SoundPlay|INotification::AlertWidget|
							  INotification::TabPageNotify|INotification::ShowMinimized|
							  INotification::AutoActivate;
		notifyType.kindDefs = notifyType.kindMask & ~(INotification::AutoActivate);
		FNotifications->registerNotificationType(NNT_JINGLE_RTP_CALL, notifyType);

		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)->getIcon(JNI_RTP_HANGUP);
		notifyType.title = tr("When incoming voice or video call missed");
		FNotifications->registerNotificationType(NNT_JINGLE_RTP_MISSED, notifyType);

		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)->getIcon(JNI_RTP_ERROR);
		notifyType.title = tr("Voice or video call session failed");
		FNotifications->registerNotificationType(NNT_JINGLE_RTP_ERROR, notifyType);

		FNotifications->insertNotificationHandler(NHO_DEFAULT, this);
	}

	QPayloadType::initialize();

	for (QAVOutputFormat::Iterator it; it; ++it)
		if ((*it).name()=="rtp")
		{
			FRtp = *it;
			break;
		}

	return true;
}

bool JingleRtp::initSettings()
{
	Options::setDefaultValue(OPV_JINGLE_RTP_CODECS_USED, stringFromInts(QList<int>() << QAVCodec::AvCodecIdOpus << QAVCodec::AvCodecIdSpeex << QAVCodec::AvCodecIdADPCMG722));	//Used codecs
	Options::setDefaultValue(OPV_JINGLE_RTP_AUDIO_BITRATE, 32000);		// Audio encoding bitrate
	Options::setDefaultValue(OPV_JINGLE_RTP_TIMEOUT, 5000);				// Audio data receive timeout
	Options::setDefaultValue(OPV_JINGLE_RTP_RTCP, true);				// Use RTCP
	Options::setDefaultValue(OPV_JINGLE_RTP_AUDIO_INPUT, QVariant());	// Audio input device
	Options::setDefaultValue(OPV_JINGLE_RTP_AUDIO_OUTPUT, QVariant());	// Audio output device
	if (FOptionsManager)
	{
		IOptionsDialogNode dnode = {ONO_JINGLERTP, OPN_JINGLERTP, MNI_JINGLE_RTP, tr("Jingle RTP")};
		FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

QMultiMap<int, IOptionsDialogWidget *> JingleRtp::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_JINGLERTP)
	{
		widgets.insertMulti(OHO_JINGLERTP_COMMON, FOptionsManager->newOptionsDialogHeader(tr("Common"), AParent));
		QSpinBox *spbTimeout = new QSpinBox();
		spbTimeout->setMinimum(0);
		spbTimeout->setMaximum(60000);
		spbTimeout->setSuffix(QString(" %1").arg(tr("msec")));
		IOptionsDialogWidget *timeout = FOptionsManager->newOptionsDialogWidget(Options::node(OPV_JINGLE_RTP_TIMEOUT),
																				tr("Data receive timeout"), spbTimeout, AParent);
		widgets.insertMulti(OWO_JINGLERTP_COMMON_TIMEOUT, timeout);
		widgets.insertMulti(OWO_JINGLERTP_COMMON_RTCP, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_JINGLE_RTP_RTCP),
																								  tr("Use RTCP"),AParent));
		widgets.insertMulti(OHO_JINGLERTP_AUDIO, FOptionsManager->newOptionsDialogHeader(tr("Audio"), AParent));
		widgets.insertMulti(OWO_JINGLERTP_AUDIO, new AudioOptions(AParent));
		widgets.insertMulti(OHO_JINGLERTP_CODECS, FOptionsManager->newOptionsDialogHeader(tr("Codecs"), AParent));
		widgets.insertMulti(OWO_JINGLERTP_CODECS, new CodecOptions(AParent));
	}
	return widgets;
}

bool JingleRtp::checkSupported(QDomElement &ADescription)
{
	Q_UNUSED(ADescription)

	return true;
}

void JingleRtp::onSessionInitiated(const QString &ASid)
{
	LOG_DEBUG(QString("JingleRtp::onSessionInitiated(%1)").arg(ASid));
	Jid contactJid = FJingle->contactJid(ASid);
	Jid streamJid = FJingle->streamJid(ASid);
	putSid(contactJid, ASid);
	writeCallMessageIntoChat(getWindow(streamJid, contactJid), Called);
	callNotify(ASid, Called);
}

void JingleRtp::onActionAcknowledged(const QString &ASid, IJingle::Action AAction,
									 IJingle::CommandRespond ARespond,
									 IJingle::SessionStatus APreviousStatus,
									 const Jid &ARedirect, IJingle::Reason AReason)
{
	LOG_DEBUG(QString("JingleRtp::onActionAcknowleged(%1, %2, %3, %4, %5")
			  .arg(ASid).arg(AAction).arg(ARespond).arg(APreviousStatus)
			  .arg(ARedirect.full()).arg(AReason));

	switch (ARespond)
	{
		case IJingle::Acknowledge:
		{
            LOG_INFO(QString("Action acknowledged: %1").arg(AAction));
			switch (AAction)
			{
				case IJingle::SessionAccept:
                    LOG_DEBUG("It's session-accept. Trying to establish connection.");
					establishConnection(ASid);
					break;

				default:
					break;
			}
			break;
		}
		case IJingle::ServiceUnavailable:
            LOG_WARNING("Service unavailable");
			removeSid(ASid);
			callChatMessage(ASid, Error);
			break;
		case IJingle::Redirect:
            LOG_WARNING(QString("Redirected: %1").arg(ARedirect.full()));
			removeSid(ASid);
			callChatMessage(ASid, Error);
			break;
		case IJingle::ResourceConstraint:
            LOG_WARNING("Resource constraint");
			removeSid(ASid);
			callChatMessage(ASid, Error);
			break;
		case IJingle::BadRequest:
            LOG_WARNING("Bad request");
			removeSid(ASid);
			callChatMessage(ASid, Error);
			break;
	}
}

void JingleRtp::onSessionAccepted(const QString &ASid)
{
	LOG_DEBUG((QString("JingleRtp::onSessionAccepted(%1)").arg(ASid)));
	IMessageChatWindow *window = chatWindow(ASid);
	if (window)
		updateChatWindowActions(window);
	establishConnection(ASid);
}

void JingleRtp::onSessionConnected(const QString &ASid)
{
	LOG_DEBUG((QString("JingleRtp::onSessionConnected(%1)").arg(ASid)));
	bool success(false);
	QThread *ioThread = new QThread(this);
	connect(ioThread, SIGNAL(finished()), ioThread, SLOT(deleteLater()));

	QHash<QString, IJingleContent *> contents = FJingle->contents(ASid);
	for (QHash<QString, IJingleContent *>::ConstIterator it=contents.constBegin(); it!=contents.constEnd(); it++)
	{
		QIODevice *rtpDevice = (*it)->ioDevice(1);
		if (rtpDevice)
		{
			rtpDevice->setObjectName("RTP");
			QList<QPayloadType> payloadTypeList;
			QHash<int,QPayloadType> payloadTypes = payloadTypesFromDescription((*it)->description(), &payloadTypeList);
			QList<int> codecIds = intsFromString(Options::node(OPV_JINGLE_RTP_CODECS_USED).value().toString());

			QPayloadType payloadType;
			for (QList<int>::ConstIterator itc = codecIds.constBegin(); itc!=codecIds.constEnd(); ++itc)
				if (payloadTypes.contains(*itc))
				{
					payloadType = payloadTypes[*itc];
					break;
				}

			if (payloadType.isNull())
				payloadType = payloadTypeList.first();

			if (!payloadType.isFilled())
				payloadType.fill();

			QIODevice *rtcpDevice = (*it)->ioDevice(2);
			if (rtcpDevice)
				rtcpDevice->setObjectName("RTCP");
			RtpIODevice *rtpio = new RtpIODevice(rtpDevice, rtcpDevice);
			rtpio->setObjectName("Output");
			connect(ioThread, SIGNAL(finished()), rtpio, SLOT(deleteLater()));
			rtpio->moveToThread(ioThread);
			ioThread->start(QThread::HighPriority);

			MediaStreamer *streamer = startStreamMedia(payloadType, rtpio);

			if (streamer)
			{
				FIOThreads.insert(ASid, ioThread);
				FStreamers.insert(*it, streamer);

				if (rtpDevice->bytesAvailable())
					checkRtpContent(*it, rtpDevice);
				else {
					FContents.insert(rtpDevice, *it);
					connect(rtpDevice, SIGNAL(readyRead()), SLOT(onRtpReadyRead()));
				}
				success = true;
			}
			else
			{
				delete rtpio;
				ioThread->exit();
				LOG_FATAL("Failed to start stream media");
			}
		}
		else
			LOG_FATAL("No Output device for component 1 UDP available!");
	}
	if (!success)
	{
		LOG_WARNING("No successful contents! Terminating session...");
		FJingle->sessionTerminate(ASid, IJingle::FailedApplication);
	}
}

void JingleRtp::onSessionTerminated(const QString &ASid,
									IJingle::SessionStatus APreviousStatus,
									IJingle::Reason AReason)
{
	LOG_DEBUG((QString("JingleRtp::onSessionTerminated(%1, %2, %3)")
			   .arg(ASid).arg(APreviousStatus).arg(AReason)));

	CallType type;

	switch (AReason)
	{
		case IJingle::Success:
			type=Finished;
			break;
		case IJingle::Decline:
		{
			type=Rejected;
			IMessageChatWindow *window = chatWindow(ASid);
			if (window)
			{
				removeNotification(window);
				updateChatWindowActions(window);
			}
			break;
		}
		case IJingle::Cancel:
			type=Cancelled;
			break;
		default:
			type=Error;
	}

	// Stop media
	stopSessionMedia(ASid);

	// Type termination message
	callChatMessage(ASid, type, AReason);

	if (type == Error ||
		(type == Cancelled &&
		 !FJingle->isOutgoing(ASid) &&
		 APreviousStatus == IJingle::Initiated))
		callNotify(ASid, type);

	removeSid(ASid);

	checkRunningContents(ASid);
}

void JingleRtp::onSessionInformed(const QDomElement &AInfoElement)
{
	LOG_DEBUG(QString("JingleRtp::onSessionInformed(%1)").arg(AInfoElement.tagName()));
}

void JingleRtp::checkRtpContent(IJingleContent *AContent, QIODevice *ARtpDevice)
{
	LOG_DEBUG(QString("JingleRtp::checkRtpContent(%1, %2)").arg(AContent->name()).arg(ARtpDevice->objectName()));

	QByteArray data = ARtpDevice->peek(2);
	if (data.size()==2)
	{
		int payloadTypeId = data[1]&0x7F;

		QDomElement description(AContent->description());
		QHash<int,QPayloadType> payloadTypes = payloadTypesFromDescription(description);
		for (QHash<int,QPayloadType>::ConstIterator it = payloadTypes.constBegin(); it!=payloadTypes.constEnd(); ++it)
			if ((*it).id==payloadTypeId)
			{
				QIODevice *rtcpDevice = AContent->ioDevice(2);
				RtpIODevice *rtpio = new RtpIODevice(ARtpDevice, rtcpDevice,
													 Options::node(OPV_JINGLE_RTP_TIMEOUT).value().toInt());
				rtpio->setObjectName("Intput");
				QThread *ioThread = FIOThreads.value(AContent->sid());
				connect(ioThread, SIGNAL(finished()), rtpio, SLOT(deleteLater()));
				rtpio->moveToThread(ioThread);
				MediaPlayer *player = startPlayMedia(*it, rtpio);
				if (player) {
					FPlayers.insert(AContent, player);
					return;
				} else {
					delete rtpio;
					LOG_ERROR("Failed to start media play!");
				}
			}
		qWarning() << "Invalid payload type!";
	}
}

void JingleRtp::callNotify(const QString &ASid, CallType AEventType)
{
	Jid contactJid=FJingle->contactJid(ASid);
	Jid streamJid=FJingle->streamJid(ASid);

	// Remove existing notification for the window, if any
	int notify=FNotifies.key(QPair<Jid,Jid>(streamJid, contactJid));
	if (notify)
		FNotifications->removeNotification(notify);

	IMessageChatWindow *window = getWindow(streamJid, contactJid);

	if (window && (!window->isActiveTabPage() ||
				   AEventType==Called ||
				   AEventType==Error))
	{
		INotification notification;
		notification.typeId = AEventType==Called ? NNT_JINGLE_RTP_CALL:
							  AEventType==Error	 ? NNT_JINGLE_RTP_ERROR:
												   NNT_JINGLE_RTP_MISSED;
		notification.kinds = FNotifications->enabledTypeNotificationKinds(notification.typeId);
		if (notification.kinds > 0)
		{
			bool video=hasVideo(ASid);
			QIcon icon = FIconStorage->getIcon(AEventType==Called ? (video ? JNI_RTP_CALL_VIDEO
																		   : JNI_RTP_CALL):
											   AEventType==Error  ? JNI_RTP_ERROR
																  : JNI_RTP_HANGUP);
			QString name = FNotifications->contactName(streamJid, contactJid);

			notification.data.insert(NDR_JINGLE_RTP_EVENT_TYPE, AEventType);
			notification.data.insert(NDR_ICON, icon);
			QString tooltip=AEventType==Called	  ? tr("Incoming %1 call from %2", "%1: call type (audio or video); %2: caller title"):
							AEventType==Cancelled ? tr("Missed %1 call from %2", "%1: call type (audio or video); %2: caller title"):
													tr("%1 call from %2 failed!", "%1: call type (audio or video); %2: caller title");

			notification.data.insert(NDR_TOOLTIP, tooltip.arg(video?tr("video")
																   :tr("voice"))
														 .arg(name));
			notification.data.insert(NDR_STREAM_JID, streamJid.full());
			notification.data.insert(NDR_CONTACT_JID, contactJid.full());

			// Popup data - used by AttentionDialog
			if (FAvatars)
			{
				QString avatarFileName=FAvatars->avatarFileName(FAvatars->avatarHash(contactJid));
				if (!avatarFileName.isEmpty())
					notification.data.insert(NDR_POPUP_IMAGE, avatarFileName);
			}
			notification.data.insert(NDR_POPUP_CAPTION, AEventType==Called	  ? tr("Incoming call!"):
														AEventType==Cancelled ? tr("Missed call!"):
																				tr("Failed call!"));
			notification.data.insert(NDR_POPUP_TITLE, name);
//			notification.data.insert(NDR_POPUP_HTML, HTML_ESCAPE_CHARS("Test"));
			if (AEventType==Called || AEventType==Cancelled)
				notification.data.insert(NDR_POPUP_TIMEOUT, 0);
			notification.data.insert(NDR_ROSTER_ORDER, RNO_JINGLE_RTP);
			notification.data.insert(NDR_ROSTER_FLAGS, IRostersNotify::Blink|
													   IRostersNotify::AllwaysVisible|
													   IRostersNotify::HookClicks);
			notification.data.insert(NDR_ROSTER_CREATE_INDEX, true);
			notification.data.insert(NDR_SOUND_FILE, AEventType==Called?SDF_JINGLE_RTP_CALL
																	   :SDF_SCHANGER_CONNECTION_ERROR);

			notification.data.insert(NDR_ALERT_WIDGET, qint64(window->instance()));
			notification.data.insert(NDR_TABPAGE_WIDGET, qint64(window->instance()));
			notification.data.insert(NDR_TABPAGE_PRIORITY, TPNP_JINGLE_RTP);
			notification.data.insert(NDR_TABPAGE_ICONBLINK, true);
			notification.data.insert(NDR_SHOWMINIMIZED_WIDGET, qint64(window->instance()));

			updateWindow(window);

			int notify=FNotifications->appendNotification(notification);

			if (window->isActiveTabPage() && AEventType != Called)
				FNotifications->removeNotification(notify);
			else {
				FNotifies.insert(notify, QPair<Jid,Jid>(streamJid, contactJid));
				if (AEventType==Called &&
					notification.kinds&INotification::SoundPlay)
					sessionInfo(streamJid, contactJid, Ringing);
			}
		}
	}
}

void JingleRtp::updateWindow(IMessageChatWindow *AWindow)
{
	QIcon icon;
	if (AWindow->instance()->isWindow() && windowNotified(AWindow))
		icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_JINGLE_RTP);
	else if (FStatusIcons)
		icon = FStatusIcons->iconByJid(AWindow->streamJid(),AWindow->contactJid());
	QString contactName = AWindow->infoWidget()->fieldValue(IMessageInfoWidget::Caption).toString();
	AWindow->updateWindow(icon, contactName, tr("%1 - Chat").arg(contactName), QString::null);
}

bool JingleRtp::showNotification(int AOrder, ushort AKind, int ANotifyId, const INotification &ANotification)
{
	Q_UNUSED(AOrder)

	if (ANotification.typeId==NNT_JINGLE_RTP_CALL)
		if (AKind==INotification::SoundPlay)
		{
			if (FPendingCalls.isEmpty())
			{
				QString soundName = ANotification.data.value(NDR_SOUND_FILE).toString();
				QString soundFile = FileStorage::staticStorage(RSR_STORAGE_SOUNDS)->fileFullName(soundName);
				FCallTimer=new JingleCallTimer(soundFile, this);
			}
			FPendingCalls.append(ANotifyId);
			return true;
		}
	return false;
}

QString JingleRtp::ns() const
{
	return NS_JINGLE_APPS_RTP;
}

void JingleRtp::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.var = NS_JINGLE_APPS_RTP;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_JINGLE_RTP);
	dfeature.name = tr("Jingle RTP Sessions");
	dfeature.description = tr("Audio/Video chat via Jingle RTP");
	FServiceDiscovery->insertDiscoFeature(dfeature);

	dfeature.var = NS_JINGLE_APPS_RTP_AUDIO;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)->getIcon(JNI_RTP_CALL);
	dfeature.name = tr("Jingle RTP Audio");
	dfeature.description = tr("Jingle RTP Audio streaming");
	FServiceDiscovery->insertDiscoFeature(dfeature);

	dfeature.var = NS_JINGLE_APPS_RTP_VIDEO;
	dfeature.active = false;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)->getIcon(JNI_RTP_CALL_VIDEO);
	dfeature.name = tr("Jingle RTP Video");
	dfeature.description = tr("Jingle RTP Video streaming");
	FServiceDiscovery->insertDiscoFeature(dfeature);
}

bool JingleRtp::hasVideo(const QString &ASid) const
{
	return false; // Video is not supported yet
	QHash<QString, IJingleContent *> contents=FJingle->contents(ASid);
	for (QHash<QString, IJingleContent *>::const_iterator it=contents.constBegin(); it!=contents.constEnd(); it++)
		if ((*it)->description().attribute("media")=="video")
			return true;
	return false;
}

void JingleRtp::onNotificationActivated(int ANotifyId)
{
	if (FNotifies.contains(ANotifyId))
	{
		QPair<Jid,Jid> address = FNotifies.value(ANotifyId);
		IMessageChatWindow *window = getWindow(address.first, address.second);
		window->showTabPage();
	}
}

void JingleRtp::onNotificationRemoved(int ANotifyId)
{
	if (FPendingCalls.contains(ANotifyId))
	{
		FPendingCalls.removeOne(ANotifyId);
		if (FPendingCalls.isEmpty())
			FCallTimer->deleteLater();
	}
	FNotifies.remove(ANotifyId);
}

void JingleRtp::onTabPageActivated()
{
	IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());	
	int notify=FNotifies.key(QPair<Jid,Jid>(window->address()->streamJid(),
											window->address()->contactJid()));
	if (notify)
	{
		INotification notification=FNotifications->notificationById(notify);
		CallType type = CallType(notification.data
								 .value(NDR_JINGLE_RTP_EVENT_TYPE).toInt());
		if (type != Called)				// All the events but incoming Call
			removeNotification(window);	// should be removed immediately!
	}
	if (FPendingChats.contains(window))
	{
		updateChatWindowActions(window);
		FPendingChats.removeAll(window);
	}
}

void JingleRtp::removeNotification(IMessageChatWindow *AWindow)
{
	if (AWindow)
	{
		int notify=FNotifies.key(QPair<Jid,Jid>(AWindow->address()->streamJid(),
												AWindow->address()->contactJid()),
								 0);
		if (notify)
		{
			FNotifications->removeNotification(notify);
			updateWindow(AWindow);
		}
	}
}

void JingleRtp::removeNotification(const Jid &AStreamJid, const QString &ASid)
{
	removeNotification(FMessageWidgets->findChatWindow(AStreamJid, FJingle->contactJid(ASid)));
}
//TODO: Check, if entity is online
bool JingleRtp::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return !FServiceDiscovery || !FServiceDiscovery->hasDiscoInfo(AStreamJid,AContactJid) ||
		   (FServiceDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_JINGLE_APPS_RTP) &&
			FServiceDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_JINGLE_APPS_RTP_AUDIO));
}

bool JingleRtp::checkContent(IJingleContent *AContent)
{
	LOG_DEBUG("JingleRtp::checkContent()");
	QDomElement description(AContent->description());

	QHash<int, QPayloadType> sourcePayloadTypes = payloadTypesFromDescription(description);
	clearDescription(description);
	QList<QPayloadType> targetPayloadTypes;
	QList<int> codecIds = intsFromString(Options::node(OPV_JINGLE_RTP_CODECS_USED).value().toString());
	for (QList<int>::ConstIterator it = codecIds.constBegin(); it!=codecIds.constEnd(); ++it)
	{
		QList<QPayloadType> payloadTypes = sourcePayloadTypes.values(*it);
		for (QList<QPayloadType>::ConstIterator itp=payloadTypes.constBegin(); itp!=payloadTypes.constEnd(); ++itp)
			if ((*itp).isSupported(FRtp, QAVCodec::Both))
				targetPayloadTypes.append(*itp);
		sourcePayloadTypes.remove(*it);
	}

	for (QHash<int, QPayloadType>::ConstIterator it=sourcePayloadTypes.constBegin(); it!=sourcePayloadTypes.constEnd(); ++it)
		if ((*it).isSupported(FRtp, QAVCodec::Both))
			targetPayloadTypes.append(*it);

	if (!targetPayloadTypes.isEmpty())
		fillDescriptionWithPayloadTypes(description, targetPayloadTypes);

	if (description.firstChildElement("payload-type").isNull())
	{
		LOG_ERROR("JingleRtp::checkContent() returns false: no supported payload types found");
		return false;
	}
	else
	{
		LOG_INFO("JingleRtp::checkContent() returns true");
		return true;
	}
}

QString JingleRtp::getSid(const Jid &AStreamJid, const Jid &AContactJid) const
{
	QList<QString> sids = FSidHash.keys(AContactJid);
	for (QList<QString>::ConstIterator it = sids.constBegin(); it!=sids.constEnd(); ++it)
		if (FJingle->streamJid(*it)==AStreamJid)
			return *it;
	return QString::null;
}

bool JingleRtp::removeSid(const QString &ASid)
{
	return FSidHash.remove(ASid)>0;
}

bool JingleRtp::windowNotified(const IMessageChatWindow *window) const
{
	QPair<Jid,Jid> address(window->address()->streamJid(),
						   window->address()->contactJid());
	for (QMap<int, QPair<Jid,Jid> >::const_iterator it=FNotifies.constBegin();
		 it!=FNotifies.constEnd(); it++)
		if (*it == address)
			return true;
	return false;
}

bool JingleRtp::sessionInfo(const Jid &AStreamJid, const Jid &AContactJid, JingleRtp::InfoType AType, const QString &AName)
{
	QString sid = getSid(AStreamJid, AContactJid);
	if (!sid.isEmpty())
	{
		QDomDocument doc;
		QDomElement info=doc.createElementNS(NS_JINGLE_APPS_RTP_INFO, FTypes[AType]);
		if (AType==Mute || AType==Unmute)
		{
			info.setAttribute("creator", FJingle->isOutgoing(sid)?"initiator":"responder");
			if (!AName.isEmpty())
				info.setAttribute("name", AName);
		}
		return FJingle->sendAction(sid, IJingle::SessionInfo, info);
	}
	return false;
}

void JingleRtp::putSid(const Jid &AContactJid, const QString &ASid)
{
	FSidHash.insertMulti(ASid, AContactJid);
}

void JingleRtp::callChatMessage(const QString &ASid, CallType AType, IJingle::Reason AReason)
{
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(FJingle->streamJid(ASid),
															   FJingle->contactJid(ASid));
	if (window)
		writeCallMessageIntoChat(window, AType, AReason);
}

QString JingleRtp::chatNotification(const QString &AIcon, const QString &AMessage)
{
	QString image("<img src=\'%1\'/> %2");
	return image.arg(QUrl::fromLocalFile(FIconStorage->fileFullName(AIcon)).toString())
				.arg(AMessage);
}

bool JingleRtp::writeCallMessageIntoChat(IMessageChatWindow *AWindow, CallType AType, IJingle::Reason AReason)
{
	Jid contactJid = AWindow->contactJid();
	Jid streamJid = AWindow->streamJid();
	QString sid = getSid(streamJid, contactJid);
	bool video = hasVideo(sid);
	bool outgoing = AType==Called?FJingle->isOutgoing(sid):false;

	IMessageStyleContentOptions options;
	options.time = QDateTime::currentDateTime();
	options.timeFormat = FMessageStyleManager->timeFormat(options.time);
	options.type = IMessageStyleContentOptions::TypeHistory;
	options.kind = IMessageStyleContentOptions::KindStatus;
	options.direction = outgoing?IMessageStyleContentOptions::DirectionOut
								:IMessageStyleContentOptions::DirectionIn;
	options.senderId  = streamJid.full();
	options.senderName   = HTML_ESCAPE(FMessageStyleManager->contactName(streamJid, contactJid));
	options.senderAvatar = FMessageStyleManager->contactAvatar(contactJid);

	QString message;	
	switch (AType)
	{
		case Called:
			message = video?chatNotification(outgoing?JNI_RTP_OUTGOING_VIDEO
													 :JNI_RTP_INCOMING_VIDEO,
											 tr("Video call"))
						   :chatNotification(outgoing?JNI_RTP_OUTGOING
													 :JNI_RTP_INCOMING,
											 tr("Voice call"));
			break;
		case Cancelled:
			message = chatNotification(JNI_RTP_HANGUP, tr("Call cancelled"));
			break;
		case Rejected:
			message = chatNotification(JNI_RTP_HANGUP, tr("Call rejected"));
			break;
		case Finished:
			message = chatNotification(JNI_RTP_HANGUP, tr("Call finished"));
			break;
		case Error:
			QString errorMessage = FJingle->errorMessage(AReason);
			message = chatNotification(JNI_RTP_ERROR,
									   errorMessage.isEmpty()?tr("Call error")
															 :tr("Call error (%1)")
															  .arg(errorMessage));
			options.status=IMessageStyleContentOptions::StatusError;
			break;
	}
	AWindow->viewWidget()->appendHtml(message, options);
	updateChatWindowActions(AWindow);
	return true;
}

void JingleRtp::updateChatWindowActions(IMessageChatWindow *AChatWindow)
{
	QList<QAction *> actions = AChatWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_JINGLE_RTP);
	if (isSupported(AChatWindow->streamJid(), AChatWindow->contactJid()))
	{
		Jid contactJid = AChatWindow->contactJid();
		Jid streamJid  = AChatWindow->streamJid();

		QString sid = getSid(streamJid, contactJid);
		IJingle::SessionStatus status=sid.isEmpty()?IJingle::None
												   :FJingle->sessionStatus(sid);

		if (actions.isEmpty())
		{
			Action *action = new Action(AChatWindow->toolBarWidget()->instance());
			action->setText(tr("Voice call"));

			if (status==IJingle::None || status==IJingle::Terminated)
				action->setIcon(FIconStorage->getIcon(JNI_RTP_CALL));
			else {
				bool video = hasVideo(sid);
				bool outgoing = FJingle->isOutgoing(sid);
				if (video)
					action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL);
				else
					action->setIcon(RSR_STORAGE_JINGLE, status==IJingle::Initiated||status==IJingle::Initiating?(outgoing?JNI_RTP_OUTGOING:JNI_RTP_INCOMING):
														status==IJingle::Accepted||status==IJingle::Accepting?JNI_RTP_CONNECT:JNI_RTP_TALK);
			}

			//action->setShortcutId(SCT_MESSAGEWINDOWS_SHOWVCARD);
			//action->setData(ADR_ACTION, ACT_CONST);
			action->setData(ADR_CONTACT_JID, contactJid.full());
			action->setData(ADR_STREAM_JID, streamJid.full());
			action->setData(ADR_COMMAND, VoiceCall);
			connect(action,SIGNAL(triggered()),SLOT(onCall()));
			AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_JINGLE_RTP);

//			Video is not supported yet
//			action = new Action(AChatWindow->toolBarWidget()->instance());
//			action->setText(tr("Video call"));
//			action->setIcon(FIconStorage->getIcon(JNI_RTP_CALL_VIDEO));
//			//action->setShortcutId(SCT_MESSAGEWINDOWS_SHOWVCARD);
//			//action->setData(ADR_ACTION, ACT_CONST);
//			action->setData(ADR_CONTACT_JID, contactJid.full());
//			action->setData(ADR_STREAM_JID, streamJid.full());
//			action->setData(ADR_COMMAND, VideoCall);
//			connect(action,SIGNAL(triggered()),SLOT(onCall()));
//			AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_JINGLE_RTP);

			action = new Action(AChatWindow->toolBarWidget()->instance());
			action->setText(tr("Hangup"));
			action->setIcon(FIconStorage->getIcon(JNI_RTP_HANGUP));
			//action->setShortcutId(SCT_MESSAGEWINDOWS_SHOWVCARD);
			//action->setData(ADR_ACTION, ACT_CONST);
			action->setData(ADR_CONTACT_JID, contactJid.full());
			action->setData(ADR_STREAM_JID, streamJid.full());
			action->setData(ADR_COMMAND, Hangup);
			action->setDisabled(status==IJingle::None || status==IJingle::Terminated);
			connect(action,SIGNAL(triggered()),SLOT(onHangup()));
			AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_JINGLE_RTP);
			AChatWindow->toolBarWidget()->toolBarChanger()->setSeparatorsVisible(true);
		}
		else
		{
			if (status==IJingle::None || status==IJingle::Terminated)
				for (QList<QAction *>::const_iterator it=actions.constBegin();
					 it!=actions.constEnd(); it++)
				{
					Action *action = AChatWindow->toolBarWidget()->toolBarChanger()->handleAction(*it);
					if (action)
					{
						action->setData(ADR_CONTACT_JID, contactJid.full());
						Command command = Command(action->data(ADR_COMMAND).toInt());
						if (command==VoiceCall) // Voice call
							action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL);
						else if (command==VideoCall) // Video call
							action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL_VIDEO);
						else if (command==Hangup)
							action->setEnabled(false);
					}
				}
			else
			{
				bool video = hasVideo(sid);
				bool outgoing = FJingle->isOutgoing(sid);
				for (QList<QAction *>::const_iterator it=actions.constBegin();
					 it!=actions.constEnd(); it++)
				{
					Action *action = AChatWindow->toolBarWidget()->toolBarChanger()->handleAction(*it);
					if (action)
					{
						action->setData(ADR_CONTACT_JID, contactJid.full());

						Command command = Command(action->data(ADR_COMMAND).toInt());
						if (command==VoiceCall) // Voice call
						{
							if (video)
								action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL);
							else
								action->setIcon(RSR_STORAGE_JINGLE, status==IJingle::Initiated||status==IJingle::Initiating?(outgoing?JNI_RTP_OUTGOING:JNI_RTP_INCOMING):
																	status==IJingle::Accepted||status==IJingle::Accepting?JNI_RTP_CONNECT:JNI_RTP_TALK);
						}
						else if (command==VideoCall) // Video call
						{
							if (video)
								action->setIcon(RSR_STORAGE_JINGLE, status==IJingle::Initiated||status==IJingle::Initiating?(outgoing?JNI_RTP_OUTGOING_VIDEO:JNI_RTP_INCOMING_VIDEO):
																	status==IJingle::Accepted||status==IJingle::Accepting?JNI_RTP_CONNECT_VIDEO:JNI_RTP_TALK_VIDEO);
							else
								action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL_VIDEO);
						}
						else if (command==Hangup)
							action->setEnabled(true);
					}
					else
						LOG_ERROR("Invalid action handle!");
				}
			}
		}
	}
	else
	{
		for (QList<QAction *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
		{
			AChatWindow->toolBarWidget()->toolBarChanger()->removeItem(*it);
			(*it)->deleteLater();
		}
	}
}

void JingleRtp::addPendingContent(IJingleContent *AContent, PendingType AType)
{
	FPendingContents.insert(AType, AContent);
}

void JingleRtp::removePendingContent(IJingleContent *AContent, PendingType AType)
{
	FPendingContents.remove(AType, AContent);
}

bool JingleRtp::hasPendingContents(const QString &ASid, PendingType AType)
{
	QList<IJingleContent *> pendingContents = FPendingContents.values(AType);
	for (QList<IJingleContent *>::ConstIterator it = pendingContents.constBegin();
		 it!=pendingContents.constEnd(); it++)
		if ((*it)->sid() == ASid)
			return true;
	return false;
}

void JingleRtp::establishConnection(const QString &ASid)
{
	LOG_DEBUG(QString("JingleRtp::establishConnection(%1)").arg(ASid));
	QHash<QString, IJingleContent *> contents = FJingle->contents(ASid);
	for (QHash<QString, IJingleContent *>::ConstIterator it=contents.constBegin();
		 it!=contents.constEnd(); it++)
	{
		LOG_DEBUG(QString("content=%1").arg((*it)->name()));
		if (FJingle->connectContent(ASid, it.key()))
			addPendingContent(*it, Connect);
	}
	if (!hasPendingContents(ASid, Connect))
		FJingle->sessionTerminate(ASid, IJingle::ConnectivityError);
}

void JingleRtp::checkRunningContents(const QString &ASid)
{
	bool pending(false);
	QHash<QString,IJingleContent*> contents = FJingle->contents(ASid);
	for (QHash<QString,IJingleContent*>::ConstIterator it=contents.constBegin();
		 it != contents.constEnd(); ++it) {
		if (FStreamers.contains(*it)) {
			pending = true;
			break;
		}

		if (FPlayers.contains(*it)) {
			pending = true;
			break;
		}
	}

	if (!pending) {
		LOG_DEBUG("No pending contents left!");
		QThread *ioThread(FIOThreads.value(ASid));
		if (ioThread)
		{
			LOG_DEBUG("Terminating I/O thread...");
			ioThread->quit();
		}
		else
			LOG_DEBUG("No I/O thread for the session");
		LOG_DEBUG("Destroying session...");
		FJingle->sessionDestroy(ASid);
	}
}

MediaStreamer *JingleRtp::startStreamMedia(const QPayloadType &APayloadType,
										   QIODevice *ARtpDevice)
{
	LOG_DEBUG(QString("JingleRtp::startStreamMedia(%1)")
			  .arg(APayloadType).arg(ARtpDevice->objectName()));

	int codecId = QPayloadType::idByName(APayloadType.name);

	// Now, let's start sending content
	if (codecId)
	{
		QAVCodec encoder = QAVCodec::findEncoder(codecId);
		if (encoder)
		{
			QVariantHash options;
			options.insert("payload_type", APayloadType.id);

			ARtpDevice->open(QIODevice::WriteOnly);
			MediaStreamer *streamer = new MediaStreamer(selectedAudioDevice(QAudio::AudioInput), encoder,
														ARtpDevice, "rtp", APayloadType.clockrate,
														Options::node(OPV_JINGLE_RTP_AUDIO_BITRATE)
														.value().toInt(), options, this);

			if (streamer->status() == MediaStreamer::Stopped)
			{
				connect(streamer, SIGNAL(statusChanged(int)),
								  SLOT(onStreamerStatusChanged(int)));

				ARtpDevice->setParent(streamer);
				streamer->setStatus(MediaStreamer::Running);
				FPluginManager->delayShutdown();
				return streamer;
			}
			else
			{
				LOG_ERROR(QString("MediaStreamer is not in Stopped state: %1")
						  .arg(streamer->status()));
				delete streamer;
			}
		}
		else
			LOG_ERROR(QString("Encoder not found! codec ID: %1").arg(codecId));
	}
	else
		LOG_ERROR(QString("codec name not found: %1").arg(APayloadType.name));

	delete ARtpDevice; // Don't need it anymore
	return nullptr;
}

MediaPlayer *JingleRtp::startPlayMedia(const QPayloadType &APayloadType,
									   QIODevice *ARtpIODevice)
{
	MediaPlayer *player = new MediaPlayer(selectedAudioDevice(QAudio::AudioOutput),
										  ARtpIODevice, APayloadType, this);	
	ARtpIODevice->setParent(player); // To delete it automaticaly once MediaPlayer is deleted
	ARtpIODevice->open(QIODevice::ReadOnly|QIODevice::WriteOnly);
	if (player->status() == MediaPlayer::Closed)
	{
		connect(player, SIGNAL(statusChanged(int,int)),
				SLOT(onPlayerStatusChanged(int,int)));

		if (player->setStatus(MediaPlayer::Running))
		{
			LOG_INFO("Player started successfuly!");
			FPluginManager->delayShutdown();
			return player;
		}
		else
			LOG_ERROR("Failed to start player!");
	}
	else
		LOG_ERROR(QString("MediaPlayer is not in Closed state: %1").arg(player->status()));
	delete player;
	return nullptr;
}

void JingleRtp::stopSessionMedia(const QString &ASid)
{
	QHash<QString, IJingleContent *> contents = FJingle->contents(ASid);
	for (QHash<QString, IJingleContent *>::ConstIterator it = contents.constBegin(); it!=contents.constEnd(); ++it)
	{
		MediaStreamer *streamer = FStreamers.value(*it);
		if (streamer)
		{
			if (streamer->status()==MediaStreamer::Running ||
				streamer->status()==MediaStreamer::Paused)
				streamer->setStatus(MediaStreamer::Stopped);
			else
			{
				FStreamers.remove(*it);
				delete streamer;
				FPluginManager->continueShutdown();
			}
		}

		MediaPlayer *player = FPlayers.value(*it);
		if (player)
		{
			if (player->status()==MediaPlayer::Running ||
				player->status()==MediaPlayer::Opened ||
				player->status()==MediaPlayer::Paused)
				player->setStatus(MediaPlayer::Finished);
			else
			{
				FPlayers.remove(*it);
				delete player;
				FPluginManager->continueShutdown();
			}
		}
	}
}

IMessageChatWindow *JingleRtp::chatWindow(const QString &ASid) const
{
	return FMessageWidgets->findChatWindow(FJingle->streamJid(ASid),
										   FJingle->contactJid(ASid));
}

QHash<int,QPayloadType> JingleRtp::payloadTypesFromDescription(const QDomElement &ADescription, QList<QPayloadType> *APayloadTypes)
{
	QHash<int,QPayloadType> result;
	if (ADescription.tagName()=="description" &&
		ADescription.namespaceURI()==NS_JINGLE_APPS_RTP)
	{
		QString mediaAttr = ADescription.attribute("media");
		QPayloadType::MediaType mediaType = mediaAttr=="audio"?QPayloadType::Audio:
											mediaAttr=="video"?QPayloadType::Video:
															   QPayloadType::Unknown;
		if (mediaType==QPayloadType::Audio) // Only Audio supported right now
			for (QDomElement element = ADescription.firstChildElement("payload-type"); !element.isNull(); element=element.nextSiblingElement("payload-type"))
				if (element.hasAttribute("id"))
				{
					bool ok;
					qint8 id = static_cast<qint8>(element.attribute("id").toInt(&ok));
					if (ok && (id<35 || (id>95 && element.hasAttribute("name") && element.hasAttribute("clockrate"))))
					{
						QPayloadType payloadType(id, element.attribute("name"), mediaType, element.attribute("clockrate").toInt(), element.attribute("channels").toInt(), element.attribute("ptime").toInt(), element.attribute("maxptime").toInt());
						if (payloadType.isValid())
						{
							QPayloadType tmp(payloadType);
							if (!tmp.isFilled())
								tmp.fill();
							int codecId = QPayloadType::idByName(tmp.name);
							if (codecId)
							{
								// Fill fmtp field with optional parameters
								for (QDomElement parameter=element.firstChildElement("parameter"); !parameter.isNull(); parameter=parameter.nextSiblingElement("parameter"))
									if (parameter.hasAttribute("name") && parameter.hasAttribute("value"))
										payloadType.fmtp.insert(parameter.attribute("name"), parameter.attribute("value"));
								result.insertMulti(codecId, payloadType);
								if (APayloadTypes)
									APayloadTypes->append(payloadType);
							}
						}
					}
				}
	}
	return result;
}

bool JingleRtp::fillDescriptionWithPayloadTypes(QDomElement &ADescription, const QList<QPayloadType> &APayloadTypes)
{
	if (ADescription.tagName()=="description" && ADescription.namespaceURI()==NS_JINGLE_APPS_RTP)
	{
		QString media = ADescription.attribute("media");
		QPayloadType::MediaType mediaType = media=="audio"?QPayloadType::Audio:
											media=="video"?QPayloadType::Video:
														   QPayloadType::Unknown;
		for (QList<QPayloadType>::ConstIterator it=APayloadTypes.constBegin(); it!=APayloadTypes.constEnd(); ++it)
			if ((*it).isValid() && (*it).id>=0)
			{
				QPayloadType tmp(*it);
				if (!tmp.isFilled())
				{
					if (!tmp.fill())
					{
						LOG_ERROR(QString("Failed to fill payloadType: %1").arg(*it));
						continue;
					}
				}
				if (tmp.media==mediaType)
				{
					QDomElement payloadType = ADescription.ownerDocument().createElement("payload-type");
					payloadType.setAttribute("id", QString::number((*it).id));

					if (!(*it).name.isEmpty())
						payloadType.setAttribute("name", (*it).name);
					if ((*it).clockrate)
						payloadType.setAttribute("clockrate", QString::number((*it).clockrate));
					if ((*it).channels)
						payloadType.setAttribute("channels", QString::number((*it).channels));
					if ((*it).ptime)
						payloadType.setAttribute("ptime", QString::number((*it).ptime));
					if ((*it).maxptime)
						payloadType.setAttribute("maxptime", QString::number((*it).maxptime));

					// Fill optional parameters
					for (QHash<QString,QString>::ConstIterator itf=(*it).fmtp.constBegin(); itf!=(*it).fmtp.constEnd(); ++itf)
					{
						QDomElement parameter = ADescription.ownerDocument().createElement("parameter");
						parameter.setAttribute("name", itf.key());
						parameter.setAttribute("value", itf.value());
						payloadType.appendChild(parameter);
					}
					ADescription.appendChild(payloadType);
				}
				else
					LOG_ERROR(QString("Wrong media type: %1").arg((*it).media));
			}
	}
	return false;
}

void JingleRtp::clearDescription(QDomElement &ADescription)
{
	for (QDomElement payloadType = ADescription.firstChildElement("payload-type");
		 !payloadType.isNull(); payloadType = ADescription.firstChildElement("payload-type"))
		ADescription.removeChild(payloadType).clear();
}

QAudioDeviceInfo JingleRtp::selectedAudioDevice(QAudio::Mode AMode)
{
	QList<QAudioDeviceInfo> devices(QAudioDeviceInfo::availableDevices(AMode));
	QString deviceName = Options::node(AMode==QAudio::AudioInput?
										   OPV_JINGLE_RTP_AUDIO_INPUT:
										   OPV_JINGLE_RTP_AUDIO_OUTPUT).value().toString();

	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin();
		it!=devices.constEnd(); ++it)
		if ((*it).deviceName()==deviceName)
			return *it;

	return AMode==QAudio::AudioInput?QAudioDeviceInfo::defaultInputDevice():QAudioDeviceInfo::defaultOutputDevice();
}

void JingleRtp::addPayloadType(IJingleContent *AContent, const QPayloadType &APayloadType)
{
	if (AContent)
	{
		QDomDocument document = AContent->document();
		QDomElement payloadType = document.createElement("payload-type");
		payloadType.setAttribute("id", QString::number(APayloadType.id));
		if (!APayloadType.name.isEmpty())
			payloadType.setAttribute("name", APayloadType.name);
		if (APayloadType.clockrate)
			payloadType.setAttribute("clockrate", QString::number(APayloadType.clockrate));
		if (APayloadType.channels)
			payloadType.setAttribute("channels", QString::number(APayloadType.channels));
		QDomElement description = AContent->description();
		description.appendChild(payloadType);
	}
}

IMessageChatWindow *JingleRtp::getWindow(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window(nullptr);
	if (AStreamJid.isValid() && AContactJid.isValid())
		if (FMessageWidgets)
		{
			window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
			if(!window)
				if (FMessageProcessor)
				{
					FMessageProcessor->getMessageWindow(AStreamJid,AContactJid,Message::Chat,IMessageProcessor::ActionAssign);
					window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
					FPendingChats.append(window);
				}

			if (window) {
				window->address()->setAddress(AStreamJid, AContactJid);
				connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onTabPageActivated()));
			}
		}
	return window;
}

void JingleRtp::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindowActions(AWindow);
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)),
											SLOT(onAddressChanged(Jid,Jid)));
}

void JingleRtp::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(),
															   address->contactJid());
	if (window)
		updateChatWindowActions(window);
}

void JingleRtp::onStreamerStatusChanged(int AStatus)
{
	LOG_DEBUG(QString("JingleRtp::onStreamerStatusChanged(%1)").arg(AStatus));

	MediaStreamer *streamer = qobject_cast<MediaStreamer *>(sender());
	IJingleContent *content = FStreamers.key(streamer);
	switch (AStatus)
	{
		case MediaStreamer::Running:
			break;

		case MediaStreamer::Error:
		{
			if (streamer)
			{
				LOG_DEBUG("Terminating session...");
				FJingle->sessionTerminate(content->sid(), IJingle::FailedApplication);
				LOG_DEBUG("Removing streamer...");
				FStreamers.remove(content);
				delete streamer;
				FPluginManager->continueShutdown();
			}
			break;
		}
		case MediaStreamer::Stopped:
		{
			if (streamer)
			{
				LOG_DEBUG("Removing streamer...");
				FStreamers.remove(content);
				delete streamer;

				QString sid(content->sid());
				if (FJingle->sessionStatus(sid) == IJingle::Connected) {
					LOG_DEBUG("Session is Running! Terminating...");
					FJingle->sessionTerminate(sid, IJingle::FailedApplication);
				} else if (FJingle->sessionStatus(sid) == IJingle::Terminated)
					checkRunningContents(sid);
				else
					LOG_ERROR(QString("Wrong session status: %1")
							  .arg(FJingle->sessionStatus(sid)));

				FPluginManager->continueShutdown();
			}
			break;
		}

		default:
			break;
	}
}

void JingleRtp::onPlayerStatusChanged(int AStatusNew, int AStatusOld)
{
	LOG_DEBUG(QString("JingleRtp::onPlayerStatusChanged(%1, %2)")
			  .arg(AStatusNew).arg(AStatusOld));

	MediaPlayer *player = qobject_cast<MediaPlayer*>(sender());
	IJingleContent *content = FPlayers.key(player);

	switch (AStatusNew)
	{
		case MediaPlayer::Running:
		{
			LOG_DEBUG("Running!");
			IMessageChatWindow *window = chatWindow(content->sid());
			if (window)
				updateChatWindowActions(window);
			break;
		}

		case MediaPlayer::Finished:
		{
			LOG_DEBUG("Finished!");
			if (player)
			{
				LOG_DEBUG("Removing player...");
				FPlayers.remove(content);
				delete player;
				FPluginManager->continueShutdown();
			}

			QString sid(content->sid());
			if (FJingle->sessionStatus(sid) == IJingle::Connected) {
				LOG_DEBUG("Session is Running! Terminating...");
				FJingle->sessionTerminate(sid, IJingle::MediaError);
			} else if (FJingle->sessionStatus(sid) == IJingle::Terminated)
				checkRunningContents(sid);
			else
				LOG_ERROR(QString("Wrong session status: %1")
						  .arg(FJingle->sessionStatus(sid)));
			break;
		}

		case MediaPlayer::Error:
		{
			LOG_DEBUG("Error!");
			if (player)
			{
				LOG_DEBUG("Terminating session...");
				FJingle->sessionTerminate(content->sid(), IJingle::FailedApplication);
				LOG_DEBUG("Removing player...");
				FPlayers.remove(content);
			}
			break;
		}
	}
}

void JingleRtp::onCall()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid  = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();

		QString sid;

		QList<QString> sids = FSidHash.keys(contactJid);
		for (QList<QString>::ConstIterator it=sids.constBegin(); it!=sids.constEnd(); ++it)
			if (FJingle->streamJid(*it)==streamJid)
			{
				sid = *it;
				break;
			}

		if (!sid.isEmpty())
		{
			if (!FJingle->isOutgoing(sid))
			{
				IJingle::Reason reason(IJingle::NoReason);
				QHash<QString, IJingleContent*> contents = FJingle->contents(sid);
				for (QHash<QString, IJingleContent*>::ConstIterator it=contents.constBegin(); it!=contents.constEnd(); it++)
				{
					if (checkContent(*it))
					{
						if (FJingle->fillIncomingTransport(*it))
							addPendingContent(*it, FillTransport);
						else
							reason = IJingle::FailedTransport;
					}
					else
						reason = IJingle::FailedApplication;
				}
				if (!hasPendingContents(sid, FillTransport))
					FJingle->sessionTerminate(sid, reason);
				else {
					FJingle->setAccepting(sid);
					IMessageChatWindow *window = chatWindow(sid);
                    removeNotification(window);
					updateChatWindowActions(window);
				}
			}
			else
				LOG_WARNING(QString("Session exists already! sid=%1").arg(sid));
			return; // Session exists already
		}

		sid = FJingle->sessionCreate(streamJid, contactJid, NS_JINGLE_APPS_RTP);
		if (!sid.isNull())
		{
			QSet<int> ids;
			QByteArray tmp;
			QBuffer device(&tmp);
			IJingleContent *content = FJingle->contentAdd(sid, "voice", "audio",
														  Options::node(OPV_JINGLE_RTP_RTCP).value().toBool()?2:1,
														  IJingleTransport::Datagram, false);
			if (content)
			{
				QAudioDeviceInfo inputDevice = selectedAudioDevice(QAudio::AudioInput);
				int bitrate = Options::node(OPV_JINGLE_RTP_AUDIO_BITRATE).value().toInt();
				QDomElement description(content->description());
				QList<int> codecIds = intsFromString(Options::node(OPV_JINGLE_RTP_CODECS_USED).value().toString());
				QList<QPayloadType> payloadTypes;
				bool payloadTypeIdsExceeded(false);
				for (QList<int>::ConstIterator it=codecIds.constBegin(); it!=codecIds.constEnd(); ++it)
				{
					QAVCodec encoder = QAVCodec::findEncoder(*it);
					QList<int> sampleRates = encoder.supportedSampleRates();
					if (sampleRates.isEmpty())
						sampleRates=inputDevice.supportedSampleRates();
					for (QList<int>::ConstIterator itr = sampleRates.constBegin();
						 itr!=sampleRates.constEnd(); ++itr)
					{
						MediaStreamer *streamer = new MediaStreamer(inputDevice, encoder, &device,
																	"rtp", *itr, bitrate,
																	QVariantHash(), this);

						if (streamer->status()==MediaStreamer::Stopped)
						{
							QPayloadType payloadType(QPayloadType::fromSdp(streamer->getSdpString()));
							if (payloadType.isValid() && !payloadTypes.contains(payloadType))
							{
								if (payloadType.id>95)
								{
									while (ids.contains(payloadType.id) && payloadType.id>95)
										payloadType.id++;
									if (payloadType.id>95)
										ids.insert(payloadType.id);
								}
								if (payloadType.id>=0)
									payloadTypes.append(payloadType);
								else
									payloadTypeIdsExceeded=true;
							}
						}
						else
							LOG_WARNING(QString("Streamer status is: %1").arg(streamer->status()));
						delete streamer;
						if (payloadTypeIdsExceeded)
							break;
					}
					if (payloadTypeIdsExceeded)
					{
						LOG_WARNING("Dynamic payload type IDs exceeded!");
						break;
					}
				}

				if (!payloadTypes.isEmpty())
				{
					fillDescriptionWithPayloadTypes(description, payloadTypes);
					addPendingContent(content, AddContent);
//				Video is not supported yet
//				if (command==VideoCall)
//				{   // Add video content
//					content = FJingle->contentAdd(streamJid, sid, "video", "video", NS_JINGLE_TRANSPORTS_RAW_UDP, false);
//					addPendingContent(content, AddContent);
//				}
					putSid(contactJid, sid);
//					FJingle->sessionInitiate(sid);
					callChatMessage(sid, Called);
				}
				else
					LOG_ERROR("No payload types available!");
			}
		}
	}
}

void JingleRtp::onHangup()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid  = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();

		QString sid;

		QList<QString> sids = FSidHash.keys(contactJid);
		for (QList<QString>::ConstIterator it=sids.constBegin(); it!=sids.constEnd(); ++it)
			if (FJingle->streamJid(*it)==streamJid)
			{
				sid = *it;
				break;
			}

		if (!sid.isEmpty())
		{
			IJingle::SessionStatus status = FJingle->sessionStatus(sid);
			IJingle::Reason reason;
			switch (status)
			{
				case IJingle::Connected:
					reason = IJingle::Success;
					break;

				case IJingle::Initiating:
				case IJingle::Initiated:
					reason = FJingle->isOutgoing(sid)?IJingle::Cancel:IJingle::Decline;
					break;

				default:
					reason = IJingle::Cancel;
					break;
			}

			FJingle->sessionTerminate(sid, reason);
		}
	}
}

void JingleRtp::onRtpReadyRead()
{
	QIODevice *rtpDevice = qobject_cast<QIODevice *>(sender());
	if (rtpDevice)	// sender() may be QObject(0x0), if SIGNAL
	{				// is disconnected from the SLOT already!
		IJingleContent *content = FContents.take(rtpDevice);
		Q_ASSERT(content);
		rtpDevice->disconnect(SIGNAL(readyRead()), this, SLOT(onRtpReadyRead()));
		checkRtpContent(content, rtpDevice);
	}
}

void JingleRtp::onConnectionEstablished(IJingleContent *AContent)
{
	LOG_DEBUG(QString("JingleRtp::onConnectionEstablished(%1)").arg(AContent->name()));	
	removePendingContent(AContent, Connect);
	if (!hasPendingContents(AContent->sid(), Connect))
		FJingle->setConnected(AContent->sid());
	else
		LOG_DEBUG("More contents pending!");
}

void JingleRtp::onConnectionFailed(IJingleContent *AContent)
{
	LOG_DEBUG(QString("JingleRtp::onConnectionFailed(%1)").arg(AContent->name()));
	removePendingContent(AContent, Connect);
	if (!hasPendingContents(AContent->sid(), Connect))
	{
		if (FJingle->contents(AContent->sid()).isEmpty())
			FJingle->sessionTerminate(AContent->sid(), IJingle::ConnectivityError);
		else
			FJingle->setConnected(AContent->sid());
	}
	LOG_DEBUG("JingleRtp::onConnectionFailed() finished!");
}

void JingleRtp::onContentAdded(IJingleContent *AContent)
{
	removePendingContent(AContent, AddContent);
	if (!hasPendingContents(AContent->sid(), AddContent))
		FJingle->sessionAccept(AContent->sid());
}

void JingleRtp::onContentAddFailed(IJingleContent *AContent)
{
	removePendingContent(AContent, AddContent);
	if (!hasPendingContents(AContent->sid(), AddContent))
	{
		if (FJingle->contents(AContent->sid()).isEmpty())
			FJingle->sessionTerminate(AContent->sid(), IJingle::FailedTransport);
		else
			FJingle->sessionAccept(AContent->sid());
	}
}

void JingleRtp::onContactStateChanged(const Jid &AStreamJid, const Jid &AContactJid, bool AStateOnline)
{
	if (!AStateOnline)
	{
		QList<int> notifies;
		for (QMap<int, QPair<Jid,Jid> >::ConstIterator it = FNotifies.constBegin();
			 it != FNotifies.constEnd(); ++it)
			if ((*it).first == AStreamJid && (*it).second == AContactJid)
				notifies.append(it.key());

		for (QList<int>::ConstIterator it = notifies.constBegin();
			 it != notifies.constEnd(); ++it)
			FNotifications->removeNotification(*it);
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleRtp,JingleRtp)
#endif

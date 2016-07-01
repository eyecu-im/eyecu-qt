#include <QDebug>
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
		QProcess::startDetached(Options::node(OPV_NOTIFICATIONS_SOUNDCOMMAND).value().toString(),QStringList()<<soundFile);
#endif
}

// JingleRtp

const QString JingleRtp::types[]={"Active","Hold","Mute","Ringing"};

JingleRtp::JingleRtp():
	FJingle(NULL),
	FServiceDiscovery(NULL),
	FOptionsManager(NULL),
	FMessageWidgets(NULL),
	FMessageStyleManager(NULL),
	FMessageProcessor(NULL),
	FNotifications(NULL),
	FAvatars(NULL),
	FStatusIcons(NULL),
	FIconStorage(NULL),
	FJingleRtpOptions(NULL),
	FCallTimer(NULL)
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
	qDebug() << "JingleRtp::intsFromString(" << FString << ")";
	QList<int> result;
	QStringList strings = FString.split(';');
	for (QStringList::ConstIterator it=strings.constBegin(); it!=strings.constEnd(); ++it)
		result.append((*it).toInt());
	return result;
}

void JingleRtp::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Jingle RTP");
	APluginInfo->description = tr("Implements XEP-0167: Jingle RTP Sessions");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(JINGLE_UUID);
}

bool JingleRtp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin= APluginManager->pluginInterface("IJingle").value(0,NULL);
	if (plugin)
	{
		FJingle = qobject_cast<IJingle *>(plugin->instance());
		connect(FJingle->instance(),SIGNAL(incomingTransportFilled(IJingleContent *)),SLOT(onIncomingTransportFilled(IJingleContent*)), Qt::QueuedConnection);
		connect(FJingle->instance(),SIGNAL(incomingTransportFillFailed(IJingleContent *)),SLOT(onIncomingTransportFillFailed(IJingleContent*)), Qt::QueuedConnection);
	}
	else
		return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
	if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IAvatars").value(0,NULL);
	if (plugin)
		FAvatars = qobject_cast<IAvatars *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IStatusIcons").value(0,NULL);
	if (plugin)
		FStatusIcons = qobject_cast<IStatusIcons *>(plugin->instance());

	plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
	if (plugin)
	{
		FNotifications = qobject_cast<INotifications *>(plugin->instance());
		if (FNotifications)
		{
			connect(FNotifications->instance(),SIGNAL(notificationActivated(int)), SLOT(onNotificationActivated(int)));
			connect(FNotifications->instance(),SIGNAL(notificationRemoved(int)), SLOT(onNotificationRemoved(int)));
		}
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
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
		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)->getIcon(JNI_RTP_CALL);
		notifyType.title = tr("When incoming voice or video call received");
		notifyType.kindMask = INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction|
							  INotification::PopupWindow|INotification::SoundPlay|INotification::AlertWidget|
							  INotification::TabPageNotify|INotification::ShowMinimized|INotification::AutoActivate;
		notifyType.kindDefs = notifyType.kindMask & ~(INotification::AutoActivate);
		FNotifications->registerNotificationType(NNT_JINGLE_RTP_CALL, notifyType);

		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_JINGLE)->getIcon(JNI_RTP_HANGUP);
		notifyType.title = tr("When incoming voice or video call missed");
		FNotifications->registerNotificationType(NNT_JINGLE_RTP_MISSED, notifyType);

		FNotifications->insertNotificationHandler(NHO_DEFAULT, this);
	}

	QPayloadType::initialize();
	QAVCodec::registerAll();
	QAVFormat::registerAll();
	QAVFormat::networkInit();

	FSupportdCodecNames = QPayloadType::names(true);

	return true;
}

bool JingleRtp::initSettings()
{
//	QList<PayloadType> payloadTypes;
//	payloadTypes.append(PayloadType(-1, "G726-40", PayloadType::Audio, 8000, 1));
//	payloadTypes.append(PayloadType(-1, "G726-32", PayloadType::Audio, 8000, 1));
//	payloadTypes.append(PayloadType(-1, "G726-24", PayloadType::Audio, 8000, 1));
//	payloadTypes.append(PayloadType(-1, "G726-16", PayloadType::Audio, 8000, 1));
//	payloadTypes.append(PayloadType(-1, "L8", PayloadType::Audio, 0, 0));
//	payloadTypes.append(PayloadType(-1, "speex", PayloadType::Audio, 8000, 1));
//	payloadTypes.append(PayloadType(-1, "speex", PayloadType::Audio, 16000, 1));
//	payloadTypes.append(PayloadType(-1, "opus", PayloadType::Audio, 48000, 1));
//	payloadTypes.append(PayloadType(-1, "opus", PayloadType::Audio, 48000, 2));
//	Options::setDefaultValue(OPV_JINGLE_RTP_PT_DYNAMIC, stringsFromAvps(payloadTypes));	//Dynamic payload types
//	payloadTypes.clear();
//	payloadTypes.append(PayloadType(-1, "opus", PayloadType::Audio, 48000, 1));
//	payloadTypes.append(PayloadType(-1, "speex", PayloadType::Audio, 8000, 1));
//	payloadTypes.append(PayloadType(-1, "speex", PayloadType::Audio, 16000, 1));
//	payloadTypes.append(PayloadType(9, "G722", PayloadType::Audio, 8000, 1));
	Options::setDefaultValue(OPV_JINGLE_RTP_CODECS_USED, stringFromInts(QList<int>() << QAVCodec::AvCodecIdOpus << QAVCodec::AvCodecIdSpeex << QAVCodec::AvCodecIdADPCMG722));	//Used codecs
	Options::setDefaultValue(OPV_JINGLE_RTP_AUDIO_BITRATE, 32000);    //Audio encoding bitrate
	Options::setDefaultValue(OPV_JINGLE_RTP_AUDIO_INPUT, QVariant()); //Audio input device
	Options::setDefaultValue(OPV_JINGLE_RTP_AUDIO_OUTPUT, QVariant()); //Audio output device
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

void JingleRtp::onSessionInitiated(const Jid &AStreamJid, const QString &ASid)
{
	qDebug() << "JingleRtp::onSessionInitiated(" << AStreamJid.full() << ", " << ASid << ")";
	Jid contactJid=FJingle->contactJid(AStreamJid, ASid);
	putSid(AStreamJid,contactJid, ASid);
	writeCallMessageIntoChat(getWindow(AStreamJid, contactJid), Called);
	callNotify(AStreamJid, ASid, Called);
}

void JingleRtp::onActionAcknowledged(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus APreviousStatus, const Jid &ARedirect, IJingle::Reason AReason)
{
	qDebug() << "JingleRtp::onActionAcknowleged(" << AStreamJid.full() << "," << ASid << "," << AAction << "," << ARespond << "," << APreviousStatus << "," << ARedirect.full() << "," << AReason << ")";
	switch (ARespond)
	{
		case IJingle::Acknowledge:
		{
			Jid contactJid=FJingle->contactJid(AStreamJid, ASid);
			IMessageChatWindow *window=FMessageWidgets->findChatWindow(AStreamJid, contactJid);
			switch (AAction)
			{
				case IJingle::SessionInitiate:
					callChatMessage(AStreamJid, ASid, Called);
					break;

				case IJingle::SessionAccept:
					removeNotification(window);
					updateWindowActions(window);
					establishConnection(AStreamJid, ASid);
					break;

				case IJingle::SessionTerminate:
				{
					removeNotification(window);
					updateWindowActions(window);
					CallType type;
					switch (AReason)
					{
						case IJingle::Success:
							type=Finished;
							break;
						case IJingle::Decline:
							type=Rejected;
							break;
						case IJingle::Cancel:
							type=Cancelled;
							break;
						default:
							type=Error;
					}

					callChatMessage(AStreamJid, ASid, type, AReason);
					removeSid(AStreamJid, ASid);
					break;
				}

				default:
					break;
			}
			break;
		}
		case IJingle::ServiceUnavailable:
			qWarning() << "Service unavailable";
			removeSid(AStreamJid, ASid);
			callChatMessage(AStreamJid, ASid, Error);
			break;
		case IJingle::Redirect:
			qWarning() << "Redirected: " << ARedirect.full();
			removeSid(AStreamJid, ASid);
			callChatMessage(AStreamJid, ASid, Error);
			break;
		case IJingle::ResourceConstraint:
			qWarning() << "Resource constraint";
			removeSid(AStreamJid, ASid);
			callChatMessage(AStreamJid, ASid, Error);
			break;
		case IJingle::BadRequest:
			qWarning() << "Bad request";
			removeSid(AStreamJid, ASid);
			callChatMessage(AStreamJid, ASid, Error);
			break;
	}
}

void JingleRtp::onSessionAccepted(const Jid &AStreamJid, const QString &ASid)
{
	qDebug() << "JingleRtp::onSessionAccepted(" << AStreamJid.full() << "," << ASid << ")";
	Jid contactJid=FJingle->contactJid(AStreamJid, ASid);
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(AStreamJid, contactJid);
	if (window)
		updateWindowActions(window);
	establishConnection(AStreamJid, ASid);
}

void JingleRtp::onSessionConnected(const Jid &AStreamJid, const QString &ASid)
{
	LOG_DEBUG(QString("JingleRtp::onSessionConnected(%1, %2)").arg(AStreamJid.full()).arg(ASid));
	bool success(false);
	QHash<QString, IJingleContent *> contents = FJingle->contents(AStreamJid, ASid);
	for (QHash<QString, IJingleContent *>::ConstIterator it=contents.constBegin(); it!=contents.constEnd(); it++)
	{
		LOG_DEBUG(QString("content: \"%1\"").arg((*it)->name()));
		QList<QDomElement> candidates = (*it)->candidates();
		for (QList<QDomElement>::ConstIterator it1 = candidates.constBegin(); it1!=candidates.constEnd(); it1++)
		{
			QString id = (*it1).attribute("id");

			QUdpSocket *outputSocket = qobject_cast<QUdpSocket *>((*it)->outputDevice(id));
			if (outputSocket)
			{
				LOG_DEBUG(QString("output socket state: %1").arg(outputSocket->state()));
				LOG_DEBUG(QString("output socket open mode: %1").arg(outputSocket->openMode()));

				QDomElement description = (*it)->description();
				QDomElement payloadType = description.firstChildElement("payload-type");

				int ptid = payloadType.attribute("id").toInt();
				QPayloadType avp;
				if (ptid < 96)
					avp = QPayloadType::payloadType(ptid);

				if (payloadType.hasAttribute("name"))
					avp.name = payloadType.attribute("name");

				if (payloadType.hasAttribute("clockrate"))
					avp.clockrate = payloadType.attribute("clockrate").toInt();

				if (payloadType.hasAttribute("channels"))
					avp.channels = payloadType.attribute("channels").toInt();
				else if (!avp.channels)
					avp.channels = 1;

				QString media = description.attribute("media");

				avp.media = media=="audio"?QPayloadType::Audio:
							media=="video"?QPayloadType::Video:
										   QPayloadType::Unknown;

				avp.id = ptid;

				MediaStreamer *sender = startSendMedia(avp, outputSocket);
				if (sender)
				{
					FSenders.insert(*it, sender);
					success = true;
				}
				else
					LOG_FATAL("Failed to start send media");

				outputSocket->close();	// Don't need it anymore
			}
			else
				LOG_FATAL("Output device is NOT a UDP socket!");
		}
	}
	if (!success)
	{
		LOG_WARNING("No successful contents! Terminating session...");
		FJingle->sessionTerminate(AStreamJid, ASid, IJingle::FailedApplication);
	}
}

void JingleRtp::onSessionTerminated(const Jid &AStreamJid, const QString &ASid, IJingle::SessionStatus APreviousStatus, IJingle::Reason AReason)
{
	qDebug() << "JingleRtp::onSessionTerminated(" << AStreamJid.full() << "," << ASid << "," << APreviousStatus << "," << AReason << "," << ")";
	CallType type;
	switch (AReason)
	{
		case IJingle::Success:
			type=Finished;
			break;
		case IJingle::Decline:
		{
			type=Rejected;
			IMessageChatWindow *window=FMessageWidgets->findChatWindow(AStreamJid, FJingle->contactJid(AStreamJid, ASid));
			if (window)
			{
				removeNotification(window);
				updateWindowActions(window);
			}
			break;
		}
		case IJingle::Cancel:
			type=Cancelled;
			break;
		default:
			type=Error;
	}

	qDebug() << "Terminating contents...";
	QHash<QString, IJingleContent *> contents = FJingle->contents(AStreamJid, ASid);
	for (QHash<QString, IJingleContent *>::ConstIterator it = contents.constBegin(); it!=contents.constEnd(); ++it)
	{
		qDebug() << "content:" << *it;
		MediaStreamer *sender = FSenders.value(*it);
		if (sender)
		{
			qDebug() << "sender found:" << sender << "; status=" << sender->status();
			if (sender->status()==MediaStreamer::Running ||
				sender->status()==MediaStreamer::Started ||
				sender->status()==MediaStreamer::Paused)
				sender->stop();
			else
				delete sender;
		}

		MediaPlayer *streamer = FStreamers.value(*it);
		if (streamer)
		{
			qDebug() << "streamer found:" << sender << "; status=" << streamer->status();
			if (streamer->status()==MediaPlayer::Running ||
				streamer->status()==MediaPlayer::Opened ||
				streamer->status()==MediaPlayer::Paused)
				streamer->setStatus(MediaPlayer::Finished);
			else
				delete streamer;
		}
	}

	connectionTerminated(AStreamJid, ASid);

	callChatMessage(AStreamJid, ASid, type, AReason);

	if (type == Cancelled && APreviousStatus == IJingle::Initiated && !FJingle->isOutgoing(AStreamJid, ASid))
		callNotify(AStreamJid, ASid, type);

	removeSid(AStreamJid, ASid);
}

void JingleRtp::onSessionInformed(const QDomElement &AInfoElement)
{
	LOG_DEBUG(QString("JingleRtp::onSessionInformed(%1)").arg(AInfoElement.tagName()));
}

void JingleRtp::onDataReceived(const Jid &AStreamJid, const QString &ASid, QIODevice *ADevice)
{
	qDebug() << "JingleRtp::onDataReceived(" << AStreamJid.full() << "," << ASid << "," << ADevice << ")";

	qDebug() << "ADevice->openMode()=" << ADevice->openMode();
	IJingleContent *content = FJingle->content(AStreamJid, ASid, ADevice);
	if (content)
	{
		QUdpSocket *socket = qobject_cast<QUdpSocket*>(ADevice);
		if (socket)
		{
			qDebug() << "socket state:" << socket->state();
			char data[64];
			quint64 size = socket->readDatagram(data, 64);
			if (size>1)
			{
				int payloadTypeId = data[1]&0x7F;
				qDebug() << "payloadTypeId=" << payloadTypeId;

				QDomElement description(content->description());
				if (description.hasAttribute("media"))
				{
					QString media = description.attribute("media");
					QPayloadType::MediaType mediaType = media=="audio"?QPayloadType::Audio:
												media=="video"?QPayloadType::Video:
															   QPayloadType::Unknown;
					for (QDomElement payloadType = description.firstChildElement("payload-type"); !payloadType.isNull(); payloadType=payloadType.nextSiblingElement("payload-type"))
					{
						if (payloadType.attribute("id").toInt()==payloadTypeId)
						{
							QHostAddress address = socket->localAddress();
							quint16	port = socket->localPort();
							socket->disconnectFromHost();
							QPayloadType pt = buildPayloadType(payloadType, mediaType);
							MediaPlayer *streamer = startPlayMedia(pt, address, port);
							if (streamer)
								FStreamers.insert(content, streamer);
							else
								LOG_ERROR("Failed to start media play!");
						}
					}
				}
			}
		}
		else
			LOG_FATAL("Not a UDP socket!");
	}
	else
		LOG_FATAL("Content not found");
}


INotification JingleRtp::callNotify(const Jid &AStreamJid, const QString &ASid, CallType ACallType)
{
	INotification notification;

	Jid contactJid=FJingle->contactJid(AStreamJid, ASid);

	IMessageChatWindow *window = getWindow(AStreamJid, contactJid);

	// Remove existing notification for the window, if any
	int notify=FNotifies.key(window);
	if (notify)
		FNotifications->removeNotification(notify);

	if (window && (!window->isActiveTabPage() || ACallType==Called))
	{
		bool video=hasVideo(AStreamJid, ASid);
		notification.kinds = FNotifications->enabledTypeNotificationKinds(ACallType==Called?NNT_JINGLE_RTP_CALL:NNT_JINGLE_RTP_MISSED);
		if (notification.kinds > 0)
		{
			QIcon icon = FIconStorage->getIcon(ACallType==Called?(video?JNI_RTP_CALL_VIDEO:JNI_RTP_CALL)
																	   :JNI_RTP_HANGUP);
			QString name = FNotifications->contactName(AStreamJid, contactJid);

			notification.typeId = ACallType==Called?NNT_JINGLE_RTP_CALL:NNT_JINGLE_RTP_MISSED;
			notification.data.insert(NDR_JINGLE_RTP_EVENT_TYPE, ACallType);
			notification.data.insert(NDR_ICON, icon);
			QString tooltip=(ACallType==Called)?tr("Incoming %1 call from %2")
											   :tr("Missed %1 call from %2");
			notification.data.insert(NDR_TOOLTIP, tooltip.arg(video?tr("video"):tr("voice")).arg(name));
			notification.data.insert(NDR_STREAM_JID, AStreamJid.full());
			notification.data.insert(NDR_CONTACT_JID, contactJid.full());

			// Popup data - used by AttentionDialog
			if (FAvatars)
			{
				QString avatarFileName=FAvatars->avatarFileName(FAvatars->avatarHash(contactJid));
				if (!avatarFileName.isEmpty())
					notification.data.insert(NDR_POPUP_IMAGE, avatarFileName);
			}
			notification.data.insert(NDR_POPUP_CAPTION, ACallType==Cancelled?tr("Missed call!"):tr("Incoming call!"));
			notification.data.insert(NDR_POPUP_TITLE, name);
			notification.data.insert(NDR_POPUP_HTML,HTML_ESCAPE_CHARS("Test"));
			notification.data.insert(NDR_ROSTER_ORDER, RNO_JINGLE_RTP);
			notification.data.insert(NDR_ROSTER_FLAGS, IRostersNotify::Blink|
													   IRostersNotify::AllwaysVisible|
													   IRostersNotify::HookClicks);
			notification.data.insert(NDR_ROSTER_CREATE_INDEX, true);
			notification.data.insert(NDR_SOUND_FILE, ACallType==Called?SDF_JINGLE_RTP_CALL
																	  :SDF_SCHANGER_CONNECTION_ERROR);

			notification.data.insert(NDR_ALERT_WIDGET, (qint64)window->instance());
			notification.data.insert(NDR_TABPAGE_WIDGET, (qint64)window->instance());
			notification.data.insert(NDR_TABPAGE_PRIORITY, TPNP_JINGLE_RTP);
			notification.data.insert(NDR_TABPAGE_ICONBLINK, true);
			notification.data.insert(NDR_SHOWMINIMIZED_WIDGET, (qint64)window->instance());

			updateWindow(window);
			int notify=FNotifications->appendNotification(notification);
			FNotifies.insert(notify, window);

			if (ACallType==Called && notification.kinds&INotification::SoundPlay)
				sessionInfo(AStreamJid, contactJid, Ringing);
		}
	}
	return notification;
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
}

bool JingleRtp::hasVideo(const Jid &AStreamJid, const QString &ASid) const
{
	return false; // Video is not supported yet
	QHash<QString, IJingleContent *> contents=FJingle->contents(AStreamJid, ASid);
	for (QHash<QString, IJingleContent *>::const_iterator it=contents.constBegin(); it!=contents.constEnd(); it++)
		if ((*it)->description().attribute("media")=="video")
			return true;
	return false;
}

void JingleRtp::onNotificationActivated(int ANotifyId)
{
	if (FNotifies.contains(ANotifyId))
	{
		IMessageChatWindow *window=FNotifies.value(ANotifyId);
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
	int notify=FNotifies.key(window);
	if (notify)
	{
		INotification notification=FNotifications->notificationById(notify);
		CallType type=(CallType)notification.data.value(NDR_JINGLE_RTP_EVENT_TYPE).toInt();
		if (type!=Called)                   // All the events but incoming Call
			removeNotification(window);     // should be removed immediately!
	}
	if (FPendingChats.contains(window))
	{
		updateWindowActions(window);
		FPendingChats.removeAll(window);
	}
}

void JingleRtp::removeNotification(IMessageChatWindow *AWindow)
{
	qDebug() << "JingleRtp::removeNotification(" << AWindow << ")";
	if (AWindow)
	{
		int notify=FNotifies.key(AWindow, 0);
		if (notify)
		{
			FNotifications->removeNotification(notify);
			updateWindow(AWindow);
		}
	}
}

void JingleRtp::removeNotification(const Jid &AStreamJid, const QString &ASid)
{
	removeNotification(FMessageWidgets->findChatWindow(AStreamJid, FJingle->contactJid(AStreamJid, ASid)));
}

bool JingleRtp::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FServiceDiscovery==NULL || !FServiceDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
			|| FServiceDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_JINGLE_APPS_RTP);
}

bool JingleRtp::checkContent(IJingleContent *AContent)
{
	LOG_DEBUG("JingleRtp::checkContent()");

	QList<QDomElement> unsupportedPayloadTypes;
	QDomElement description(AContent->description());

	for (QDomElement pt = description.firstChildElement("payload-type"); !pt.isNull(); pt=pt.nextSiblingElement("payload-type"))
	{
		bool supported(false);
		if (pt.hasAttribute("id"))
		{
			QString name;
			if (pt.hasAttribute("name"))
				name = pt.attribute("name");
			else
			{
				bool ok;
				int id = pt.attribute("id").toInt(&ok);
				if (ok)
				{
					if (id<96)
					{
						QPayloadType payloadType = QPayloadType::payloadType(id);
						if (payloadType.isValid())
							name = payloadType.name;
					}
					else
						LOG_WARNING("Dynamic payload types must have \"name\" attribute!");
				}
				else
					LOG_WARNING("Invalid id attribute of <payload-type/> element!");

			}
			if (!name.isEmpty())
				supported = FSupportdCodecNames.contains(name);
		}
		else
			LOG_WARNING("Invalid <payload-type/> element!");

		if (!supported)
			unsupportedPayloadTypes.append(pt);
	}

	LOG_INFO(QString("Unsupported payload types found: %1").arg(unsupportedPayloadTypes.size()));

	for (QList<QDomElement>::ConstIterator it = unsupportedPayloadTypes.constBegin(); it!=unsupportedPayloadTypes.constEnd(); ++it)
		description.removeChild(*it);

	bool rc = !description.firstChildElement("payload-type").isNull();

	LOG_DEBUG(QString("JingleRtp::checkContent() returns: %1").arg(rc));

	return rc;
}

QString JingleRtp::getSid(const Jid &AStreamJid, const Jid &AContactJid) const
{
	if (FSidHash.contains(AStreamJid))
	{
		QHash<Jid, QString> sids=FSidHash[AStreamJid];
		if (sids.contains(AContactJid))
			return sids[AContactJid];
	}
	return QString();
}

bool JingleRtp::removeSid(const Jid &AStreamJid, const QString &ASid)
{
	qDebug() << "JingleRtp::removeSid(" << AStreamJid.full() << "," << ASid << ")";
	if (FSidHash.contains(AStreamJid))
	{
		Jid jid=FSidHash[AStreamJid].key(ASid, Jid());
		if (jid.isValid())
			if (FSidHash[AStreamJid].remove(jid))
			{
				if (FSidHash[AStreamJid].isEmpty())
					FSidHash.remove(AStreamJid);
				qDebug() << "FSidHash.size()=" << FSidHash.size();
				qDebug() << "return true";
				return true;
			}
	}
	qDebug() << "return false";
	return false;
}

bool JingleRtp::windowNotified(const IMessageChatWindow *window) const
{
	for (QMap<int, IMessageChatWindow *>::const_iterator it=FNotifies.constBegin(); it!=FNotifies.constEnd(); it++)
		if (*it==window)
			return true;
	return false;
}

bool JingleRtp::sessionInfo(const Jid &AStreamJid, const Jid &AContactJid, JingleRtp::InfoType AType, const QString &AName)
{
	QString sid=getSid(AStreamJid, AContactJid);
	if (!sid.isEmpty())
	{
		QDomDocument doc;
		QDomElement info=doc.createElementNS(NS_JINGLE_APPS_RTP_INFO, types[AType]);
		if (AType==Mute || AType==Unmute)
		{
			info.setAttribute("creator", FJingle->isOutgoing(AStreamJid, sid)?"initiator":"responder");
			if (!AName.isEmpty())
				info.setAttribute("name", AName);
		}
		return FJingle->sendAction(AStreamJid, sid, IJingle::SessionInfo, info);
	}
	return false;
}

void JingleRtp::putSid(const Jid &AStreamJid, const Jid &AContactJid, const QString &ASid)
{
	FSidHash[AStreamJid].insert(AContactJid, ASid);
}

void JingleRtp::callChatMessage(const Jid &AStreamJid, const QString &ASid, CallType AType, IJingle::Reason AReason)
{
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(AStreamJid, FJingle->contactJid(AStreamJid, ASid));
	if (window)
		writeCallMessageIntoChat(window, AType, AReason);
}

bool JingleRtp::writeCallMessageIntoChat(IMessageChatWindow *AWindow, CallType AType, IJingle::Reason AReason)
{
	Jid contactJid=AWindow->contactJid();
	Jid streamJid=AWindow->streamJid();
	QString sid=getSid(streamJid, contactJid);
	bool video=hasVideo(streamJid, sid);
	bool outgoing=AType==Called?FJingle->isOutgoing(streamJid, sid):false;

	IMessageStyleContentOptions options;
	options.time = QDateTime::currentDateTime();
	options.timeFormat = FMessageStyleManager->timeFormat(options.time);
	options.type = IMessageStyleContentOptions::TypeHistory;
	options.kind = IMessageStyleContentOptions::KindStatus;
	options.direction = outgoing?IMessageStyleContentOptions::DirectionOut:IMessageStyleContentOptions::DirectionIn;
	options.senderId  = streamJid.full();
	options.senderName   = HTML_ESCAPE(FMessageStyleManager->contactName(streamJid, contactJid));
	options.senderAvatar = FMessageStyleManager->contactAvatar(contactJid);

	QString message;
	QString image("<img src=\'%1\'/> %2");
	switch (AType)
	{
		case Called:
			message=video?image.arg(FIconStorage->fileFullName(outgoing
															   ?JNI_RTP_OUTGOING_VIDEO
															   :JNI_RTP_INCOMING_VIDEO)).arg(tr("Video call"))

						 :image.arg(FIconStorage->fileFullName(outgoing
															   ?JNI_RTP_OUTGOING
															   :JNI_RTP_INCOMING)).arg(tr("Voice call"));
			break;
		case Cancelled:
			message=image.arg(FIconStorage->fileFullName(JNI_RTP_HANGUP)).arg(tr("Call cancelled"));
			break;
		case Rejected:
			message=image.arg(FIconStorage->fileFullName(JNI_RTP_HANGUP)).arg(tr("Call rejected"));
			break;
		case Finished:
			message=image.arg(FIconStorage->fileFullName(JNI_RTP_HANGUP)).arg(tr("Call finished"));
			break;
		case Error:
			QString errorMessage = FJingle->errorMessage(AReason);
			message=image.arg(FIconStorage->fileFullName(JNI_RTP_ERROR)).arg(errorMessage.isEmpty()?tr("Call error")
																										   :tr("Call error (%1)").arg(errorMessage));
			options.status=IMessageStyleContentOptions::StatusError;
			break;
	}
	AWindow->viewWidget()->appendHtml(message, options);
	return updateWindowActions(AWindow);
}

bool JingleRtp::updateWindowActions(IMessageChatWindow *AWindow)
{
	if (!AWindow)
		return false;
	ToolBarChanger *toolBarChanger=AWindow->toolBarWidget()->toolBarChanger();
	QList<QAction *>actions = toolBarChanger->groupItems(TBG_MWTBW_JINGLE_RTP);
	Jid streamJid=AWindow->streamJid();
	Jid contactJid=AWindow->contactJid();
	QString sid=getSid(streamJid, contactJid);

	IJingle::SessionStatus status=sid.isEmpty()?IJingle::None
											   :FJingle->sessionStatus(streamJid, sid);

	if (status==IJingle::None || status==IJingle::Terminated)
	{
		for (QList<QAction *>::iterator it=actions.begin(); it!=actions.end(); it++)
		{
			Action *action=toolBarChanger->handleAction(*it);
			Command command=(Command)action->data(ADR_COMMAND).toInt();
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
		bool video=hasVideo(streamJid, sid);
		bool outgoing=FJingle->isOutgoing(streamJid, sid);
		for (QList<QAction *>::iterator it=actions.begin(); it!=actions.end(); it++)
		{
			Action *action=toolBarChanger->handleAction(*it);
			Command command=(Command)action->data(ADR_COMMAND).toInt();
			if (command==VoiceCall) // Voice call
			{
				if (video)
					action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL);
				else
					action->setIcon(RSR_STORAGE_JINGLE, status==IJingle::Initiated?(outgoing?JNI_RTP_OUTGOING:JNI_RTP_INCOMING):
														status==IJingle::Accepted ?JNI_RTP_CONNECT:JNI_RTP_TALK);
			}
			else if (command==VideoCall) // Video call
			{
				if (video)
					action->setIcon(RSR_STORAGE_JINGLE, status==IJingle::Initiated?(outgoing?JNI_RTP_OUTGOING_VIDEO:JNI_RTP_INCOMING_VIDEO):
														status==IJingle::Accepted ?JNI_RTP_CONNECT_VIDEO:JNI_RTP_TALK_VIDEO);
				else
					action->setIcon(RSR_STORAGE_JINGLE, JNI_RTP_CALL_VIDEO);
			}
			else if (command==Hangup)
				action->setEnabled(true);
		}
	}

	return false;
}

void JingleRtp::updateChatWindowActions(IMessageChatWindow *AChatWindow)
{
	qDebug() << "JingleRtp::updateChatWindowActions(" << AChatWindow << ")";
	QList<QAction *> actions = AChatWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_JINGLE_RTP);
	if (isSupported(AChatWindow->streamJid(), AChatWindow->contactJid()))
	{
		qDebug() << "Supported!";
		Jid contactJid = AChatWindow->contactJid();
		Jid streamJid  = AChatWindow->streamJid();
		qDebug() << "contactJid =" << contactJid.full();
		if (actions.isEmpty())
		{
			qDebug() << "Adding actions...";
			Action *action = new Action(AChatWindow->toolBarWidget()->instance());
			action->setText(tr("Voice call"));
			action->setIcon(FIconStorage->getIcon(JNI_RTP_CALL));
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
			action->setDisabled(true);
			connect(action,SIGNAL(triggered()),SLOT(onHangup()));
			AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_JINGLE_RTP);
			AChatWindow->toolBarWidget()->toolBarChanger()->setSeparatorsVisible(true);
		}
		else
		{
			qDebug() << "Changing action target addressed...";
			for (QList<QAction *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
			{
				Action *action = AChatWindow->toolBarWidget()->toolBarChanger()->handleAction(*it);
				if (action)
					action->setData(ADR_CONTACT_JID, contactJid.full());
				else
					LOG_ERROR("Invalid action handle!");
			}
		}
	}
	else
	{
		qDebug() << "Not supported! Removing actions...";
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

bool JingleRtp::hasPendingContents(const QString &AStreamJid, const QString &ASid, PendingType AType)
{
	QList<IJingleContent *> pendingContents = FPendingContents.values(AType);
	for (QList<IJingleContent *>::ConstIterator it = pendingContents.constBegin(); it!=pendingContents.constEnd(); it++)
		if ((*it)->streamJid() == AStreamJid && (*it)->sid() == ASid)
			return true;
	return false;
}

void JingleRtp::establishConnection(const Jid &AStreamJid, const QString &ASid)
{
	LOG_DEBUG(QString("JingleRtp::establishConnection(%1, %2)").arg(AStreamJid.full()).arg(ASid));
	QHash<QString, IJingleContent *> contents = FJingle->contents(AStreamJid, ASid);
	for (QHash<QString, IJingleContent *>::ConstIterator it=contents.constBegin(); it!=contents.constEnd(); it++)
	{
		LOG_DEBUG(QString("content=%1").arg((*it)->name()));
		if (FJingle->connectContent(AStreamJid, ASid, it.key()))
			addPendingContent(*it, Connect);
	}
	if (!hasPendingContents(AStreamJid.full(), ASid, Connect))
		FJingle->sessionTerminate(AStreamJid, ASid, IJingle::ConnectivityError);
}

void JingleRtp::connectionEstablished(const Jid &AStreamJid, const QString &ASid)
{
	qDebug() << "JingleRtp::connectionEstablished(" << AStreamJid.full() << "," << ASid << ")";
}

void JingleRtp::connectionTerminated(const Jid &AStreamJid, const QString &ASid)
{
	qDebug() << "JingleRtp::connectionTerminated(" << AStreamJid.full() << "," << ASid << ")";
}

MediaStreamer *JingleRtp::startSendMedia(const QPayloadType &APayloadType, QUdpSocket *AOutputSocket)
{
	qDebug() << "JingleRtp::startSendMedia(" << APayloadType << "," << AOutputSocket << ")";
	int codecId = QPayloadType::idByName(APayloadType.name);

	// Now, let's start sending content
	if (codecId)
	{
		QAVCodec encoder = QAVCodec::findEncoder(codecId);
		if (encoder)
		{
			QVariantHash options;
			options.insert("payload_type", APayloadType.id);
			MediaStreamer *sender = new MediaStreamer(selectedAudioDevice(QAudio::AudioInput), encoder, AOutputSocket->peerAddress(), AOutputSocket->peerPort(), 0, APayloadType.clockrate, Options::node(OPV_JINGLE_RTP_AUDIO_BITRATE).value().toInt(), options, this);
			if (sender->status() == MediaStreamer::Stopped)
			{
				if (connect(sender, SIGNAL(statusChanged(int)), SLOT(onSenderStatusChanged(int))))
				{
					qDebug() << "Starting streamer...";
					sender->start();
					qDebug() << "Streamer started!";
					return sender;
				}
				else
					LOG_ERROR("connect failed!");
			}
			else
			{
				LOG_ERROR(QString("MediaSender is not in Stopped state: %1").arg(sender->status()));
				delete sender;
			}
		}
		else
			LOG_ERROR(QString("Encoder not found! codec ID: %1").arg(codecId));
	}
	else
		LOG_ERROR(QString("codec name not found: %1").arg(APayloadType.name));
	return NULL;
}

MediaPlayer *JingleRtp::startPlayMedia(const QPayloadType &APayloadType, const QHostAddress &AHostAddress, quint16 APort)
{
	qDebug() << "JingleRtp::startPlayMedia(" << APayloadType << "," << AHostAddress << "," << APort << ")";
	MediaPlayer *streamer = new MediaPlayer(selectedAudioDevice(QAudio::AudioOutput), AHostAddress, APort, APayloadType.id, APayloadType.name, APayloadType.clockrate, APayloadType.channels, this);
	if (streamer->status() == MediaPlayer::Closed)
	{
		if (connect(streamer, SIGNAL(statusChanged(int,int)), SLOT(onStreamerStatusChanged(int,int))))
		{
			qDebug() << "Starting streamer...";
			if (streamer->setStatus(MediaPlayer::Running))
			{
				qDebug() << "Streamer started successfuly!";
				return streamer;
			}
			else				
				qDebug() << "Failed to start streamer!";
		}
		else
			qDebug() << "connect failed!";
	}
	else
		qDebug() << "MediaStreamer is not in Closeed state:" << streamer->status();
	delete streamer;
	return NULL;

}

QAudioDeviceInfo JingleRtp::selectedAudioDevice(QAudio::Mode AMode)
{
	qDebug() << "JingleRtp::selectedAudioDevice()";
	QList<QAudioDeviceInfo> devices(QAudioDeviceInfo::availableDevices(AMode));
	QString deviceName = Options::node(AMode==QAudio::AudioInput?OPV_JINGLE_RTP_AUDIO_INPUT:OPV_JINGLE_RTP_AUDIO_OUTPUT).value().toString();
	qDebug() << "deviceName=" << deviceName;

	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		if ((*it).deviceName()==deviceName)
		{
			qDebug() << "returning:" << (*it).deviceName();
			return *it;
		}

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

QPayloadType JingleRtp::buildPayloadType(const QDomElement &APayloadType, QPayloadType::MediaType AMedia)
{
	QPayloadType payloadType;

	payloadType.media = AMedia;

	if (APayloadType.hasAttribute("id"))
	{
		bool ok;
		int id = APayloadType.attribute("id").toInt(&ok);
		if (ok)
			payloadType.id=id;
	}

	if (APayloadType.hasAttribute("clockrate"))
	{
		bool ok;
		int clockrate = APayloadType.attribute("clockrate").toInt(&ok);
		if (ok)
			payloadType.clockrate=clockrate;
	}

	if (APayloadType.hasAttribute("channels"))
	{
		bool ok;
		int channels = APayloadType.attribute("channels").toInt(&ok);
		if (ok)
			payloadType.channels=channels;
	}

	if (APayloadType.hasAttribute("name"))
		payloadType.name=APayloadType.attribute("name");

	return payloadType;
}

//QStringList JingleRtp::stringsFromAvps(const QList<PayloadType> &AAvps)
//{
//	QStringList strings;
//	for (QList<PayloadType>::ConstIterator it=AAvps.constBegin(); it!=AAvps.constEnd(); ++it)
//		strings.append((*it));
//	return strings;
//}

//QList<PayloadType> JingleRtp::avpsFromStrings(const QStringList &AStrings)
//{
//	QList<PayloadType> avps;
//	for (QStringList::ConstIterator it=AStrings.constBegin(); it!=AStrings.constEnd(); ++it)
//		avps.append((*it));
//	return avps;
//}

IMessageChatWindow *JingleRtp::getWindow(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window = NULL;
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

			if (window)
				connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onTabPageActivated()));
		}
	return window;
}

void JingleRtp::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindowActions(AWindow);
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void JingleRtp::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
	if (window)
		updateChatWindowActions(window);
}

void JingleRtp::onSenderStatusChanged(int AStatus)
{
	qDebug() << "JingleRtp::onSenderStatusChanged(" << AStatus << ")";
	MediaStreamer *s = qobject_cast<MediaStreamer *>(sender());
	IJingleContent *content = FSenders.key(s);
	switch (AStatus)
	{
		case MediaStreamer::Started:
			qDebug() << "Started!";
			break;

		case MediaStreamer::Running:
			qDebug() << "Running!";
			break;

		case MediaStreamer::Error:
		{
			qDebug() << "Error!";
			if (s)
			{
				LOG_DEBUG("Terminating session...");
				FJingle->sessionTerminate(content->streamJid(), content->sid(), IJingle::FailedApplication);
				LOG_DEBUG("Removing sender...");
				FSenders.remove(content);
				delete s;
			}
			break;
		}
		case MediaStreamer::Stopped:
		{
			qDebug() << "Stopped!";
			if (s)
			{
				LOG_DEBUG("Removing sender...");
				FSenders.remove(content);
				delete s;
			}
			break;
		}

		default:
			break;
	}
}

void JingleRtp::onStreamerStatusChanged(int AStatusNew, int AStatusOld)
{
	qDebug() << "JingleRtp::onStreamerStatusChanged(" << AStatusNew << "," << AStatusOld << ")";

	MediaPlayer *streamer = qobject_cast<MediaPlayer*>(sender());
	IJingleContent *content = FStreamers.key(streamer);
	qDebug() << "content=" << content;
	switch (AStatusNew)
	{
		case MediaPlayer::Running:
		{
			LOG_DEBUG("Running!");
			Jid contactJid=FJingle->contactJid(content->streamJid(), content->sid());
			IMessageChatWindow *window=FMessageWidgets->findChatWindow(content->streamJid(), contactJid);
			if (window)
				updateWindowActions(window);
			connectionEstablished(content->streamJid(), content->sid());
			break;
		}

		case MediaPlayer::Finished:
		{
			LOG_DEBUG("Finished!");
			if (streamer)
			{
				LOG_DEBUG("Removing sender...");
				FStreamers.remove(content);
				delete streamer;
			}
			break;
		}

		case MediaPlayer::Error:
		{
			LOG_DEBUG("Error!");
			if (streamer)
			{
				LOG_DEBUG("Terminating session...");
				FJingle->sessionTerminate(content->streamJid(), content->sid(), IJingle::FailedApplication);
				LOG_DEBUG("Removing sender...");
				FStreamers.remove(content);
				delete streamer;
			}
			break;
		}
	}
}

void JingleRtp::onCall()
{
	qDebug() << "JingleRtp::onCall()";
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid  = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();
		Command command=(Command)action->data(ADR_COMMAND).toInt();

		if (FSidHash.contains(streamJid))
			if (FSidHash[streamJid].contains(contactJid))
			{
				QString sid=FSidHash[streamJid][contactJid];
				if (!FJingle->isOutgoing(streamJid, sid))
				{
					IJingle::Reason reason;
					QHash<QString, IJingleContent*> contents = FJingle->contents(streamJid, sid);
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
					if (!hasPendingContents(streamJid.full(), sid, FillTransport))
						FJingle->sessionTerminate(streamJid, sid, reason);
				}
				else
					qWarning() << "Session exists already! sid=" << sid;
				return; // Session exists already
			}

		QString sid=FJingle->sessionCreate(streamJid, contactJid, NS_JINGLE_APPS_RTP);
		if (!sid.isNull())
		{
			QSet<int> ids;
			IJingleContent *content = FJingle->contentAdd(streamJid, sid, "voice", "audio", NS_JINGLE_TRANSPORTS_RAW_UDP, false);
			if (content)
			{
				QAudioDeviceInfo inputDevice = selectedAudioDevice(QAudio::AudioInput);
				int bitrate = Options::node(OPV_JINGLE_RTP_AUDIO_BITRATE).value().toInt();
				QDomElement description(content->description());
				QList<int> codecIds = intsFromString(Options::node(OPV_JINGLE_RTP_CODECS_USED).value().toString());
				QSet<QPayloadType> payloadTypes;
				for (QList<int>::ConstIterator it=codecIds.constBegin(); it!=codecIds.constEnd(); ++it)
				{
					qDebug() << "codec id=" << *it;
					QAVCodec encoder = QAVCodec::findEncoder(*it);
					QList<int> sampleRates = encoder.supportedSampleRates();
					if (sampleRates.isEmpty())
						sampleRates=inputDevice.supportedSampleRates();
					for (QList<int>::ConstIterator itr = sampleRates.constBegin(); itr!=sampleRates.constEnd(); ++itr)
					{
						MediaStreamer *streamer = new MediaStreamer(inputDevice, encoder, QHostAddress("127.0.0.1"), 6666, 0, *itr, bitrate, QVariantHash(), this);
						if (streamer->status()==MediaStreamer::Stopped)
						{
							qDebug() << "Streamer status is stoppped!";
							QPayloadType payloadType(QPayloadType::fromSdp(streamer->getSdpString()));
							if (!payloadTypes.contains(payloadType))
//TODO: Make adequate validation
//							if (payloadType.isValid())
							{
								payloadTypes.insert(payloadType);
								QDomDocument document(content->document());
								QDomElement pt = document.createElement("payload-type");
								if (payloadType.id>95)
								{
									while(ids.contains(payloadType.id))
										++payloadType.id;
									ids.insert(payloadType.id);
								}

								pt.setAttribute("id", QString::number(payloadType.id));
								if (!payloadType.name.isEmpty())
									pt.setAttribute("name", payloadType.name);
								if (payloadType.clockrate)
									pt.setAttribute("clockrate", QString::number(payloadType.clockrate));
								if (payloadType.channels)
									pt.setAttribute("channels", QString::number(payloadType.channels));
								if (payloadType.ptime)
									pt.setAttribute("ptime", QString::number(payloadType.ptime));
								if (payloadType.maxptime)
									pt.setAttribute("maxptime", QString::number(payloadType.maxptime));
								description.appendChild(pt);
							}
						}
						else
							qWarning() << "Streamer status is:" << streamer->status();
						delete streamer;
						qDebug() << "Streamer deleted!";
					}
				}

				addPendingContent(content, AddContent);
				if (command==VideoCall)
				{   // Add video content
					content = FJingle->contentAdd(streamJid, sid, "video", "video", NS_JINGLE_TRANSPORTS_RAW_UDP, false);
					addPendingContent(content, AddContent);
				}
				putSid(streamJid, contactJid, sid);
				FJingle->sessionInitiate(streamJid, sid);
			}
		}
	}
	qDebug() << "JingleRtp::onCall():finished!";
}

void JingleRtp::onHangup()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid  = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();
		if (FSidHash.contains(streamJid))
			if (FSidHash[streamJid].contains(contactJid))
			{
				QString sid=FSidHash[streamJid][contactJid];
				IJingle::SessionStatus status = FJingle->sessionStatus(streamJid, sid);
				IJingle::Reason reason;
				switch (status)
				{
					case IJingle::ReceivingData:
						reason = IJingle::Success;
						break;

					case IJingle::Initiated:
						reason = FJingle->isOutgoing(streamJid, sid)?IJingle::Cancel:IJingle::Decline;
						break;

					default:
						reason = IJingle::Cancel;
						break;
				}

				FJingle->sessionTerminate(streamJid, sid, reason);
			}
	}
}

void JingleRtp::onConnectionEstablished(IJingleContent *AContent)
{
	LOG_DEBUG(QString("JingleRtp::onConnectionEstablished(%1)").arg(AContent->name()));
	removePendingContent(AContent, Connect);
	if (!hasPendingContents(AContent->streamJid(), AContent->sid(), Connect))
		FJingle->setConnected(AContent->streamJid(), AContent->sid());
}

void JingleRtp::onConnectionFailed(IJingleContent *AContent)
{
	removePendingContent(AContent, Connect);
	if (!hasPendingContents(AContent->streamJid(), AContent->sid(), Connect))
	{
		if (FJingle->contents(AContent->streamJid(), AContent->sid()).isEmpty())
			FJingle->sessionTerminate(AContent->streamJid(), AContent->sid(), IJingle::ConnectivityError);
		else
			FJingle->setConnected(AContent->streamJid(), AContent->sid());
	}
}

void JingleRtp::onContentAdded(IJingleContent *AContent)
{
	qDebug() << "JingleRtp::onContentAdded(" << AContent << ")";
	removePendingContent(AContent, AddContent);
	if (!hasPendingContents(AContent->streamJid(), AContent->sid(), AddContent))
		FJingle->sessionAccept(AContent->streamJid(), AContent->sid());
}

void JingleRtp::onContentAddFailed(IJingleContent *AContent)
{
	qDebug() << "JingleRtp::onContentAddFailed(" << AContent << ")";
	removePendingContent(AContent, AddContent);
	if (!hasPendingContents(AContent->streamJid(), AContent->sid(), AddContent))
	{
		if (FJingle->contents(AContent->streamJid(), AContent->sid()).isEmpty())
			FJingle->sessionTerminate(AContent->streamJid(), AContent->sid(), IJingle::FailedTransport);
		else
			FJingle->sessionAccept(AContent->streamJid(), AContent->sid());
	}
}

void JingleRtp::onIncomingTransportFilled(IJingleContent *AContent)
{
	qDebug() << "JingleRtp::onIncomingTransportFilled(" << AContent << ")";
	QStringList candidateIds = AContent->candidateIds();
	qDebug() << "candidateIds=" << candidateIds;
	for (QStringList::ConstIterator it = candidateIds.constBegin(); it != candidateIds.constEnd(); ++it)
	{
		QUdpSocket *socket = qobject_cast<QUdpSocket *>(AContent->inputDevice(*it));
		qDebug() << "socket=" << socket << ";" << socket->localAddress() << ":" << socket->localPort() << "/ state:" << socket->state();
	}

	removePendingContent(AContent, FillTransport);
	if (!hasPendingContents(AContent->streamJid(), AContent->sid(), FillTransport))
		FJingle->sessionAccept(AContent->streamJid(), AContent->sid());
}

void JingleRtp::onIncomingTransportFillFailed(IJingleContent *AContent)
{
	qDebug() << "JingleRtp::onIncomingTransportFillFailed(" << AContent << ")";
	removePendingContent(AContent, FillTransport);	
	if (!hasPendingContents(AContent->streamJid(), AContent->sid(), FillTransport))
	{
		if (FJingle->contents(AContent->streamJid(), AContent->sid()).isEmpty())
			FJingle->sessionTerminate(AContent->streamJid(), AContent->sid(), IJingle::FailedTransport);
		else
			FJingle->sessionAccept(AContent->streamJid(), AContent->sid());
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleRtp,JingleRtp)
#endif

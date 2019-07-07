#include <QDebug>
#include <QTimer>

#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/messagewindowwidgets.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/shortcuts.h>

#include <utils/options.h>

extern "C" {
#include <gcrypt.h>
}

#include "omemo.h"
#include "signalprotocol.h"

#define SHC_REQUEST "/iq[@type='set']/jingle[@xmlns='" NS_JINGLE "']"
#define SHC_RESULT  "/iq[@type='result']/pubsub[@xmlns='" NS_PUBSUB "']"
#define SHC_ERROR  "/iq[@type='error']"

#define TAG_NAME_LIST	"list"
#define TAG_NAME_DEVICE	"device"
#define TAG_NAME_BUNDLE	"bundle"
#define TAG_NAME_ITEMS	"items"

//#define DIR_OMEMO       "omemo"
#define DBFN_OMEMO      "omemo.db"

#define ADR_CONTACT_JID Action::DR_Parametr2
#define ADR_STREAM_JID Action::DR_StreamJid

static QByteArray encryptMessage(const QString &AMessageText, const QByteArray &AKey,
								 const QByteArray &AIv, QByteArray &AAuthTag)
{
	int rc = SG_SUCCESS;
	char * err_msg = nullptr;

	int algo = GCRY_CIPHER_AES128;
	int mode = GCRY_CIPHER_MODE_GCM;
	gcry_cipher_hd_t cipher_hd = {nullptr};
	QByteArray outBuf;
	QByteArray inBuf(AMessageText.toUtf8());

	if(AIv.size() != 16) {
		err_msg = "invalid AES IV size (must be not less than 8)";
		rc = SG_ERR_UNKNOWN;
		goto cleanup;
	}

	rc = int(gcry_cipher_open(&cipher_hd, algo, mode, 0));
	if (rc) {
		err_msg = "failed to init cipher";
		goto cleanup;
	}

	rc = int(gcry_cipher_setkey(cipher_hd, AKey.data(), size_t(AKey.size())));
	if (rc) {
		err_msg = "failed to set key";
		goto cleanup;
	}

	outBuf.resize(inBuf.size());
	rc = int(gcry_cipher_encrypt(cipher_hd, outBuf.data(), size_t(outBuf.size()),
											inBuf.data(), size_t(inBuf.size())));
	if (rc) {
		err_msg = "failed to encrypt";
		goto cleanup;
	}

	AAuthTag.resize(8);
	rc = int(gcry_cipher_gettag (cipher_hd, AAuthTag.data(), size_t(AAuthTag.size())));
	if (rc) {
		err_msg = "failed get authentication tag";
	} else {
		qDebug("Authentication tag: %s", AAuthTag.toHex().data());
	}

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(rc), gcry_strerror(rc));
			rc = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}
	}

	gcry_cipher_close(cipher_hd);

	return outBuf;
}

static QString decryptMessage(const QByteArray &AEncryptedText,
							  const QByteArray &AKey,
							  const QByteArray &AIv,
							  const QByteArray &AAuthTag)
{
	int ret_val = SG_SUCCESS;
	char * err_msg = nullptr;

	int algo = GCRY_CIPHER_AES128;
	int mode = GCRY_CIPHER_MODE_GCM;

	gcry_cipher_hd_t cipher_hd = {nullptr};
	QByteArray outBuf;

	if(AIv.size() != 16) {
	  err_msg = "invalid AES IV size (must be not less than 8)";
	  ret_val = SG_ERR_UNKNOWN;
	  goto cleanup;
	}

	ret_val = int(gcry_cipher_open(&cipher_hd, algo, mode, 0));
	if (ret_val) {
	  err_msg = "failed to init cipher";
	  goto cleanup;
	}

	ret_val = int(gcry_cipher_setkey(cipher_hd, AKey.data(), size_t(AKey.size())));
	if (ret_val) {
	  err_msg = "failed to set key";
	  goto cleanup;
	}

	outBuf.resize(AEncryptedText.size());
	ret_val = int(gcry_cipher_decrypt(cipher_hd, outBuf.data(), size_t(outBuf.size()),
									  AEncryptedText.data(), size_t(AEncryptedText.size())));
	if (ret_val) {
		err_msg = "failed to decrypt";
		goto cleanup;
	}

	ret_val = int(gcry_cipher_checktag (cipher_hd, AAuthTag.data(),
										size_t(AAuthTag.size())));
	if (ret_val) {
		err_msg = "failed check authentication tag";
	} else {
		qDebug("Authentication tag checked successfuly!");
	}

  cleanup:
	if (ret_val) {
		if (ret_val > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, err_msg, gcry_strsource(ret_val), gcry_strerror(ret_val));
			ret_val = SG_ERR_UNKNOWN;
		} else {
			qCritical("%s: %s\n", __func__, err_msg);
		}
	}

	gcry_cipher_close(cipher_hd);

	return QString::fromUtf8(outBuf);
}

static void getKeyPair(QByteArray &AKey, QByteArray &AIv)
{
	AKey.resize(16);
	gcry_randomize(AKey.data(), size_t(AKey.size()), GCRY_STRONG_RANDOM);
	AIv.resize(16);
	gcry_randomize(AIv.data(), size_t(AIv.size()), GCRY_STRONG_RANDOM);
}

Omemo::Omemo(): FPepManager(nullptr),
				FStanzaProcessor(nullptr),
				FXmppStreamManager(nullptr),
				FPresenceManager(nullptr),
				FOptionsManager(nullptr),
				FMessageProcessor(nullptr),
				FDiscovery(nullptr),
				FMessageWidgets(nullptr),
				FPluginManager(nullptr),
				FIconStorage(nullptr),
				FOmemoHandlerIn(0),
				FOmemoHandlerOut(0),
				FSHIResult(0),
				FSHIError(0),
				FSignalProtocol(nullptr)
{}

Omemo::~Omemo()
{}

void Omemo::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("OMEMO");
	APluginInfo->description = tr("Modern way of P2P encryption, using SignalProtocol");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
	APluginInfo->dependences.append(PEPMANAGER_UUID);
}

bool Omemo::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	FPluginManager = APluginManager;

	IPlugin *plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0, nullptr);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),
												   SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),
												   SLOT(onStreamClosed(IXmppStream *)));
		}
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0, nullptr);
	if (plugin) {
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
		if (FPresenceManager) {
			connect(FPresenceManager->instance(),SIGNAL(presenceOpened(IPresence *)),
												 SLOT(onPresenceOpened(IPresence *)));
			connect(FPresenceManager->instance(),SIGNAL(presenceClosed(IPresence *)),
												 SLOT(onPresenceClosed(IPresence *)));
		}
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
	if (plugin) {
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
		if (FOptionsManager)
			connect(FOptionsManager->instance(),SIGNAL(profileOpened(QString)),
												 SLOT(onProfileOpened(QString)));
	}

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0, nullptr);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IPEPManager").value(0);
	if (plugin)
		FPepManager = qobject_cast<IPEPManager *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	else return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, nullptr);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0, nullptr);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
			connect(FMessageWidgets->instance(),SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onNormalWindowCreated(IMessageNormalWindow *)));
		}
	}

	//AInitOrder = 100;   // This one should be initialized AFTER ....!
	return true;
}

bool Omemo::initObjects()
{
	SignalProtocol::init();
//	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	if (FDiscovery)
		registerDiscoFeatures();

	if (FMessageProcessor)
	{
//		FMessageProcessor->insertMessageWriter(MWO_OMEMO, this);
		FMessageProcessor->insertMessageEditor(MEO_OMEMO, this);
	}

	FPepManager->insertNodeHandler(QString(NS_PEP_OMEMO), this);

	if (FStanzaProcessor)
	{   // Register Stanza handlers
		IStanzaHandle requestHandle;
		requestHandle.handler = this;
		requestHandle.order = SHO_DEFAULT;
		requestHandle.direction = IStanzaHandle::DirectionIn;

		requestHandle.conditions.clear();
		requestHandle.conditions.append(SHC_RESULT);
		FSHIResult = FStanzaProcessor->insertStanzaHandle(requestHandle);

		requestHandle.conditions.clear();
		requestHandle.conditions.append(SHC_ERROR);
		FSHIError = FStanzaProcessor->insertStanzaHandle(requestHandle);
	}
	else
		return false;


	QByteArray key, iv;
	getKeyPair(key, iv);
	qDebug() << "key=" << key.toBase64() << "; iv=" << iv.toBase64();

	const QString message("Test message");
	qDebug() << "message=" << message;

	QByteArray authTag;
	QByteArray encrypted = encryptMessage(message, key, iv, authTag);
	qDebug() << "encrypted=" << encrypted.toBase64();

	QString decrypted = decryptMessage(encrypted, key, iv, authTag);
	qDebug() << "decrypted=" << decrypted;

	return true;
}

bool Omemo::initSettings()
{
	return true;
}

bool Omemo::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	qDebug() << "Omemo::processPEPEvent(" << AStreamJid.full() << "," << AStanza.toString() << ")";
	if (AStanza.type()!="error")
	{
		Jid contactJid = AStanza.from();
		QDomElement event  = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items  = event.firstChildElement("items");
		if(!items.isNull())
		{
//			bool stop=false;
			QDomElement item  = items.firstChildElement("item");
			if(!item.isNull())
			{
				QDomElement list = item.firstChildElement(TAG_NAME_LIST);
				if(!list.isNull())
				{
					QList<quint32> ids;
					for (QDomElement device = list.firstChildElement(TAG_NAME_DEVICE);
						   !device.isNull(); device = device.nextSiblingElement(TAG_NAME_DEVICE))
					{
						QString ida = device.attribute("id");
						qDebug() << "ida=" << ida;
						bool ok;
						quint32 id = ida.toUInt(&ok);
						if (ok)
							ids.append(id);
						else
							qCritical() << "Invalid id attribute value:" << ida;
					}
					if (!ids.isEmpty())
					{
						if (contactJid.bare() == AStreamJid.bare()) // Own ID list
						{
							IXmppStream *stream = FXmppStreamManager->findXmppStream(AStreamJid);
							if (FPepDelay.contains(stream))
							{
								QTimer *timer = FPepDelay.take(stream);
								timer->stop();
								timer->deleteLater();
							}

							quint32 ownId;
							if (FSignalProtocol->getDeviceId(ownId)==0) {
								qDebug() << "own device ID:" << ownId;
								if (!ids.contains(ownId))
								{
									ids.append(ownId);
									FDeviceIds.insert(contactJid.bare(), ids);
									publishOwnDeviceIds(AStreamJid);
								}
							} else {
								qCritical() << "SignalProtocol::getDeviceId() failed!";
							}
						}
						else
							FDeviceIds.insert(contactJid.bare(), ids);
					}
					else
						qCritical() << "No valid IDs found in OMEMO stanza!";
				}
			}

//			if(!stop && event.firstChild().firstChild().nodeName() == "retract")
//			{
//				if(contactJid.bare() == AStreamJid.bare())
//					FIdHash.remove(AStreamJid.bare());
//				stop=true;
//			}

//			if (stop)
//			{
//				if (contactJid.bare() == AStreamJid.bare())
//					FCurrentActivity.clear();
//				FActivityHash.remove(contactJid.bare());
//				updateChatWindows(contactJid, AStreamJid);
//				updateDataHolder(contactJid);
//				return true;
//			}
		}
	}
	return false;
}

void Omemo::registerDiscoFeatures()
{
	qDebug() << "Omemo::registerDiscoFeatures()";
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.var = NS_PEP_OMEMO_NOTIFY;
//	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK);
	dfeature.name = tr("OMEMO");
	dfeature.description = tr("P2P Encryption using OMEMO");
//	FDiscovery->insertDiscoFeature(dfeature);
//	qDebug() << "feature inserted:" << dfeature.var;

//	dfeature.var.append(NODE_NOTIFY_SUFFIX);
//	dfeature.name = tr("OMEMO Notification");
//	dfeature.description = tr("Receives notifications about devices, which support OMEMO");
	FDiscovery->insertDiscoFeature(dfeature);
	qDebug() << "feature inserted:" << dfeature.var;
}

bool Omemo::isSupported(const QString &ABareJid) const
{
	return FDeviceIds.contains(ABareJid);
}

bool Omemo::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return isSupported(AContactJid.bare()) &&
		   (!FDiscovery->hasDiscoInfo(AStreamJid,AContactJid) ||
			 FDiscovery->discoInfo(AStreamJid,AContactJid)
			.features.contains(NS_PEP_OMEMO_NOTIFY));
}

int Omemo::isSupported(const IMessageAddress *AAddresses) const
{
	if ((!AAddresses->contactJid().hasResource() &&
		 isSupported(AAddresses->contactJid().bare())) ||
		isSupported(AAddresses->streamJid(), AAddresses->contactJid()))
		return 2;
	QMultiMap<Jid,Jid> addresses = AAddresses->availAddresses(true);
	for (QMultiMap<Jid,Jid>::ConstIterator it = addresses.constBegin();
		 it != addresses.constEnd(); ++it) {
		if (isSupported(it->bare()))
			return 1;
	}
	return 0;
}

bool Omemo::setActiveSession(const Jid &AStreamJid, const QString &ABareJid, bool AActive)
{
	if (AActive)
	{
		if (!FActiveSessions[AStreamJid].contains(ABareJid))
		{
			FActiveSessions[AStreamJid].append(ABareJid);
			return true;
		}
	}
	else
	{
		if (FActiveSessions.contains(AStreamJid))
		{
			if (FActiveSessions[AStreamJid].contains(ABareJid))
			{
				FActiveSessions[AStreamJid].removeAll(ABareJid);
				if (FActiveSessions[AStreamJid].isEmpty())
					FActiveSessions.remove(AStreamJid);
				return true;
			}
		}
	}

	return false;
}

bool Omemo::isActiveSession(const Jid &AStreamJid, const QString &ABareJid) const
{
	return FActiveSessions.contains(AStreamJid) &&
		   FActiveSessions[AStreamJid].contains(ABareJid);
}

bool Omemo::isActiveSession(const IMessageAddress *AAddresses) const
{
	QMultiMap<Jid, Jid> addresses = AAddresses->availAddresses(true);
	for (QMultiMap<Jid, Jid>::ConstIterator it = addresses.constBegin();
		 it != addresses.constEnd(); ++it)
		if (isActiveSession(it.key(), it->bare()))
			return true;
	return false;
}

void Omemo::onNormalWindowCreated(IMessageNormalWindow *AWindow)
{
	if(AWindow->mode()==IMessageNormalWindow::WriteMode && isSupported(AWindow->streamJid(), AWindow->contactJid()))
	{
//		Action *action = new Action(new OmemoLinkList(FIconStorage, AWindow->instance()));
//		action->setText(tr("Add link"));
//		action->setIcon(RSR_STORAGE_MENUICONS, MNI_LINK_ADD);
//		action->setShortcutId(SCT_MESSAGEWINDOWS_OOB_INSERTLINK);
//		connect(action, SIGNAL(triggered(bool)), SLOT(onInsertLink(bool)));
//		AWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_OOB_VIEW);
	}
}

void Omemo::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(const Jid &, const Jid &)),
											SLOT(onAddressChanged(const Jid &, const Jid &)));
	updateChatWindowActions(AWindow);
}

void Omemo::updateChatWindowActions(IMessageChatWindow *AWindow)
{
	qDebug() << "Omemo::updateChatWindowActions()";
	QString contact = AWindow->contactJid().uFull();
	QString stream = AWindow->streamJid().uFull();

	QList<QAction*> omemoActions = AWindow->toolBarWidget()->toolBarChanger()
										->groupItems(TBG_MWTBW_OMEMO);
	Action *omemoAction = omemoActions.isEmpty()?nullptr:AWindow->toolBarWidget()
							->toolBarChanger()->handleAction(omemoActions.first());
	int supported = isSupported(AWindow->address());
	qDebug() << "supported=" << supported;
	if (supported)
	{
		if (!omemoAction)
		{
			omemoAction = new Action(AWindow->toolBarWidget()->instance());
			omemoAction->setToolTip(tr("OMEMO encryption"));

			QToolButton *omemoButton = AWindow->toolBarWidget()->
					toolBarChanger()->insertAction(omemoAction, TBG_MWTBW_OMEMO);
			omemoButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
			connect(omemoAction, SIGNAL(triggered()), SLOT(onOmemoActionTriggered()));
		}
		omemoAction->setData(ADR_STREAM_JID, stream);
		omemoAction->setData(ADR_CONTACT_JID, contact);
		omemoAction->setEnabled(supported==2);
		QString iconKey = isActiveSession(AWindow->address())?MNI_CRYPTO_ON
															 :MNI_CRYPTO_OFF;
		omemoAction->setIcon(RSR_STORAGE_MENUICONS, iconKey);
	}
	else
	{
		if (omemoAction)
		{
			AWindow->toolBarWidget()->toolBarChanger()->removeItem(omemoAction);
		}
	}
}

void Omemo::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)
	IMessageAddress *address = qobject_cast<IMessageAddress *>(sender());
	onUpdateMessageState(address->streamJid(), address->contactJid());
}

void Omemo::onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
	if (window)
	{
		updateChatWindowActions(window);
	}
}

void Omemo::onOmemoActionTriggered()
{
	qDebug() << "Omemo::onOmemoActionTriggered()";
	Action *action = qobject_cast<Action*>(sender());
	if (action)
	{
		Jid streamJid(action->data(ADR_STREAM_JID).toString());
		Jid contactJid(action->data(ADR_CONTACT_JID).toString());

		bool active = isActiveSession(streamJid, contactJid.bare());

		if (setActiveSession(streamJid, contactJid.bare(), !active))
		{
			IMessageChatWindow *window = FMessageWidgets
									->findChatWindow(streamJid, contactJid);
			if (window)
				updateChatWindowActions(window);
		}
	}
}

bool Omemo::publishOwnDeviceIds(const Jid &AStreamJid)
{
	qDebug() << "Omemo::publishOwnDeviceIds(" << AStreamJid.full() << ")";

	if (!FDeviceIds.contains(AStreamJid.bare()))
	{
		qWarning() << "No device ID list for the stream:" << AStreamJid.bare();
		return false;
	}
	QList<quint32> ids = FDeviceIds.value(AStreamJid.bare());
	if (ids.isEmpty())
	{
		qWarning() << "Own device ID list is empty!";
		return false;
	}

	QDomDocument doc;
	QDomElement item=doc.createElement("item");
	item.setAttribute("id", "current");

	QDomElement list=doc.createElementNS(NS_OMEMO, TAG_NAME_LIST);
	item.appendChild(list);
	for (QList<quint32>::ConstIterator it = ids.constBegin();
		 it != ids.constEnd(); ++it)
	{
		QDomElement device=doc.createElement(TAG_NAME_DEVICE);
		device.setAttribute("id", QString::number(*it));
		list.appendChild(device);
	}

	return FPepManager->publishItem(AStreamJid, NS_PEP_OMEMO, item);
}

bool Omemo::publishOwnKeys(const Jid &AStreamJid)
{
	quint32 deviceId;
	if (FSignalProtocol->getDeviceId(deviceId)!=SG_SUCCESS)
	{
		qCritical() << "Failed to get own device ID!";
		return false;
	}

	QDomDocument doc;
	QDomElement item=doc.createElement("item");
	item.setAttribute("id", "current");

	QDomElement bundle=doc.createElementNS(NS_OMEMO, TAG_NAME_BUNDLE);
	item.appendChild(bundle);

	QByteArray signedPublic = FSignalProtocol->getSignedPreKeyPublic();
	if (signedPublic.isNull())
	{
		qCritical() << "Failed to get Signed Pre Key";
		return false;
	}
	QDomElement signedPreKeyPublic=doc.createElement("signedPreKeyPublic");
	signedPreKeyPublic.setAttribute("signedPreKeyId", QString::number(SIGNED_PRE_KEY_ID));
	signedPreKeyPublic.appendChild(doc.createTextNode(signedPublic.toBase64()));
	bundle.appendChild(signedPreKeyPublic);

	QByteArray signature = FSignalProtocol->getSignedPreKeySignature();
	if (signature.isNull())
	{
		qCritical() << "Failed to get Signed Pre Key signature";
		return false;
	}
	QDomElement signedPreKeySignature=doc.createElement("signedPreKeySignature");
	signedPreKeySignature.appendChild(doc.createTextNode(signature.toBase64()));
	bundle.appendChild(signedPreKeySignature);

	QByteArray identityKeyPublic = FSignalProtocol->getIdentityKeyPublic();
	if (identityKeyPublic.isNull())
	{
		qCritical() << "Failed to get Signed Pre Key";
		return false;
	}
	QDomElement identityKey=doc.createElement("identityKey");
	identityKey.appendChild(doc.createTextNode(identityKeyPublic.toBase64()));
	bundle.appendChild(identityKey);

	QDomElement prekeys=doc.createElement("prekeys");
	for (quint32 i=PRE_KEYS_START; i<PRE_KEYS_START+PRE_KEYS_AMOUNT; ++i)
	{
		QByteArray key = FSignalProtocol->getPreKeyPublic(i);
		if (key.isNull())
		{
			qCritical() << "Pre Key id=" << i << "is NULL!";
			return false;
		}
		QDomElement preKeyPublic=doc.createElement("preKeyPublic");
		preKeyPublic.setAttribute("id", QString::number(i));
		preKeyPublic.appendChild(doc.createTextNode(key.toBase64()));
		prekeys.appendChild(preKeyPublic);
	}
	bundle.appendChild(prekeys);

	return FPepManager->publishItem(AStreamJid, QString("%1:%2").arg(NS_PEP_OMEMO_BUNDLES)
									.arg(deviceId), item);
}

bool Omemo::requestDeviceBundle(const Jid &AStreamJid, const QString &ABareJid, quint32 ADevceId)
{
//	<iq type='get'
//		from='romeo@montague.lit'
//		to='juliet@capulet.lit'
//		id='fetch1'>
//	  <pubsub xmlns='http://jabber.org/protocol/pubsub'>
//		<items node='eu.siacs.conversations.axolotl.bundles:31415'/>
//	  </pubsub>
//	</iq>
	Stanza iq(STANZA_KIND_IQ);
	iq.setUniqueId()
	  .setType(STANZA_TYPE_GET)
//	  .setFrom(AStreamJid.full())
	  .setTo(ABareJid);
	QDomElement pubsub = iq.addElement("pubsub", NS_PUBSUB);
	QDomElement items = iq.document().createElement(TAG_NAME_ITEMS);
	items.setAttribute("node", QString("%1:%2").arg(NS_PEP_OMEMO_BUNDLES)
											   .arg(ADevceId));
	pubsub.appendChild(items);
	return FStanzaProcessor->sendStanzaOut(AStreamJid, iq);
}

void Omemo::onProfileOpened(const QString &AProfile)
{
	FOmemoDir.setPath(FOptionsManager->profilePath(AProfile));
	FSignalProtocol = SignalProtocol::instance(FOmemoDir.filePath(DBFN_OMEMO));

	if (FSignalProtocol->install() != SG_SUCCESS) {
		qCritical() << "SignalProtocol::install() failed!";
		return;
	}
}

void Omemo::onStreamOpened(IXmppStream *AXmppStream)
{
	qDebug() << "Omemo::onStreamOpened(" << AXmppStream->streamJid().full() << ")";
	FStreamOmemo.insert(AXmppStream->streamJid(), nullptr);
}

void Omemo::onStreamClosed(IXmppStream *AXmppStream)
{
	qDebug() << "Omemo::onStreamClosed(" << AXmppStream->streamJid().full() << ")";
	FStreamOmemo.remove(AXmppStream->streamJid());
}

void Omemo::onPresenceOpened(IPresence *APresence)
{
	qDebug() << "Omemo::onPresenceOpened(" << APresence->streamJid().full() << ")";
	IXmppStream *stream = FXmppStreamManager->findXmppStream(APresence->streamJid());
	QTimer *timer = new QTimer(this);
	FPepDelay.insert(stream, timer);
	connect(timer, SIGNAL(timeout()), SLOT(onPepTimeout()));
	timer->start(10000);

	publishOwnKeys(APresence->streamJid());
}

void Omemo::onPresenceClosed(IPresence *APresence)
{
	qDebug() << "Omemo::onPresenceClosed(" << APresence->streamJid().full() << ")";
}

void Omemo::onPepTimeout()
{
	qDebug() << "Omemo::onPepTimeout()";
	QTimer *timer = qobject_cast<QTimer*>(sender());
	for (QHash<IXmppStream*, QTimer*>::Iterator it = FPepDelay.begin();
		 it != FPepDelay.end();)
		if (*it == timer) {
			IXmppStream *stream = it.key();
			it = FPepDelay.erase(it);
			qDebug() << "stream JID:" << stream->streamJid().full();
			QList<quint32> ids;
			quint32 id;
			if (FSignalProtocol->getDeviceId(id)==0) {
				qDebug() << "Publishing device ID:" << id;
				ids.append(id);
				FDeviceIds.insert(stream->streamJid().bare(), ids);
				publishOwnDeviceIds(stream->streamJid());				
			} else {
				qCritical() << "SignalProtocol::getDeviceId(id) failed!";
			}
		}
		else
			++it;
	timer->deleteLater();
}

//bool Omemo::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
//{
//	Q_UNUSED(ALang)
//	Q_UNUSED(AOrder)

//	return !AMessage.stanza().firstElement("x",NS_JABBER_OOB_X).isNull();
//}

//bool Omemo::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
//{
//	Q_UNUSED(ALang)
//	Q_UNUSED(AOrder)

//	bool noRuler=true;
//	QTextCursor cursor(ADocument);
//	QString         icon;
//	QTextCharFormat origFormat;
//	QTextCharFormat linkFormat;

//	QUrl bodyUrl(QUrl::fromUserInput(ADocument->toPlainText()));

//	for(QDomElement e=AMessage.stanza().firstElement("x",NS_JABBER_OOB_X); !e.isNull(); e=e.nextSiblingElement("x"))
//		if(!e.firstChildElement("url").text().isEmpty())
//		{
//			QUrl url = QUrl::fromEncoded(e.firstChildElement("url").text().toLatin1());
//			if (url.isValid() && url != bodyUrl)
//			{
//				if (noRuler)    // No ruler inserted so far
//				{
//					QTextBlockFormat origBlockFormat = cursor.blockFormat();    // Original block format
//					icon = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\" /> ")
//								   .arg(QUrl::fromLocalFile(FIconStorage->fileFullName(MNI_LINK)).toString())
//								   .arg(tr("Link"));

//					cursor.movePosition(QTextCursor::End);
//					origFormat = cursor.charFormat();
//					linkFormat = origFormat;
//					linkFormat.setAnchor(true);

//					cursor.insertHtml("<hr>");
//					cursor.insertBlock();
//					cursor.setBlockFormat(origBlockFormat);                     // Restore original format
//					noRuler=false;  // Flag ruler inserted
//				}
//				else
//				{
//					cursor.setCharFormat(origFormat);
//					cursor.insertHtml("<br>");
//				}

//				cursor.insertHtml(icon);
//				linkFormat.setAnchorHref(url.toEncoded());
//				linkFormat.setToolTip(url.toString());
//				cursor.setCharFormat(linkFormat);

//				QString desc = !e.firstChildElement("desc").text().isEmpty()?e.firstChildElement("desc").text():"";
//				cursor.insertText(desc.isEmpty()?url.toString():desc);
//			}
//		}
//	return !noRuler;
//}

//bool Omemo::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
//{
//	Q_UNUSED(AOrder) Q_UNUSED(AMessage) Q_UNUSED(ADocument) Q_UNUSED(ALang)
//	return false;
//}

bool Omemo::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

	qDebug() << "Omemo::messageReadWrite(" << AOrder << "," << AStreamJid.full()
			 << "," << AMessage.stanza().toString() << "," << ADirection << ")";

	if (ADirection==IMessageProcessor::DirectionOut)
	{
		qDebug() << "Outgoing message!";
		if (AMessage.stanza().firstElement("encrypted", NS_OMEMO).isNull())
		{
			qDebug() << "No <encrypted/> element found! Will process...";
			QString bareJid = AMessage.toJid().bare();
			if (isActiveSession(AStreamJid, bareJid))
			{
				qDebug() << "Have active session!";
				if (isSupported(AStreamJid, AMessage.toJid()))
				{
					QList<quint32> deviceIds = FDeviceIds[bareJid];
					qDebug() << "The contact supports OMEMO!";
					qDebug() << "Device IDs of the contact:" << deviceIds;
					bool needData = false;
					for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
						 it != deviceIds.constEnd(); ++it)
					{
						int sessionExists = FSignalProtocol->isSessionExistsAndInitiated(bareJid, *it);
						qDebug() << "device ID:" << *it << "; sessionExists=" << sessionExists;
						if (!sessionExists)
						{
							needData = true;
							requestDeviceBundle(AStreamJid, bareJid, *it);
						}
					}
					qDebug() << "needData =" << needData;
				}
			}
			else
				qDebug() << "No active session!";
		}
	}
	else
	{
		// Nothing to do right now
	}
	return false;
}

bool Omemo::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (AHandleId == FSHIResult)
	{
		qDebug() << "Got IQ result! id=" << AStanza.id();
		qDebug() << "AStanza:" << AStanza.toString();
		QDomElement items = AStanza.element()
								   .firstChildElement("pubsub")
								   .firstChildElement("items");
		if (items.isNull())
			return false;

		QString node = items.attribute("node");
		qDebug() << "node=" << node;

		if (!node.startsWith(NS_PEP_OMEMO_BUNDLES":"))
			return false;

		quint32 deviceId = node.split(":").at(1).toUInt();
		qDebug() << "deviceId=" << deviceId;
		QDomElement item = items.firstChildElement("item");
		QDomElement bundle = items.firstChildElement("bundle");

		QDomElement signedPreKeyPublic = items.firstChildElement("signedPreKeyPublic");
		qDebug() << "signedPreKeyPublic: id=" << signedPreKeyPublic.attribute("id")
				 << "; value=" << signedPreKeyPublic.text();

		QDomElement signedPreKeySignature = items.firstChildElement("signedPreKeySignature");
		qDebug() << "signedPreKeyPublic:" << signedPreKeySignature.text();

		QDomElement identityKey = items.firstChildElement("identityKey");
		qDebug() << "identityKey:" << identityKey.text();

		QDomElement prekeys = items.firstChildElement("prekeys");
		for (QDomElement preKeyPublic = items.firstChildElement("preKeyPublic");
			 !preKeyPublic.isNull(); preKeyPublic = item.nextSiblingElement("preKeyPublic"))
		{
			qDebug() << "id:" << preKeyPublic.attribute("id")
					 << "; value:" << preKeyPublic.text();
		}
	}
	else if (AHandleId == FSHIError)
	{
		qDebug() << "Got IQ error! id=" << AStanza.id();
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_omemo, Omemo)
#endif
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   

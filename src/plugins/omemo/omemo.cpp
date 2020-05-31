#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QpXhtml>
#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messagedataroles.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/resources.h>

#include <utils/options.h>
#include <utils/datetime.h>

#include "omemooptions.h"
#include "omemokeys.h"

extern "C" {
#include <gcrypt.h>
}

#include "omemo.h"

// PEP elements
#define TAG_NAME_PUBSUB					"pubsub"
#define TAG_NAME_DEVICES				"devices"
#define TAG_NAME_DEVICE					"device"
#define TAG_NAME_BUNDLE					"bundle"
#define TAG_NAME_ITEMS					"items"
#define TAG_NAME_ITEM					"item"
#define TAG_NAME_IK						"ik"
#define TAG_NAME_SPK					"spk"
#define TAG_NAME_SPKS					"spks"
#define TAG_NAME_PREKEYS				"prekeys"
#define TAG_NAME_PK						"pk"

// Message elements
#define TAG_NAME_BODY					"body"
#define TAG_NAME_ENCRYPTED				"encrypted"
#define TAG_NAME_HEADER					"header"
#define TAG_NAME_KEY					"key"
#define TAG_NAME_KEYS					"keys"
// #define TAG_NAME_IV					    "iv"
#define TAG_NAME_PAYLOAD				"payload"

#define ATTR_NAME_ID					"id"

#define ATTR_NAME_SID					"sid"
#define ATTR_NAME_RID					"rid"
#define ATTR_NAME_KEX				    "kex"

#define DIR_OMEMO						"omemo"

#define AES_256_KEY_LENGTH				32
#define AES_IV_LENGTH					16

#define ADR_CONTACT_JID Action::DR_Parametr2
#define ADR_STREAM_JID Action::DR_StreamJid

#define SHC_MESSAGE "/message/body"
#define SHC_MESSAGE_ENCRYPTED "/message/encrypted[@xmlns='" NS_OMEMO "']"

#define VDATA_SIZE(A) A.data(), size_t(A.size())

static QByteArray pkcs7pad(const QByteArray &AData)
{
	int pad = AData.size()%16;
	QByteArray padding;
	if (pad)
	{
		pad = 16-pad;
		padding.fill(char(pad), pad);
	}
	return AData+padding;
}

static QByteArray pkcs7unpad(const QByteArray &AData)
{
	if (!AData.isEmpty() && (AData.size()%16 == 0))
	{
		quint8 padsize = quint8(AData.at(AData.size() - 1));
		if (padsize < 16 && padsize < AData.size())
		{
			for (int i = 0; i < padsize; ++i)
				if (quint8(AData.at(AData.size()-i-1)) != padsize)
					return QByteArray(); // No padding
			return AData.left(AData.size()-padsize);
		}
	}
	return QByteArray(); // No padding
}

static QByteArray encryptMessageContent(SignalProtocol *ASignalProtocol, const QByteArray &AContent, QByteArray &AKeyHmac)
{
	gcry_error_t rc = SG_SUCCESS;
	char * errMsg = nullptr;

	gcry_cipher_hd_t cipherHd = nullptr;
	QByteArray inBuf, outBuf;
	QByteArray encryptionKey, authKey, iv, key, hmac, hmac1;
	key.resize(AES_256_KEY_LENGTH);
	gcry_randomize(VDATA_SIZE(key), GCRY_STRONG_RANDOM);
	QByteArray randomData = ASignalProtocol->hkdf_gen(80, key, QByteArray("OMEMO Payload"));
	encryptionKey = randomData.left(32);
	authKey = randomData.mid(32,32);
	iv = randomData.right(16);

	rc = gcry_cipher_open(&cipherHd, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
	if (rc) {
		errMsg = "Failed to init cipher";
		goto cleanup;
	}

	rc = gcry_cipher_setkey(cipherHd, VDATA_SIZE(encryptionKey));
	if (rc) {
		errMsg = "Failed to set key";
		goto cleanup;
	}

	rc = gcry_cipher_setiv(cipherHd, VDATA_SIZE(iv));
	if (rc) {
		errMsg = "Failed to set IV";
		goto cleanup;
	}

	inBuf = pkcs7pad(AContent);
	outBuf.resize(inBuf.size());
	rc = gcry_cipher_encrypt(cipherHd, VDATA_SIZE(outBuf), VDATA_SIZE(inBuf));
	if (rc) {
		errMsg = "Failed to encrypt";
		goto cleanup;
	}

	hmac = SignalProtocol::sha256hmac(authKey, outBuf);

	AKeyHmac = key+hmac;

cleanup:
	if (rc) {
		if (rc > 0)
			qCritical("%s: %s (%s: %s (%d))\n", __func__, errMsg,
					  gcry_strsource(rc), gcry_strerror(rc), rc);
		else
			qCritical("%s: %s\n", __func__, errMsg);
	}

	gcry_cipher_close(cipherHd);

	return outBuf;
}

static QByteArray decryptMessageContent(SignalProtocol *ASignalProtocol, const QByteArray &AEncryptedContent, const QByteArray &AKeyHmac)
{
	gcry_error_t rc = SG_SUCCESS;
	char * errMsg = nullptr;

	gcry_cipher_hd_t cipherHd = nullptr;
	QByteArray outBuf;
	QByteArray key, randomData, randomDataMy, encryptionKey, authKey, iv, hmac;

	if(AKeyHmac.size() != 64) {
		errMsg = "Invalid Key+HMAC size (must be 64)";
		rc = gcry_error_t(SG_ERR_UNKNOWN);
		goto cleanup;
	}

	key = AKeyHmac.left(32);
	randomData = ASignalProtocol->hkdf_gen(80, key, QByteArray("OMEMO Payload"));
	encryptionKey = randomData.left(32);
	authKey = randomData.mid(32,32);
	iv = randomData.right(16);

	hmac = SignalProtocol::sha256hmac(authKey, AEncryptedContent);
	if (hmac != AKeyHmac.right(32)) {
		errMsg = "HMAC verification failed!";
		goto cleanup;
	}

	rc = gcry_cipher_open(&cipherHd, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
	if (rc) {
	  errMsg = "failed to init cipher";
	  goto cleanup;
	}

	rc = gcry_cipher_setkey(cipherHd, VDATA_SIZE(encryptionKey));
	if (rc) {
	  errMsg = "failed to set key";
	  goto cleanup;
	}

	rc = gcry_cipher_setiv(cipherHd, VDATA_SIZE(iv));
	if (rc) {
		errMsg = "Failed to set IV";
		goto cleanup;
	}

	outBuf.resize(AEncryptedContent.size());
	rc = gcry_cipher_decrypt(cipherHd, VDATA_SIZE(outBuf), VDATA_SIZE(AEncryptedContent));
	if (rc) {
		outBuf = QByteArray();
		errMsg = "failed to decrypt";
		goto cleanup;
	}

	outBuf = pkcs7unpad(outBuf);

cleanup:
	if (rc) {
		if (rc > 0) {
			qCritical("%s: %s (%s: %s)\n", __func__, errMsg, gcry_strsource(rc),
															 gcry_strerror(rc));
		} else {
			qCritical("%s: %s\n", __func__, errMsg);
		}
	}

	gcry_cipher_close(cipherHd);

	return outBuf;
}

static QString getRandomString(int ALength)
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-!%^$#@*/_.,;?'`~");

   QString randomString;
   for(int i=0; i<ALength; ++i)
   {
	   int index = qrand() % possibleCharacters.length();
	   QChar nextChar = possibleCharacters.at(index);
	   randomString.append(nextChar);
   }
   return randomString;
}

static QByteArray getContent(const Stanza &AStanza)
{
	QDomDocument d;
	QDomElement content = d.createElementNS(NS_SCE, "content");
	d.appendChild(content);
	if (!AStanza.from().isEmpty())
	{
		QDomElement from = d.createElement("from");
		content.appendChild(from);
		from.setAttribute("jid", AStanza.from());
	}
	if (!AStanza.to().isEmpty())
	{
		QDomElement to = d.createElement("from");
		content.appendChild(to);
		to.setAttribute("jid", AStanza.to());
	}
	QDomElement rpad = d.createElement("rpad");
	content.appendChild(rpad);
	rpad.appendChild(d.createTextNode(getRandomString(qrand()*200/RAND_MAX)));
	QDomElement time = d.createElement("time");
	content.appendChild(time);
	time.setAttribute("stamp", DateTime(QDateTime::currentDateTime()).toX85UTC());
	QDomElement payload = d.createElement("payload");
	content.appendChild(payload);
	QDomNode body = d.importNode(AStanza.element().firstChildElement("body"), true);
	payload.appendChild(body);
	return d.toByteArray();
}

Omemo::Omemo(): FAccountManager(nullptr),
				FOptionsManager(nullptr),
				FPepManager(nullptr),
				FStanzaProcessor(nullptr),
				FMessageProcessor(nullptr),
				FXmppStreamManager(nullptr),
				FPresenceManager(nullptr),
				FDiscovery(nullptr),
				FMessageWidgets(nullptr),
				FMainWindowPlugin(nullptr),
				FPluginManager(nullptr),
//				FIconStorage(nullptr),
				FOmemoHandlerIn(0),
				FOmemoHandlerOut(0),
				FSHIMessageIn(0),
				FSHIMessageOut(0),
				FCleanup(true) // Temporary flag for own device information cleanup
{}

Omemo::~Omemo()
{}

SignalProtocol *Omemo::signalProtocol(const Jid &AStreamJid)
{
	return FSignalProtocols.value(AStreamJid);
}

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
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());

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

	plugin = APluginManager->pluginInterface("IAccountManager").value(0, nullptr);
	if (plugin) {
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());
		if (FAccountManager) {
			connect(FAccountManager->instance(),SIGNAL(accountInserted(IAccount *)),
												SLOT(onAccountInserted(IAccount *)));
			connect(FAccountManager->instance(),SIGNAL(accountRemoved(IAccount *)),
												SLOT(onAccountRemoved(IAccount *)));
			connect(FAccountManager->instance(),SIGNAL(accountDestroyed(QUuid)),
												SLOT(onAccountDestroyed(QUuid)));
		}
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0, nullptr);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0, nullptr);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IPEPManager").value(0);
	if (plugin)
		FPepManager = qobject_cast<IPEPManager *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, nullptr);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0, nullptr);
	if (plugin)
		FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
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

	connect(Options::instance(), SIGNAL(optionsOpened()),
								 SLOT(onOptionsOpened()));
	connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)),
								 SLOT(onOptionsChanged(OptionsNode)));
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
		FMessageProcessor->insertMessageEditor(MEO_OMEMO, this);
		FMessageProcessor->insertMessageWriter(MWO_OOB, this);
	}
	else
		return false;

	if (FStanzaProcessor)
	{   // Register Stanza handlers
		IStanzaHandle requestHandle;
		requestHandle.handler = this;
		requestHandle.conditions.append(SHC_MESSAGE);
		requestHandle.order = SHO_OMEMO_OUT;
		requestHandle.direction = IStanzaHandle::DirectionOut;
		FSHIMessageOut = FStanzaProcessor->insertStanzaHandle(requestHandle);
		requestHandle.conditions.clear();
		requestHandle.conditions.append(SHC_MESSAGE_ENCRYPTED);
		requestHandle.order = SHO_OMEMO_IN;
		requestHandle.direction = IStanzaHandle::DirectionIn;
		FSHIMessageIn = FStanzaProcessor->insertStanzaHandle(requestHandle);
	}
	else
		return false;

	FPepManager->insertNodeHandler(QString(NS_PEP_OMEMO), this);

	if (FOptionsManager)
	{
		IOptionsDialogNode p2pNode = { ONO_P2P, OPN_P2P, MNI_CRYPTO_ON, tr("P2P Encryption") };
		FOptionsManager->insertOptionsDialogNode(p2pNode);

		IOptionsDialogNode omemoNode = { ONO_P2P_OMEMO, OPN_P2P_OMEMO, MNI_CRYPTO_ACK, tr("OMEMO Keys") };
		FOptionsManager->insertOptionsDialogNode(omemoNode);

		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

bool Omemo::initSettings()
{
	Options::setDefaultValue(OPV_OMEMO_SIMULATEERROR, false);
	Options::setDefaultValue(OPV_OMEMO_RETRACT_ACCOUNT, false);
	Options::setDefaultValue(OPV_OMEMO_FALLBACKMESSAGE,
							 tr("I sent you an OMEMO encrypted message but "
								"your client doesn’t seem to support that. "
								"Find more information on "
								"https://xmpp.org/extensions/xep-0384.html"));
	Options::setDefaultValue(OPV_OMEMO_OPTOUTMESSAGE,
							 tr("Sorry, but I need to terminate private conversation."));
	Options::setDefaultValue(OPV_OMEMO_OPTOUTCONFIRM, false);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> Omemo::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	Q_UNUSED(AParent)
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_P2P)
	{
		widgets.insertMulti(OHO_OMEMO, FOptionsManager->newOptionsDialogHeader(
								tr("OMEMO"), AParent));
		widgets.insertMulti(OWO_OMEMO, new OmemoOptions(this, AParent));
	}
	else if (ANodeId == OPN_P2P_OMEMO)
	{
		widgets.insertMulti(OWO_OMEMO, new OmemoKeys(this, AParent));
	}
	return widgets;
}

bool Omemo::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.type()!="error")
	{
		QString bareJid = AStanza.fromJid().bare();
		QDomElement event  = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items  = event.firstChildElement(TAG_NAME_ITEMS);
		SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
		if(!items.isNull())
		{
			QDomElement item  = items.firstChildElement(TAG_NAME_ITEM);
			if(!item.isNull())
			{
				QDomElement devices = item.firstChildElement(TAG_NAME_DEVICES);
				if(!devices.isNull())
				{
					QList<quint32> ids;
					for (QDomElement device = devices.firstChildElement(TAG_NAME_DEVICE);
						 !device.isNull(); device = device.nextSiblingElement(TAG_NAME_DEVICE))
					{
						if (device.hasAttribute("label"))
						{
							QString label = device.attribute("label");
//TODO: Process "label" attribute
						}
						QString ida = device.attribute(ATTR_NAME_ID);
						bool ok;
						quint32 id = ida.toUInt(&ok);
						if (ok)
							ids.append(id);
						else
							qCritical() << "Invalid id attribute value:" << ida;
					}

					if (FDeviceIds.contains(bareJid)) // Cleanup orpaned devices
					{
						QList<quint32> &currentIds = FDeviceIds[bareJid];
						for (QList<quint32>::ConstIterator it = currentIds.constBegin();
							 it != currentIds.constEnd(); ++it)
						{
							if (!ids.contains(*it) &&							 // Device was removed and
								(signalProtocol->sessionInitStatus(bareJid, *it) > SignalProtocol::NoSession)) // session with the device exists
								signalProtocol->deleteSession(bareJid, *it);
						}
					}

					if (!ids.isEmpty())
					{
						QList<quint32> newIds;
						bool cleanup = Options::node(OPV_OMEMO_RETRACT)
								.value("account",
									   FAccountManager->findAccountByStream(AStreamJid)->accountId().toString())
								.toBool();
						bool ownJid = bareJid == AStreamJid.bare();
						if (FDeviceIds.contains(bareJid) && !(ownJid && cleanup))
						{
							QList<quint32> &currentIds = FDeviceIds[bareJid];
							for (QList<quint32>::ConstIterator it = ids.constBegin();
								 it != ids.constEnd(); ++it)
								if (!currentIds.contains(*it))
									newIds.append(*it);
						}

						if (ownJid) // Own ID list
						{
							IXmppStream *stream = FXmppStreamManager->findXmppStream(AStreamJid);
							if (FPepDelay.contains(stream))
							{
								QTimer *timer = FPepDelay.take(stream);
								timer->stop();
								timer->deleteLater();
							}

							qDebug() << "cleanup!";

							quint32 ownId = FSignalProtocols[AStreamJid]->getDeviceId();
							if (ownId)
							{
								if (!ids.contains(ownId))
								{
									ids.append(ownId);
									FDeviceIds.insert(bareJid, ids);
									if (!cleanup)
										publishOwnDeviceIds(AStreamJid);
								}
								else
									FDeviceIds.insert(bareJid, ids);								

								if (cleanup)
									removeOtherDevices(AStreamJid);
							}
						}
						else
							FDeviceIds.insert(bareJid, ids);

						if (!newIds.isEmpty() &&					// Have new IDs for the contact and
							isActiveSession(AStreamJid, bareJid))	// have active session with the contact
						{
							for (QList<quint32>::Iterator it = newIds.begin();
								 it != newIds.end();)
								if (signalProtocol->getIdentityTrusted(bareJid, *it) > -1) // Record exists
									it = newIds.erase(it); // Erase device ID for existing identity
								else
									++it;

							if (!newIds.isEmpty())
								requestBundles4Devices(AStreamJid, bareJid, newIds);
						}


					}
					else
						qCritical() << "No valid IDs found in OMEMO stanza!";					
				}
			}
		}
		return true;
	}
	return false;
}

bool Omemo::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FSignalProtocols.contains(AStreamJid))
	{
		SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
		if (AHandleId == FSHIMessageOut)
		{
			if (AStanza.firstElement(TAG_NAME_ENCRYPTED, NS_OMEMO).isNull())
			{
				QString bareJid = AStanza.toJid().bare();
				if (isActiveSession(AStreamJid, bareJid))
				{
					bool haveTrustedIdentities(false);
					if (isSupported(AStreamJid, AStanza.toJid()))
					{
						QList<quint32> deviceIds = FDeviceIds[bareJid];
						QHash<quint32, SignalDeviceBundle> bundles = FBundles.value(bareJid);
						bool needBundles = false;
						QList<quint32> bundlesToRequest;
						for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
							 it != deviceIds.constEnd(); ++it)
						{
							int trusted = signalProtocol->getIdentityTrusted(bareJid, *it);
							if (trusted == 1) // Trusted identity
								haveTrustedIdentities = true;
							else if (trusted == -1) // New identity
							{
								if (!bundles.contains(*it))
								{
									needBundles = true;
									if (!FPendingRequests.contains(bareJid, *it))
										bundlesToRequest.append(*it);
								}
							}
						}

						QString ownJid = AStreamJid.bare();
						deviceIds = FDeviceIds[ownJid]; // Own device IDs
						quint32 deviceId = signalProtocol->getDeviceId();
						if (deviceId) // Add own device IDs except this one
							deviceIds.removeOne(deviceId);

						QList<quint32> ownBundlesToRequest;
						for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
							 it != deviceIds.constEnd(); ++it)
						{
							if (signalProtocol->getIdentityTrusted(bareJid, *it) == -1) // New identity
							{
								if (!bundles.contains(*it))
								{
									needBundles = true;
									if (!FPendingRequests.contains(ownJid, *it))
										ownBundlesToRequest.append(*it);
								}
							}
						}
//TODO: Store list of failed devices to skip rerequesting bundles them every time
						if (needBundles)
						{
							if (!bundlesToRequest.isEmpty())
							{
								QString id = requestBundles4Devices(AStreamJid, bareJid, bundlesToRequest);
								if (!id.isNull())
								{
									for (QList<quint32>::ConstIterator it = bundlesToRequest.constBegin();
										 it != bundlesToRequest.constEnd(); ++it)
									{
										FBundleRequests.insertMulti(id, *it);
										FPendingRequests.insertMulti(bareJid, *it);
									}
								}
							}
							if (!ownBundlesToRequest.isEmpty())
							{
								QString id = requestBundles4Devices(AStreamJid, ownJid, ownBundlesToRequest);
								if (!id.isNull())
								{
									for (QList<quint32>::ConstIterator it = ownBundlesToRequest.constBegin();
										 it != ownBundlesToRequest.constEnd(); ++it)
									{
										FBundleRequests.insertMulti(id, *it);
										FPendingRequests.insertMulti(ownJid, *it);
									}
								}
							}
							if (haveTrustedIdentities)
								FPendingMessages.insert(bareJid, AStanza);
							else
							{
								QMessageBox::critical(FMainWindowPlugin->mainWindow()->instance(),
													 bareJid,
													 tr("No trusted identities for the contact!\n"
														"The message wasn't sent"),
													 QMessageBox::Ok);
							}
							return true;
						}
						else
							encryptMessage(AStanza); // Just encrypt message, allowing to send it
					}
				}
			}
		}
		else
		{
			QDomElement encrypted = AStanza.firstElement(TAG_NAME_ENCRYPTED, NS_OMEMO);
			if (!encrypted.isNull())
			{
				QDomElement payload = encrypted.firstChildElement(TAG_NAME_PAYLOAD);
				if (!payload.isNull())
				{
					QDomElement header = encrypted.firstChildElement(TAG_NAME_HEADER);
					if (!header.isNull())
					{
						if (header.hasAttribute(ATTR_NAME_SID))
						{
							bool ok;
							quint32 sid = header.attribute(ATTR_NAME_SID).toUInt(&ok);
							if (ok && sid)
							{
								quint32 deviceId = signalProtocol->getDeviceId();

								for (QDomElement keys = header.firstChildElement(TAG_NAME_KEYS);
									 !keys.isNull(); keys = keys.nextSiblingElement(TAG_NAME_KEYS))
									if (keys.attribute("jid")==AStreamJid.bare())
									{
										for (QDomElement key = keys.firstChildElement(TAG_NAME_KEY);
											 !key.isNull(); key = key.nextSiblingElement(TAG_NAME_KEY))
											if (key.hasAttribute(ATTR_NAME_RID))
											{
												quint32 rid = key.attribute(ATTR_NAME_RID).toUInt(&ok);
												if (ok && rid)
												{
													if (rid==deviceId)
													{
														QByteArray decoded = QByteArray::fromBase64(key.text().toLatin1());
														SignalProtocol::Cipher cipher = signalProtocol->sessionCipherCreate(AStanza.fromJid().bare(), sid);
														QByteArray keyHmac;
														if (key.attribute(ATTR_NAME_KEX)=="true")
														{
															SignalProtocol::PreKeySignalMessage message = signalProtocol->getPreKeySignalMessage(decoded);
															bool preKeyUpdated;
															keyHmac = cipher.decrypt(message, preKeyUpdated);
															if (preKeyUpdated)
																publishOwnKeys(AStreamJid);
														}
														else
														{
															SignalProtocol::SignalMessage message = signalProtocol->getSignalMessage(decoded);
															keyHmac = cipher.decrypt(message);
														}
														QByteArray encryptedContent = QByteArray::fromBase64(payload.text().toLatin1());
														QByteArray decryptedContent = decryptMessageContent(signalProtocol, encryptedContent, keyHmac);
// Don't delete <encrypted/> element: it will be used later in messageReadWrite()
//														AStanza.element().removeChild(encrypted);
														if (decryptedContent.isEmpty() || Options::node(OPV_OMEMO_SIMULATEERROR).value().toBool())
															decryptedContent = QString(	"<content xmlns='" NS_SCE "'>" \
																						" <payload>" \
																						"  <failed xmlns='" NS_EYECU "'/>" \
																						"  <body>%1</body>" \
																						" </payload>" \
																						"</content>")
																				.arg(tr("Failed to decrypt message"))
																				.toUtf8();
														else
															if (!isActiveSession(AStreamJid, AStanza.fromJid().bare()))
																setActiveSession(AStreamJid, AStanza.fromJid().bare());
														QDomDocument content;
														if (content.setContent(decryptedContent, true))
														{
//TODO: Process all the fiels in <content/> element
															QDomElement body = AStanza.firstElement(TAG_NAME_BODY);
															if (!body.isNull())
																AStanza.element().removeChild(body);
															QDomElement root = content.documentElement();
															if (root.tagName()=="content" && root.namespaceURI()==NS_SCE)
															{
																QDomElement payload = root.firstChildElement("payload");
																if (!payload.isNull())
																{
																	for (QDomElement e = payload.firstChildElement(); !e.isNull();
																		 e = e.nextSiblingElement())
																	{
																		QDomNode node = AStanza.document().importNode(e, true);
																		AStanza.element().appendChild(node);
																	}
																}
																else
																	qCritical("No payload element found in content!");
															}
															else
																qCritical("Invalid content!");
														}
														else
															qCritical("Invalid content!");
														break;
													}
												}
												else
													qCritical() << "Invalid rid attribute:" << header.attribute(ATTR_NAME_RID);
											}
											else
												qCritical() << "rid attribute is missing!";
									}
							}
							else
								qCritical() << "Invalid sid attribute:" << header.attribute(ATTR_NAME_SID);
						}
						else
							qCritical() << "sid attribute is missing!";
					}
					else
						qCritical() << "<header/> element is missing!";
				}
				else
					qCritical() << "<payload/> element is missing!";
			}
		}
	}
	return false;
}

void Omemo::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.var = NS_PEP_OMEMO_NOTIFY;
//	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK);
	dfeature.name = tr("OMEMO");
	dfeature.description = tr("P2P Encryption using OMEMO");

	FDiscovery->insertDiscoFeature(dfeature);
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

			QList<quint32> deviceIds = FDeviceIds[ABareJid];
			SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
			bool needBundles = false;
			QList<quint32> bundlesToRequest;
			for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
				 it != deviceIds.constEnd(); ++it)
			{
				int sessionState = signalProtocol->sessionInitStatus(ABareJid, *it);
				if (sessionState == SignalProtocol::NoSession)
				{
					needBundles = true;
					if (!FPendingRequests.contains(ABareJid, *it))
						bundlesToRequest.append(*it);
				}
			}

//TODO: Store list of failed devices to skip rerequesting bundles them every time
			if (needBundles)
			{
				if (!bundlesToRequest.isEmpty())
				{
					QString id = requestBundles4Devices(AStreamJid, ABareJid, bundlesToRequest);
					if (!id.isNull())
						for (QList<quint32>::ConstIterator it = bundlesToRequest.constBegin();
							 it != bundlesToRequest.constEnd(); ++it)
						{
							FBundleRequests.insertMulti(id, *it);
							FPendingRequests.insertMulti(ABareJid, *it);
						}
				}
				return true;
			}
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
	QString contact = AWindow->contactJid().uFull();
	QString stream = AWindow->streamJid().uFull();
	QList<QAction*> omemoActions = AWindow->toolBarWidget()->toolBarChanger()
										->groupItems(TBG_MWTBW_OMEMO);
	Action *omemoAction = omemoActions.isEmpty()?nullptr:AWindow->toolBarWidget()
							->toolBarChanger()->handleAction(omemoActions.first());
	int supported = isSupported(AWindow->address());
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
		updateOmemoAction(omemoAction);
	}
	else
	{
		if (omemoAction)
		{
			AWindow->toolBarWidget()->toolBarChanger()->removeItem(omemoAction);
		}
	}
}

void Omemo::updateOmemoAction(Action *AAction)
{
	QString iconKey = MNI_CRYPTO_OFF;
	Jid streamJid = AAction->data(ADR_STREAM_JID).toString();
	Jid contactJid = AAction->data(ADR_CONTACT_JID).toString();
	if (isActiveSession(streamJid, contactJid.bare()))
	{
		iconKey = MNI_CRYPTO_ON;
		QString bareJid = contactJid.bare();
		if (FDeviceIds.contains(bareJid))
		{
			const QList<quint32> &devices = FDeviceIds[bareJid];
			SignalProtocol *signalProtocol = FSignalProtocols[streamJid];
			for (QList<quint32>::ConstIterator itc=devices.constBegin();
				 itc != devices.constEnd(); ++itc)
				if (signalProtocol->sessionInitStatus(bareJid, *itc)
						==SignalProtocol::SessionAcknowledged)
				{
					iconKey = MNI_CRYPTO_ACK;
					break;
				}
		}
	}
	AAction->setIcon(RSR_STORAGE_MENUICONS, iconKey);
}

void Omemo::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)
	IMessageAddress *address = qobject_cast<IMessageAddress *>(sender());
	onUpdateMessageState(address->streamJid(), address->contactJid());
}

void Omemo::onAccountInserted(IAccount *AAccount)
{
	QString name=AAccount->accountId().toString();
	QDir omemoDir;
	omemoDir.setPath(Options::instance()->filesPath());
	if (!omemoDir.cd(DIR_OMEMO))
	{
		omemoDir.mkdir(DIR_OMEMO);
		if (!omemoDir.cd(DIR_OMEMO))
		{
			qCritical("Cannot switch to %s", DIR_OMEMO);
			return;
		}
	}

	SignalProtocol *signalProtocol = new SignalProtocol(omemoDir.filePath(name+".db"), name, this, 4);
	if (signalProtocol->install() == SG_SUCCESS)
		FSignalProtocols.insert(AAccount->streamJid(), signalProtocol);
	else
	{
		qCritical() << "SignalProtocol::install() for account"
					<< AAccount->name()
					<< AAccount->accountId()
					<< "failed!";
		delete signalProtocol;
	}
}

void Omemo::onAccountRemoved(IAccount *AAccount)
{
	if (FSignalProtocols.contains(AAccount->streamJid()))
		delete FSignalProtocols.take(AAccount->streamJid());
}

void Omemo::onAccountDestroyed(const QUuid &AAccountId)
{
	QString fileName = AAccountId.toString()+".db";
	QDir omemoDir(Options::instance()->filesPath());
	omemoDir.cd(DIR_OMEMO);
	if (!omemoDir.remove(fileName))
		qCritical() << "Failed to delete databse file:" << fileName;
}

void Omemo::onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
	if (window)
		updateChatWindowActions(window);
}

void Omemo::onOmemoActionTriggered()
{
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
	if (!FDeviceIds.contains(AStreamJid.bare()))
		return false;
	QList<quint32> ids = FDeviceIds.value(AStreamJid.bare());
	if (ids.isEmpty())
	{
		qWarning("Own device ID list is empty!");
		return false;
	}

	QDomDocument doc;
	QDomElement item=doc.createElement(TAG_NAME_ITEM);
	item.setAttribute(ATTR_NAME_ID, "current");

	QDomElement list=doc.createElementNS(NS_OMEMO, TAG_NAME_DEVICES);
	item.appendChild(list);
	for (QList<quint32>::ConstIterator it = ids.constBegin();
		 it != ids.constEnd(); ++it)
	{
		QDomElement device=doc.createElement(TAG_NAME_DEVICE);
		device.setAttribute(ATTR_NAME_ID, QString::number(*it));
//TODO: Set "label" attribute
		list.appendChild(device);
	}

	IDataForm options;
	IDataField field;
	field.var="pubsub#access_model";
	field.value="open";
	options.fields.append(field);
	field.var="FORM_TYPE";
	field.type="hidden";
	field.value=NS_PUBSUB"#publish-options";
	options.fields.append(field);

	return FPepManager->publishItem(AStreamJid, NS_PEP_OMEMO, item, &options);
}

bool Omemo::publishOwnKeys(const Jid &AStreamJid)
{
	if (!FSignalProtocols.contains(AStreamJid))
		return false;
	SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
	quint32 deviceId = signalProtocol->getDeviceId();
	if (!deviceId)
		return false;
	QDomDocument doc;
	QDomElement item=doc.createElement(TAG_NAME_ITEM);
	item.setAttribute(ATTR_NAME_ID, QString::number(deviceId));

	QDomElement bundle=doc.createElementNS(NS_OMEMO, TAG_NAME_BUNDLE);
	item.appendChild(bundle);

	QByteArray signedPublic = signalProtocol->getSignedPreKeyPublic();
	if (signedPublic.isNull())
	{
		qCritical("Failed to get Signed Pre Key");
		return false;
	}
	QDomElement spk=doc.createElement(TAG_NAME_SPK);
	spk.setAttribute(ATTR_NAME_ID, QString::number(SIGNED_PRE_KEY_ID));
	spk.appendChild(doc.createTextNode(signedPublic.toBase64()));
	bundle.appendChild(spk);

	QByteArray signature = signalProtocol->getSignedPreKeySignature();
	if (signature.isNull())
	{
		qCritical("Failed to get Signed Pre Key signature");
		return false;
	}
	QDomElement spks=doc.createElement(TAG_NAME_SPKS);
	spks.appendChild(doc.createTextNode(signature.toBase64()));
	bundle.appendChild(spks);

	QByteArray identityKeyPublic = signalProtocol->getIdentityKeyPublic();
	if (identityKeyPublic.isNull())
	{
		qCritical("Failed to get Identity");
		return false;
	}
	QDomElement ik=doc.createElement(TAG_NAME_IK);
	ik.appendChild(doc.createTextNode(identityKeyPublic.toBase64()));
	bundle.appendChild(ik);

	QDomElement prekeys=doc.createElement(TAG_NAME_PREKEYS);
	QMap<quint32, QByteArray> preKeys = signalProtocol->getPreKeys();
	for (QMap<quint32, QByteArray>::ConstIterator it=preKeys.constBegin();
		 it != preKeys.constEnd(); ++it)
	{
		QDomElement pk=doc.createElement(TAG_NAME_PK);
		pk.setAttribute(ATTR_NAME_ID, QString::number(it.key()));
		pk.appendChild(doc.createTextNode(it->toBase64()));
		prekeys.appendChild(pk);
	}

	bundle.appendChild(prekeys);

	IDataForm options;
	IDataField field;
	field.var="pubsub#max_items";
	field.value="max";
	options.fields.append(field);
	field.var="FORM_TYPE";
	field.type="hidden";
	field.value=NS_PUBSUB"#publish-options";
	options.fields.append(field);

	return FPepManager->publishItem(AStreamJid, NS_PEP_OMEMO_BUNDLES, item);
}

bool Omemo::removeOtherKeys(const Jid &AStreamJid)
{
	if (!FSignalProtocols.contains(AStreamJid))
		return false;
	SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
	quint32 deviceId = signalProtocol->getDeviceId();
	if (!deviceId)
		return false;

	QList<quint32> deviceIds = FDeviceIds.value(AStreamJid.bare());

	QDomDocument doc;
	for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
		 it != deviceIds.constEnd(); ++it)
		if (*it != deviceId)
		{
			QDomElement item=doc.createElement(TAG_NAME_ITEM);
			item.setAttribute(ATTR_NAME_ID, QString::number(*it));
			FPepManager->deleteItem(AStreamJid, NS_PEP_OMEMO_BUNDLES, item);
		}

	return true;
}

void Omemo::removeOtherDevices(const Jid &AStreamJid)
{
	QList<quint32> deviceIds = FDeviceIds.value(AStreamJid.bare());
	if (deviceIds.size() != 1)
	{
		removeOtherKeys(AStreamJid);
		deviceIds.clear();
		deviceIds.append(FSignalProtocols.value(AStreamJid)->getDeviceId());
		FDeviceIds.insert(AStreamJid.bare(), deviceIds);
		publishOwnDeviceIds(AStreamJid);
	}

	Options::node(OPV_OMEMO_RETRACT).removeNode("account",
		FAccountManager->findAccountByStream(AStreamJid)->accountId().toString());
}

QString Omemo::requestBundles4Devices(const Jid &AStreamJid, const QString &ABareJid, const QList<quint32> &ADevceIds)
{
	Stanza iq(STANZA_KIND_IQ);
	iq.setUniqueId()
	  .setType(STANZA_TYPE_GET)
	  .setTo(ABareJid);	
	QDomElement pubsub = iq.addElement(TAG_NAME_PUBSUB, NS_PUBSUB);
	QDomDocument doc = iq.document();
	QDomElement items = doc.createElement(TAG_NAME_ITEMS);
	items.setAttribute("node", NS_PEP_OMEMO_BUNDLES);
	for (QList<quint32>::ConstIterator it = ADevceIds.constBegin();
		 it != ADevceIds.constEnd(); ++it)
	{
		QDomElement item = doc.createElement(TAG_NAME_ITEM);
		item.setAttribute(ATTR_NAME_ID, QString::number(*it));
		items.appendChild(item);
	}
	pubsub.appendChild(items);

	if (FStanzaProcessor->sendStanzaRequest(this, AStreamJid, iq, 1000))
		return iq.id();
	else
		return QString::null;
}

void Omemo::bundlesProcessed(const Jid &AStreamJid, const QString &ABareJid)
{
	if (AStreamJid.bare()==ABareJid) // Own bundles
	{
		for (QMultiHash<QString,Stanza>::Iterator it = FPendingMessages.begin();
			 it != FPendingMessages.end();)
		{
			if (it->fromJid()==AStreamJid && !FPendingRequests.contains(it.key()))
			{
				encryptMessage(*it);
				FStanzaProcessor->sendStanzaOut(it->fromJid(), *it);
				it = FPendingMessages.erase(it);
			}
			else ++it;
		}
	}
	else
	{
		if (!FPendingRequests.contains(AStreamJid.bare()))
		{
			QList<Stanza> messages = FPendingMessages.values(ABareJid);
			for (QList<Stanza>::Iterator it = messages.begin();
				 it != messages.end(); ++it)
			{
				encryptMessage(*it);
				FStanzaProcessor->sendStanzaOut(it->fromJid(), *it);
			}
			FPendingMessages.remove(ABareJid);
		}
	}
}

void Omemo::encryptMessage(Stanza &AMessageStanza)
{
	if (!FSignalProtocols.contains(AMessageStanza.fromJid()))
		return;
	SignalProtocol *signalProtocol = FSignalProtocols[AMessageStanza.fromJid()];
	quint32 deviceId = signalProtocol->getDeviceId();
	if (deviceId)
	{
		QString bareJid = AMessageStanza.toJid().bare();

		QDomDocument doc = AMessageStanza.document();
		QDomElement encrypted = AMessageStanza.addElement(TAG_NAME_ENCRYPTED, NS_OMEMO);

		QByteArray keyHmac;
		QByteArray content = getContent(AMessageStanza);
		QByteArray data = encryptMessageContent(signalProtocol, content, keyHmac);

		QDomElement payload = doc.createElement(TAG_NAME_PAYLOAD);
		QDomText text = doc.createTextNode(data.toBase64());
		payload.appendChild(text);
		encrypted.appendChild(payload);

		QDomElement header = doc.createElement(TAG_NAME_HEADER);
		header.setAttribute(ATTR_NAME_SID, QString::number(deviceId));

		QStringList jids;
		if (AMessageStanza.type()!="groupchat")
			jids.append(bareJid);
//TODO: Implement groupchat	message encryption

		QHash<quint32, SignalDeviceBundle> bundles = FBundles.value(bareJid);
		for (QStringList::ConstIterator itj = jids.constBegin(); itj!=jids.constEnd(); ++itj)
		{
			QList<quint32> devices = FDeviceIds.value(*itj);			
			if (!devices.isEmpty())
			{
				QDomElement keys = doc.createElement(TAG_NAME_KEYS);
				keys.setAttribute("jid", *itj);
				header.appendChild(keys);

				for (QList<quint32>::ConstIterator it=devices.constBegin();
					 it != devices.constEnd(); ++it)
				{
					int sessionState = signalProtocol->sessionInitStatus(bareJid, *it);
					if (sessionState < 0)
						break;

					bool kex = sessionState < SignalProtocol::SessionAcknowledged;
					bool ok = sessionState > SignalProtocol::NoSession;

					if (!ok)
					{
						if (bundles.contains(*it))
						{
							const SignalDeviceBundle &b = bundles[*it];
							if (signalProtocol->getIdentityTrusted(bareJid, *it, b.FIdentityKey) == 1) // Send messages only for trusted identities!
							{								
								QList<quint32> keyIds = b.FPreKeys.keys();
								int keyCount = keyIds.size();
								int keyIndex = qrand()*keyCount/RAND_MAX;
								quint32 keyId = keyIds.at(keyIndex);
								QByteArray preKeyPublic = b.FPreKeys.value(keyId);

								quint32 registrationId = signalProtocol->getDeviceId();
								if (registrationId)
								{
									session_pre_key_bundle *bundle = signalProtocol->createPreKeyBundle(
													registrationId, *it, keyId, preKeyPublic,
													b.FSignedPreKeyId, b.FSignedPreKeyPublic,
													b.FSignedPreKeySignature, b.FIdentityKey);
									if (signalProtocol->getSessionBuilder(bareJid, *it)
											.processPreKeyBundle(bundle))
									{
										bundles.remove(*it); // Remove unneeded bundle
										ok = true;
									}
									else
										qCritical("Failed to process preKeyBundle()! Session is not built");
								}
							}
						}
					}

					if (ok)
					{
						SignalProtocol::Cipher cipher = signalProtocol->sessionCipherCreate(bareJid, *it);

						if (cipher.isNull())
							qCritical("Cipher is NULL!");
						else
						{
							QByteArray encryptedKey = cipher.encrypt(keyHmac);
							if (!encryptedKey.isNull())
							{
								QDomElement key = doc.createElement(TAG_NAME_KEY);
								key.setAttribute(ATTR_NAME_RID, QString::number(*it));
								if (kex)
									key.setAttribute(ATTR_NAME_KEX, "true");
								key.appendChild(doc.createTextNode(encryptedKey.toBase64()));
								keys.appendChild(key);
							}
						}
					}
				}
			}
		}

		encrypted.appendChild(header);
		QDomElement body = AMessageStanza.element().firstChildElement("body");
		AMessageStanza.element().removeChild(body);
		body = AMessageStanza.addElement(TAG_NAME_BODY, NS_JABBER_CLIENT);
		body.appendChild(doc.createTextNode(Options::node(OPV_OMEMO_FALLBACKMESSAGE).value().toString()));
		AMessageStanza.addElement("store", NS_HINTS);
	}
}

void Omemo::onOptionsOpened()
{
	QTimer::singleShot(0, this, SLOT(purgeDatabases()));
}

void Omemo::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.cleanPath()==OPV_OMEMO_RETRACT_ACCOUNT)
	{
		if (ANode.value().toBool())
		{
			IAccount *account = FAccountManager->findAccountById(ANode.nspace());
			if (account)
			{
				IPresence *presence = FPresenceManager->findPresence(account->streamJid());
				if (presence &&
					presence->isOpen() &&
					presence->show() != IPresence::Offline &&
					presence->show() != IPresence::Error) // Online
					removeOtherDevices(account->streamJid());
			}
		}
	}
}

void Omemo::purgeDatabases()
{	// Remove orphaned files
	Options *options = qobject_cast<Options *>(sender());
	QDir omemoDir(options->filesPath());
	omemoDir.cd(DIR_OMEMO);

	QStringList files = omemoDir.entryList(QStringList("{?\?\?\?\?\?\?\?-?\?\?\?-?\?\?\?-?\?\?\?-?\?\?\?\?\?\?\?\?\?\?\?}.db"),
											QDir::Files);
	QList<IAccount*> accounts = FAccountManager->accounts();

	for (QList<IAccount*>::ConstIterator it=accounts.constBegin();
		 it != accounts.constEnd(); ++it)
		files.removeOne((*it)->accountId().toString()+".db");

	for (QStringList::ConstIterator it=files.constBegin();
		 it != files.constEnd(); ++it)
		if (omemoDir.remove(*it))
			qDebug() << "Database file:" << *it << "deleted.";
		else
			qCritical() << "Failed to delete database file:" << *it;
}

void Omemo::onPresenceOpened(IPresence *APresence)
{
	IXmppStream *stream = FXmppStreamManager->findXmppStream(APresence->streamJid());
	QTimer *timer = new QTimer(this);
	FPepDelay.insert(stream, timer);
	connect(timer, SIGNAL(timeout()), SLOT(onPepTimeout()));
	timer->start(10000);

	publishOwnKeys(APresence->streamJid());
}

void Omemo::onPresenceClosed(IPresence *APresence)
{
	Q_UNUSED(APresence);
}

void Omemo::onPepTimeout()
{
	QTimer *timer = qobject_cast<QTimer*>(sender());
	for (QHash<IXmppStream*, QTimer*>::Iterator it = FPepDelay.begin();
		 it != FPepDelay.end();)
		if (*it == timer) {
			IXmppStream *stream = it.key();
			it = FPepDelay.erase(it);
			QList<quint32> ids;
			Jid streamJid = stream->streamJid();
			if (FSignalProtocols.contains(streamJid))
			{
				SignalProtocol *signalProtocol = FSignalProtocols[streamJid];
				quint32 id = signalProtocol->getDeviceId();
				if (id)
				{
					ids.append(id);
					FDeviceIds.insert(streamJid.bare(), ids);
					publishOwnDeviceIds(streamJid);
				}
			}
		}
		else
			++it;
	timer->deleteLater();
}

bool Omemo::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

	if (AMessage.data(MDR_MESSAGE_DIRECTION).toInt()==IMessageProcessor::DirectionOut)
		if (FSignalProtocols.contains(AStreamJid))
		{
			SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
			if (ADirection == IMessageProcessor::DirectionOut)
			{
				Stanza stanza(AMessage.stanza());
				if (stanza.firstElement(TAG_NAME_ENCRYPTED, NS_OMEMO).isNull() &&
					!stanza.firstElement(TAG_NAME_BODY).isNull() &&
					(AMessage.type() == Message::Chat || AMessage.type() == Message::Normal))
				{
					QString bareJid = stanza.toJid().bare();
					if (isActiveSession(AStreamJid, bareJid))
					{
						bool haveTrustedIdentities(false);
						if (isSupported(AStreamJid, stanza.toJid()))
						{
							QList<quint32> deviceIds = FDeviceIds[bareJid];
							QHash<quint32, SignalDeviceBundle> bundles = FBundles.value(bareJid);
							for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
								 it != deviceIds.constEnd(); ++it)
								if (signalProtocol->getIdentityTrusted(bareJid, *it) == 1) // Trusted identity
								{
									haveTrustedIdentities = true;
									break;
								}

	//TODO: Store list of failed devices to skip rerequesting bundles them every time
							if (!haveTrustedIdentities)
							{
								QMessageBox::warning(FMainWindowPlugin->mainWindow()->instance(),
													 bareJid,
													 tr("No trusted identities for the contact!\n"
														"You can't send an encrypted message."),
													 QMessageBox::Ok);
								return true;
							}
						}
					}
				}
			}
		}
	return false;
}

void Omemo::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	QList<quint32> deviceIds = FBundleRequests.values(AStanza.id());
	FBundleRequests.remove(AStanza.id());
	QString bareJid = AStanza.hasAttribute("from")?AStanza.fromJid().bare()
												  :AStreamJid.bare(); // Reply from own JID
	if (AStanza.type() == STANZA_TYPE_RESULT)
	{
		QDomElement items = AStanza.element()
								   .firstChildElement(TAG_NAME_PUBSUB)
								   .firstChildElement(TAG_NAME_ITEMS);
		if (!items.isNull())
		{
			if (items.attribute("node") == NS_PEP_OMEMO_BUNDLES)
			{
				for (QDomElement item = items.firstChildElement(TAG_NAME_ITEM);
					 !item.isNull(); item = item.nextSiblingElement(TAG_NAME_ITEM))
				{
					SignalDeviceBundle deviceBundle;
					bool ok = false;
					quint32 deviceId = item.attribute(ATTR_NAME_ID).toInt(&ok);
					if (ok)
					{
						if (!deviceIds.contains(deviceId))
							qWarning() << "Device ID:" << deviceId
									   << "is not expected in the result:" << deviceIds;
						else if (FDeviceIds.value(bareJid).contains(deviceId))
						{
							QDomElement bundle = item.firstChildElement(TAG_NAME_BUNDLE);

							QDomElement spk = bundle.firstChildElement(TAG_NAME_SPK);
							deviceBundle.FSignedPreKeyId = spk.attribute(ATTR_NAME_ID).toUInt(&ok);
							if (ok)
							{
								deviceBundle.FSignedPreKeyPublic = QByteArray::fromBase64(spk.text()
																							.toLatin1());
								QDomElement spks = bundle.firstChildElement(TAG_NAME_SPKS);
								deviceBundle.FSignedPreKeySignature = QByteArray::fromBase64(spks.text().toLatin1());
								QDomElement ik = bundle.firstChildElement(TAG_NAME_IK);
								deviceBundle.FIdentityKey = QByteArray::fromBase64(ik.text().toLatin1());

								QDomElement prekeys = bundle.firstChildElement(TAG_NAME_PREKEYS);
								for (QDomElement pk = prekeys.firstChildElement(TAG_NAME_PK);
									 !pk.isNull(); pk = pk.nextSiblingElement(TAG_NAME_PK))
								{
									quint32 id = pk.attribute(ATTR_NAME_ID).toUInt(&ok);
									if (ok)
										deviceBundle.FPreKeys.insert(id, QByteArray::fromBase64(
																			pk.text().toLatin1()));
									else
										qCritical("Invalid pre key ID!");
								}
								FBundles[bareJid].insert(deviceId, deviceBundle);
								FSignalProtocols.value(AStreamJid)->saveIdentity(bareJid, deviceId, deviceBundle.FIdentityKey);
							}
							else
								qCritical("Invalid signed pre key ID!");
						}
						else
							qDebug("Device ID for the bare JID is obsolete!");
					}
					else
						qCritical("Invalid device ID!");
				}
			}
			else
				qCritical("Invalid pub-sub node!");
		}
		else
			qCritical("Invalid stanza structure!");
	}

	bool removed(false);
	for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
		 it != deviceIds.constEnd(); ++it)
	{
		if (FPendingRequests.contains(bareJid, *it))
		{
			FPendingRequests.remove(bareJid, *it);
			removed = true;
		}
		else
			qWarning() << "No pending requests for bare JID:" << AStanza.fromJid().bare()
					   << "and device ID" << *it;
	}

	if (removed && !FPendingRequests.contains(bareJid))
		bundlesProcessed(AStreamJid, bareJid);
}

bool Omemo::onNewKeyReceived(const QString &ABareJid, quint32 ADeviceId, const QByteArray &AKeyData, bool AExists, SignalProtocol *ASignalProtocol)
{
	FMainWindowPlugin->mainWindow()->showWindow();
	int rc = QMessageBox::question(FMainWindowPlugin->mainWindow()->instance(),
								   ABareJid,
								   tr("A new identity key received for the device: %1\n"
									  "Identity key:\n%2%3"
									  "\nDo you trust it?").arg(ADeviceId)
														   .arg(SignalProtocol::calcFingerprint(
																ASignalProtocol->curveFromEd(AKeyData)))
														   .arg(AExists?QString("\n%1")
																			.arg(tr("But you already have a different key for it!"))
																	   :QString()),
								   QMessageBox::Yes,QMessageBox::No);
	return rc==QMessageBox::Yes;
}

void Omemo::onSessionStateChanged(const QString &ABareJid, quint32 ADeviceId, SignalProtocol *ASignalProtocol)
{
	Jid stremJid = FSignalProtocols.key(ASignalProtocol);
	if (stremJid.isValid())
	{
		if (isActiveSession(stremJid, ABareJid))
		{
			IMessageChatWindow *chatWindow = FMessageWidgets->findChatWindow(stremJid, ABareJid);
			if (chatWindow)
				updateChatWindowActions(chatWindow);
		}
	}
}

void Omemo::onSessionDeleted(const QString &ABareJid, quint32 ADeviceId, SignalProtocol *ASignalProtocol)
{
	Jid stremJid = FSignalProtocols.key(ASignalProtocol);
	if (stremJid.isValid())
	{
		if (isActiveSession(stremJid, ABareJid))
		{
			setActiveSession(stremJid, ABareJid, false);
			IMessageChatWindow *chatWindow = FMessageWidgets->findChatWindow(stremJid, ABareJid);
			if (chatWindow)
			{
				QList<QAction*> omemoActions = chatWindow->toolBarWidget()->toolBarChanger()
													->groupItems(TBG_MWTBW_OMEMO);
				if (!omemoActions.isEmpty())
					updateOmemoAction(chatWindow->toolBarWidget()->toolBarChanger()
									  ->handleAction(omemoActions.first()));
			}
		}
	}
}

static QString calcId(const QString &AStreamJid, const QString &ABareJid, int ADeviceId)
{
	return QString::fromLatin1(QCryptographicHash::hash(QString("omemo|%1|%2|%3").arg(AStreamJid)
																				 .arg(ABareJid)
																				 .arg(ADeviceId)
																				 .toUtf8(),
														QCryptographicHash::Md4).toHex());
}

void Omemo::setImage(IMessageChatWindow *AWindow, const Jid &AStreamJid,
					 const QString &ABareJid, int ADeviceId,
					 const QString &AImage, const QString &ATitle)
{
	QString id = calcId(AStreamJid.full(), ABareJid, ADeviceId);
	AWindow->viewWidget()->setImageUrl(id, QUrl::fromLocalFile(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)
																->fileFullName(AImage)).toString(), true);
	AWindow->viewWidget()->setObjectTitle(id, ATitle, true);
}

void Omemo::onIdentityTrustChanged(const QString &ABareJid, quint32 ADeviceId, const QByteArray &AEd25519Key, bool ATrusted, SignalProtocol *ASignalProtocol)
{
	Jid stremJid = FSignalProtocols.key(ASignalProtocol);
	if (stremJid.isValid())
	{
		IMessageChatWindow *chatWindow = FMessageWidgets->findChatWindow(stremJid, ABareJid);
		if (chatWindow)
		{
			if (ATrusted)
				setImage(chatWindow, stremJid, ABareJid, ADeviceId, MNI_EMPTY_BOX, QString());
			else
				setImage(chatWindow, stremJid, ABareJid, ADeviceId, MNI_CRYPTO_NO_TRUST, tr("From untrusted identity"));
		}
	}
}

bool Omemo::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder)
	int direction = AMessage.data(MDR_MESSAGE_DIRECTION).toInt();
	if (direction == IMessageProcessor::DirectionIn)
	{
		QDomElement encrypted = AMessage.stanza().firstElement(TAG_NAME_ENCRYPTED, NS_OMEMO);
		if (!encrypted.isNull())
		{
			QDomElement header = encrypted.firstChildElement(TAG_NAME_HEADER);
			if (header.isNull())
				qCritical("No <header/> element found!");
			else
			{
				bool ok;
				quint32 deviceId = header.attribute(ATTR_NAME_SID).toUInt(&ok);
				if (ok)
				{
					SignalProtocol *signalProtocol = FSignalProtocols.value(AMessage.toJid());
					bool trusted = signalProtocol->getIdentityTrusted(AMessage.fromJid().bare(), deviceId);
					if (!trusted)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool Omemo::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder)
	int direction = AMessage.data(MDR_MESSAGE_DIRECTION).toInt();
	if (direction == IMessageProcessor::DirectionIn)
	{
		QDomElement encrypted = AMessage.stanza().firstElement(TAG_NAME_ENCRYPTED, NS_OMEMO);
		if (!encrypted.isNull())
		{
			QDomElement header = encrypted.firstChildElement(TAG_NAME_HEADER);
			if (header.isNull())
				qCritical("No <header/> element found!");
			else
			{
				bool ok;
				quint32 deviceId = header.attribute(ATTR_NAME_SID).toUInt(&ok);
				if (ok)
				{
					SignalProtocol *signalProtocol = FSignalProtocols.value(AMessage.toJid());
					bool trusted = signalProtocol->getIdentityTrusted(AMessage.fromJid().bare(), deviceId);

					QTextCursor cursor(ADocument);
					cursor.movePosition(QTextCursor::End);
					QTextImageFormat image;
					image.setName(QUrl::fromLocalFile(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)
													  ->fileFullName(trusted?MNI_EMPTY_BOX
																			:MNI_CRYPTO_NO_TRUST)).toString());
					image.setProperty(QpXhtml::ObjectId, calcId(AMessage.toJid().full(), AMessage.fromJid().bare(), deviceId));
					if (!trusted)
						image.setToolTip(tr("From untrusted identity"));
					cursor.insertImage(image);
					return true;

				}
			}
		}
	}
	return false;
}

bool Omemo::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ADocument)
	Q_UNUSED(AMessage)
	Q_UNUSED(ALang)

	return false;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_omemo, Omemo)
#endif                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   

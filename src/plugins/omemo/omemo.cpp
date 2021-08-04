#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QMessageBox>

#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/messagewindowwidgets.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/shortcuts.h>

#include <utils/options.h>

#include "omemooptions.h"
#include "omemokeys.h"

extern "C" {
#include <gcrypt.h>
}

#include "omemo.h"

// PEP elements
#define TAG_NAME_PUBSUB					"pubsub"
#define TAG_NAME_LIST					"list"
#define TAG_NAME_DEVICE					"device"
#define TAG_NAME_BUNDLE					"bundle"
#define TAG_NAME_ITEMS					"items"
#define TAG_NAME_ITEM					"item"
#define TAG_NAME_IDENTITYKEY			"identityKey"
#define TAG_NAME_SIGNEDPREKEYPUBLIC		"signedPreKeyPublic"
#define TAG_NAME_SIGNEDPREKEYSIGNATURE "signedPreKeySignature"
#define TAG_NAME_PREKEYS				"prekeys"
#define TAG_NAME_PREKEYPUBLIC			"preKeyPublic"

// Message elements
#define TAG_NAME_BODY					"body"
#define TAG_NAME_ENCRYPTED				"encrypted"
#define TAG_NAME_HEADER					"header"
#define TAG_NAME_KEY					"key"
#define TAG_NAME_IV					    "iv"
#define TAG_NAME_PAYLOAD				"payload"

#define ATTR_NAME_PREKEY_ID				"preKeyId"
#define ATTR_NAME_SIGNED_PREKEY_ID		"signedPreKeyId"

#define ATTR_NAME_SID					"sid"
#define ATTR_NAME_RID					"rid"
#define ATTR_NAME_PREKEY				"prekey"

#define DIR_OMEMO						"omemo"

#define AES_128_KEY_LENGTH				16
#define AES_GCM_IV_LENGTH				16
#define AES_GCM_TAG_LENGTH				16

#define ADR_CONTACT_JID Action::DR_Parametr2
#define ADR_STREAM_JID Action::DR_StreamJid

#define SHC_MESSAGE "/message/body"
#define SHC_MESSAGE_ENCRYPTED "/message/encrypted[@xmlns='" NS_OMEMO "']"

#define VDATA_SIZE(A) A.data(), size_t(A.size())

static QByteArray encryptMessageText(const QString &AMessageText, const QByteArray &AKey,
								 const QByteArray &AIv, QByteArray &AAuthTag)
{
	gcry_error_t rc = SG_SUCCESS;
	char * errMsg = nullptr;

	gcry_cipher_hd_t cipherHd = nullptr;
	QByteArray outBuf;
	QByteArray inBuf(AMessageText.toUtf8());

	if(AIv.size() != AES_GCM_IV_LENGTH) {
		errMsg = "Invalid AES IV size (must be 16)";
		rc = gcry_error_t(SG_ERR_UNKNOWN);
		goto cleanup;
	}

	rc = gcry_cipher_open(&cipherHd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_GCM, GCRY_CIPHER_SECURE);
	if (rc) {
		errMsg = "Failed to init cipher";
		goto cleanup;
	}

	rc = gcry_cipher_setkey(cipherHd, VDATA_SIZE(AKey));
	if (rc) {
		errMsg = "Failed to set key";
		goto cleanup;
	}

	rc = gcry_cipher_setiv(cipherHd, VDATA_SIZE(AIv));
	if (rc) {
		errMsg = "Failed to set IV";
		goto cleanup;
	}

	outBuf.resize(inBuf.size());
	rc = gcry_cipher_encrypt(cipherHd, VDATA_SIZE(outBuf), VDATA_SIZE(inBuf));
	if (rc) {
		errMsg = "Failed to encrypt";
		goto cleanup;
	}

	AAuthTag.resize(AES_GCM_TAG_LENGTH);
	rc = gcry_cipher_gettag(cipherHd, VDATA_SIZE(AAuthTag));
	if (rc)
		errMsg = "Failed get authentication tag";

cleanup:
	if (rc) {
		if (rc > 0)
			qCritical("%s: %s (%s: %s)\n", __func__, errMsg,
					  gcry_strsource(rc), gcry_strerror(rc));
		else
			qCritical("%s: %s\n", __func__, errMsg);
	}

	gcry_cipher_close(cipherHd);

	return outBuf;
}

static QString decryptMessageText(const QByteArray &AEncryptedText, const QByteArray &AKey,
								  const QByteArray &AIv, const QByteArray &AAuthTag)
{
	gcry_error_t rc = SG_SUCCESS;
	char * errMsg = nullptr;

	gcry_cipher_hd_t cipherHd = nullptr;
	QByteArray outBuf;

	if(AIv.size() != AES_GCM_IV_LENGTH) {
	  errMsg = "Invalid AES IV size (must be 16)";
	  rc = gcry_error_t(SG_ERR_UNKNOWN);
	  goto cleanup;
	}

	rc = gcry_cipher_open(&cipherHd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_GCM, GCRY_CIPHER_SECURE);
	if (rc) {
	  errMsg = "failed to init cipher";
	  goto cleanup;
	}

	rc = gcry_cipher_setkey(cipherHd, VDATA_SIZE(AKey));
	if (rc) {
	  errMsg = "failed to set key";
	  goto cleanup;
	}

	rc = gcry_cipher_setiv(cipherHd, VDATA_SIZE(AIv));
	if (rc) {
		errMsg = "Failed to set IV";
		goto cleanup;
	}

	outBuf.resize(AEncryptedText.size());
	rc = gcry_cipher_decrypt(cipherHd, VDATA_SIZE(outBuf), VDATA_SIZE(AEncryptedText));
	if (rc) {
		errMsg = "failed to decrypt";
		goto cleanup;
	}

	rc = gcry_cipher_checktag(cipherHd, VDATA_SIZE(AAuthTag));
	if (rc)
		errMsg = "failed check authentication tag";

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

	return outBuf.isNull()?QString():QString::fromUtf8(outBuf);
}

static void getKeyPair(QByteArray &AKey, QByteArray &AIv)
{
	AKey.resize(AES_128_KEY_LENGTH);
	gcry_randomize(VDATA_SIZE(AKey), GCRY_STRONG_RANDOM);
	AIv.resize(AES_GCM_IV_LENGTH);
	gcry_randomize(VDATA_SIZE(AIv), GCRY_STRONG_RANDOM);
}

Omemo::Omemo(): FAccountManager(nullptr),
				FOptionsManager(nullptr),
				FPepManager(nullptr),
				FStanzaProcessor(nullptr),
				FXmppStreamManager(nullptr),
				FPresenceManager(nullptr),
				FDiscovery(nullptr),
				FMessageWidgets(nullptr),
				FPluginManager(nullptr),
				FIconStorage(nullptr),
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

		IOptionsDialogNode omemoNode = { ONO_P2P_OMEMO, OPN_P2P_OMEMO, MNI_CRYPTO_FULL, tr("OMEMO Keys") };
		FOptionsManager->insertOptionsDialogNode(omemoNode);

		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

bool Omemo::initSettings()
{
	Options::setDefaultValue(OPV_OMEMO_RETRACT_ACCOUNT, false);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> Omemo::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	Q_UNUSED(AParent);
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
		if(!items.isNull())
		{
			QDomElement item  = items.firstChildElement(TAG_NAME_ITEM);
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
						bool ok;
						quint32 id = ida.toUInt(&ok);
						if (ok)
							ids.append(id);
						else
							qCritical() << "Invalid id attribute value:" << ida;
					}

					if (!ids.isEmpty())
					{
						if (bareJid == AStreamJid.bare()) // Own ID list
						{
							IXmppStream *stream = FXmppStreamManager->findXmppStream(AStreamJid);
							if (FPepDelay.contains(stream))
							{
								QTimer *timer = FPepDelay.take(stream);
								timer->stop();
								timer->deleteLater();
							}

							bool cleanup = Options::node(OPV_OMEMO_RETRACT)
									.value("account",
										   FAccountManager->findAccountByStream(AStreamJid)->accountId().toString())
									.toBool();

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

						// Remove missing bundles from the hash
						for (QMultiHash<QString,SignalDeviceBundle>::Iterator it=FBundles.begin();
							 it != FBundles.constEnd();)
							if (it.key()==bareJid &&
								!ids.contains(it->FDeviceId))
								it = FBundles.erase(it);
							else
								++it;
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
		QString iconKey = MNI_CRYPTO_OFF;
		if (isActiveSession(AWindow->address()))
		{
			iconKey = MNI_CRYPTO_ON;
			QString bareJid = AWindow->address()->contactJid().bare();
			if (FDeviceIds.contains(bareJid))
			{
				const QList<quint32> &devices = FDeviceIds[bareJid];
				SignalProtocol *signalProtocol = FSignalProtocols[AWindow->address()->streamJid()];
				for (QList<quint32>::ConstIterator itc=devices.constBegin();
					 itc != devices.constEnd(); ++itc)
					if (signalProtocol->sessionInitStatus(bareJid, *itc)
							==SignalProtocol::SessionAcknowledged)
					{
						iconKey = MNI_CRYPTO_FULL;
						break;
					}
			}
		}
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

	SignalProtocol *signalProtocol = new SignalProtocol(omemoDir.filePath(name+".db"), name, this);
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
	qDebug() << "Omemo::publishOwnDeviceIds(" << AStreamJid.full() << ")";
	if (!FDeviceIds.contains(AStreamJid.bare()))
		return false;
	QList<quint32> ids = FDeviceIds.value(AStreamJid.bare());
	if (ids.isEmpty())
	{
		qWarning() << "Own device ID list is empty!";
		return false;
	}

	QDomDocument doc;
	QDomElement item=doc.createElement(TAG_NAME_ITEM);
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
	qDebug() << "Omemo::publishOwnKeys(" << AStreamJid.full() << ")";
	if (!FSignalProtocols.contains(AStreamJid))
		return false;
	SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
	quint32 deviceId = signalProtocol->getDeviceId();
	if (!deviceId)
		return false;
	QDomDocument doc;
	QDomElement item=doc.createElement(TAG_NAME_ITEM);
	item.setAttribute("id", "current");

	QDomElement bundle=doc.createElementNS(NS_OMEMO, TAG_NAME_BUNDLE);
	item.appendChild(bundle);

	QByteArray signedPublic = signalProtocol->getSignedPreKeyPublic();
	if (signedPublic.isNull())
	{
		qCritical() << "Failed to get Signed Pre Key";
		return false;
	}
	QDomElement signedPreKeyPublic=doc.createElement(TAG_NAME_SIGNEDPREKEYPUBLIC);
	signedPreKeyPublic.setAttribute(ATTR_NAME_SIGNED_PREKEY_ID,
									QString::number(SIGNED_PRE_KEY_ID));
	signedPreKeyPublic.appendChild(doc.createTextNode(signedPublic.toBase64()));
	bundle.appendChild(signedPreKeyPublic);

	QByteArray signature = signalProtocol->getSignedPreKeySignature();
	if (signature.isNull())
	{
		qCritical() << "Failed to get Signed Pre Key signature";
		return false;
	}
	QDomElement signedPreKeySignature=doc.createElement(TAG_NAME_SIGNEDPREKEYSIGNATURE);
	signedPreKeySignature.appendChild(doc.createTextNode(signature.toBase64()));
	bundle.appendChild(signedPreKeySignature);

	QByteArray identityKeyPublic = signalProtocol->getIdentityKeyPublic();
	qDebug() << "identityKeyPublic = " << identityKeyPublic.toHex();
	if (identityKeyPublic.isNull())
	{
		qCritical() << "Failed to get Signed Pre Key";
		return false;
	}
	QDomElement identityKey=doc.createElement(TAG_NAME_IDENTITYKEY);
	identityKey.appendChild(doc.createTextNode(identityKeyPublic.toBase64()));
	bundle.appendChild(identityKey);

	QDomElement prekeys=doc.createElement(TAG_NAME_PREKEYS);
	QMap<quint32, QByteArray> preKeys = signalProtocol->getPreKeys();
	for (QMap<quint32, QByteArray>::ConstIterator it=preKeys.constBegin();
		 it != preKeys.constEnd(); ++it)
	{
		QDomElement preKeyPublic=doc.createElement(TAG_NAME_PREKEYPUBLIC);
		preKeyPublic.setAttribute(ATTR_NAME_PREKEY_ID, QString::number(it.key()));
		preKeyPublic.appendChild(doc.createTextNode(it->toBase64()));
		prekeys.appendChild(preKeyPublic);
	}

	bundle.appendChild(prekeys);

	return FPepManager->publishItem(AStreamJid, QString("%1:%2").arg(NS_PEP_OMEMO_BUNDLES)
									.arg(deviceId), item);
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
	QDomElement item=doc.createElement(TAG_NAME_ITEM);
	item.setAttribute("id", "current");
	QString ns(NS_PEP_OMEMO_BUNDLES":%1");

	for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
		 it != deviceIds.constEnd(); ++it)
		if (*it != deviceId)
			FPepManager->deleteItem(AStreamJid, ns.arg(*it), item);

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

QString Omemo::requestDeviceBundle(const Jid &AStreamJid, const QString &ABareJid, quint32 ADevceId)
{
	Stanza iq(STANZA_KIND_IQ);
	iq.setUniqueId()
	  .setType(STANZA_TYPE_GET)
	  .setTo(ABareJid);
	QDomElement pubsub = iq.addElement(TAG_NAME_PUBSUB, NS_PUBSUB);
	QDomElement items = iq.document().createElement(TAG_NAME_ITEMS);
	items.setAttribute("node", QString("%1:%2").arg(NS_PEP_OMEMO_BUNDLES)
											   .arg(ADevceId));
	pubsub.appendChild(items);
	if (FStanzaProcessor->sendStanzaRequest(this, AStreamJid, iq, 1000))
		return iq.id();
	else
		return QString::null;
}

void Omemo::bundlesProcessed(const QString &ABareJid)
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

void Omemo::encryptMessage(Stanza &AMessageStanza)
{
	if (!FSignalProtocols.contains(AMessageStanza.fromJid()))
		return;
	SignalProtocol *signalProtocol = FSignalProtocols[AMessageStanza.fromJid()];
	quint32 deviceId = signalProtocol->getDeviceId();
	if (deviceId)
	{
		QString bareJid = AMessageStanza.toJid().bare();
		QList<SignalDeviceBundle> bundles = FBundles.values(bareJid);
		QList<quint32> devices = FDeviceIds.value(bareJid);

		QDomDocument doc = AMessageStanza.document();
		QDomElement encrypted = AMessageStanza.addElement(TAG_NAME_ENCRYPTED, NS_OMEMO);

		QByteArray keyData, ivData;
		getKeyPair(keyData, ivData);

		QDomElement body = AMessageStanza.firstElement(TAG_NAME_BODY);
		const QString message = body.text();

		QByteArray authTag;
		QByteArray data = encryptMessageText(message, keyData, ivData, authTag);
		QDomElement payload = doc.createElement(TAG_NAME_PAYLOAD);
		QDomText text = doc.createTextNode(data.toBase64());
		payload.appendChild(text);
		encrypted.appendChild(payload);

		QDomElement header = doc.createElement(TAG_NAME_HEADER);
		header.setAttribute(ATTR_NAME_SID, QString::number(deviceId));

		for (QList<quint32>::ConstIterator it=devices.constBegin();
			 it != devices.constEnd(); ++it)
		{
			int sessionState = signalProtocol->sessionInitStatus(bareJid, *it);
			if (sessionState < 0)
				break;

			bool prekey = sessionState < SignalProtocol::SessionAcknowledged;
			bool ok = sessionState > SignalProtocol::NoSession;

			if (!ok)
			{
//FIXME: Store bundles associated with device IDs to improve performance of this search
				for(QList<SignalDeviceBundle>::ConstIterator itb=bundles.constBegin();
					itb != bundles.constEnd(); ++itb)
					if (itb->FDeviceId == *it)
					{
						QList<quint32> keyIds = itb->FPreKeys.keys();
						int keyCount = keyIds.size();
						int keyIndex = qrand()*keyCount/RAND_MAX;
						quint32 keyId = keyIds.at(keyIndex);
						QByteArray preKeyPublic = itb->FPreKeys.value(keyId);

						quint32 registrationId = signalProtocol->getDeviceId();
						if (registrationId)
						{
							session_pre_key_bundle *bundle = signalProtocol->createPreKeyBundle(
											registrationId, *it, keyId, preKeyPublic,
											itb->FSignedPreKeyId, itb->FSignedPreKeyPublic,
											itb->FSignedPreKeySignature, itb->FIdentityKey);
							if (signalProtocol->getSessionBuilder(bareJid, *it)
									.processPreKeyBundle(bundle))
								ok = true;
							else
								qCritical("Failed to process preKeyBundle()! Session is not built");
						}
						break;
					}
			}

			if (ok)
			{
				SignalProtocol::Cipher cipher = signalProtocol->sessionCipherCreate(bareJid, *it);

				if (cipher.isNull())
					qCritical("Cipher is NULL!");
				else
				{
					QByteArray encryptedKey = cipher.encrypt(keyData+authTag);
					if (!encryptedKey.isNull())
					{
						QDomElement key = doc.createElement(TAG_NAME_KEY);
						key.setAttribute(ATTR_NAME_RID, QString::number(*it));
						if (prekey)
							key.setAttribute(ATTR_NAME_PREKEY, "true");
						key.appendChild(doc.createTextNode(encryptedKey.toBase64()));
						header.appendChild(key);
					}
				}
			}
		}

		QDomElement iv = doc.createElement(TAG_NAME_IV);
		iv.appendChild(doc.createTextNode(ivData.toBase64()));
		header.appendChild(iv);
		encrypted.appendChild(header);
		AMessageStanza.element().removeChild(body);
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

bool Omemo::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (FSignalProtocols.contains(AStreamJid))
	{
		SignalProtocol *signalProtocol = FSignalProtocols[AStreamJid];
		if (AHandleId==FSHIMessageOut)
		{
			if (AStanza.firstElement(TAG_NAME_ENCRYPTED, NS_OMEMO).isNull())
			{
				QString bareJid = AStanza.toJid().bare();
				if (isActiveSession(AStreamJid, bareJid))
				{
					if (isSupported(AStreamJid, AStanza.toJid()))
					{
						QList<quint32> deviceIds = FDeviceIds[bareJid];
						quint32 deviceId = signalProtocol->getDeviceId();
						if (deviceId)
						{
							// Add own device IDs except this one
							QList<quint32> ownDeviceIds = FDeviceIds[AStreamJid.bare()];
							ownDeviceIds.removeOne(deviceId);
							deviceIds.append(ownDeviceIds);
						}

						bool needBundles = false;
						for (QList<quint32>::ConstIterator it = deviceIds.constBegin();
							 it != deviceIds.constEnd(); ++it)
						{
							int sessionState = signalProtocol->sessionInitStatus(bareJid, *it);
							if (sessionState == SignalProtocol::NoSession)
							{
								needBundles = true;
								if (!FPendingRequests.contains(bareJid, *it))
								{
									QString id = requestDeviceBundle(AStreamJid, bareJid, *it);
									if (!id.isNull())
									{
										FBundleRequests.insert(id, *it);
										FPendingRequests.insert(bareJid, *it);
									}
								}
							}
						}
						if (needBundles)
						{
							FPendingMessages.insert(AStanza.toJid().bare(), AStanza);
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
								QDomElement iv = header.firstChildElement(TAG_NAME_IV);
								if (!iv.isNull())
								{
									QByteArray ivData = QByteArray::fromBase64(iv.text().toLatin1());

									quint32 deviceId = signalProtocol->getDeviceId();
									for (QDomElement key = header.firstChildElement(TAG_NAME_KEY);
										 !key.isNull(); key = key.nextSiblingElement(TAG_NAME_KEY))
									{
										if (key.hasAttribute(ATTR_NAME_RID))
										{
											quint32 rid = key.attribute(ATTR_NAME_RID).toUInt(&ok);
											if (ok && rid)
											{
												if (rid==deviceId)
												{
													QByteArray decoded = QByteArray::fromBase64(key.text().toLatin1());
													SignalProtocol::Cipher cipher = signalProtocol->sessionCipherCreate(AStanza.fromJid().bare(), sid);
													QByteArray keyTuple;
													if (key.attribute(ATTR_NAME_PREKEY)=="true")
													{
														SignalProtocol::PreKeySignalMessage message = signalProtocol->getPreKeySignalMessage(decoded);
														bool preKeyUpdated;
														keyTuple = cipher.decrypt(message, preKeyUpdated);
														if (preKeyUpdated)
															publishOwnKeys(AStreamJid);
													}
													else
													{
														SignalProtocol::SignalMessage message = signalProtocol->getSignalMessage(decoded);
														keyTuple = cipher.decrypt(message);
													}
													QByteArray encryptedText = QByteArray::fromBase64(payload.text().toLatin1());
													QString decryptedText = decryptMessageText(encryptedText, keyTuple.left(AES_128_KEY_LENGTH),
																							   ivData, keyTuple.mid(AES_128_KEY_LENGTH));
													AStanza.element().removeChild(encrypted);
													if (decryptedText.isEmpty())
														decryptedText = tr("Failed to decrypt message");
													else
														if (!isActiveSession(AStreamJid, AStanza.fromJid().bare()))
															setActiveSession(AStreamJid, AStanza.fromJid().bare());
													QDomText text = AStanza.document().createTextNode(decryptedText);
													QDomElement body = AStanza.firstElement(TAG_NAME_BODY);
													if (!body.isNull())
														AStanza.element().removeChild(body);
													AStanza.addElement(TAG_NAME_BODY).appendChild(text);
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

void Omemo::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	quint32 deviceId = FBundleRequests.take(AStanza.id());
	QString bareJid = AStanza.fromJid().bare();

	if (AStanza.type() == STANZA_TYPE_RESULT)
	{
		bool ok = false;
		QDomElement items = AStanza.element()
								   .firstChildElement(TAG_NAME_PUBSUB)
								   .firstChildElement(TAG_NAME_ITEMS);
		if (!items.isNull())
		{
			QString node = items.attribute("node");

			QString start(NS_PEP_OMEMO_BUNDLES":");
			if (node.startsWith(start))
			{
				SignalDeviceBundle deviceBundle;
				deviceBundle.FDeviceId = node.mid(start.size()).toUInt(&ok);
				if (ok)
				{
					if (FDeviceIds.value(bareJid).contains(deviceBundle.FDeviceId))
					{
						if (deviceId != deviceBundle.FDeviceId)
							qWarning() << "Device IDs do not match:" << deviceId
									   << "/" << deviceBundle.FDeviceId;

						QDomElement item = items.firstChildElement(TAG_NAME_ITEM);
						QDomElement bundle = item.firstChildElement(TAG_NAME_BUNDLE);

						QDomElement signedPreKeyPublic = bundle.firstChildElement(TAG_NAME_SIGNEDPREKEYPUBLIC);
						deviceBundle.FSignedPreKeyId = signedPreKeyPublic.attribute(ATTR_NAME_SIGNED_PREKEY_ID)
																			.toUInt(&ok);
						if (ok)
						{
							deviceBundle.FSignedPreKeyPublic = QByteArray::fromBase64(signedPreKeyPublic.text()
																						.toLatin1());
							QDomElement signedPreKeySignature = bundle.firstChildElement(TAG_NAME_SIGNEDPREKEYSIGNATURE);
							deviceBundle.FSignedPreKeySignature = QByteArray::fromBase64(signedPreKeySignature.text()
																							.toLatin1());
							QDomElement identityKey = bundle.firstChildElement(TAG_NAME_IDENTITYKEY);
							deviceBundle.FIdentityKey = QByteArray::fromBase64(identityKey.text().toLatin1());

							QDomElement prekeys = bundle.firstChildElement(TAG_NAME_PREKEYS);
							for (QDomElement preKeyPublic = prekeys.firstChildElement(TAG_NAME_PREKEYPUBLIC);
								 !preKeyPublic.isNull();
								 preKeyPublic = preKeyPublic.nextSiblingElement(TAG_NAME_PREKEYPUBLIC))
							{
								quint32 id = preKeyPublic.attribute(ATTR_NAME_PREKEY_ID).toUInt(&ok);
								if (ok)
									deviceBundle.FPreKeys.insert(id,
																 QByteArray::fromBase64(
																	 preKeyPublic.text().toLatin1()));
								else
									qCritical("Invalid pre key ID!");
							}
							FBundles.insert(bareJid, deviceBundle);
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
			else
				qCritical("Invalid pub-sub node!");
		}
		else
			qCritical("Invalid stanza structure!");

		FPendingRequests.remove(bareJid, deviceId);
		if (!FPendingRequests.contains(bareJid))
			bundlesProcessed(bareJid);
	}
	else
	{
		if (!FPendingRequests.contains(bareJid, deviceId))
			qWarning() << "No pending requests for bare JID:" << AStanza.fromJid().bare()
					   << "and device ID" << deviceId;

		FPendingRequests.remove(bareJid, deviceId);

		if (!FPendingRequests.contains(bareJid))
			bundlesProcessed(bareJid);
	}
}

bool Omemo::onNewKeyReceived(const QString &AName, const QByteArray &AKeyData)
{
/*
	int rc = QMessageBox::question(nullptr,
								   tr("A new identity key received from %1").arg(AName),
								   tr("%1\n\nDo you trust it?").arg(SignalProtocol::calcFingerprint(AKeyData)),
								   QMessageBox::Yes,QMessageBox::No);
	qDebug() << "rc=" << rc;
	bool retval = rc==QMessageBox::Yes;
	qDebug() << "returning:" << retval;
*/
	return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_omemo, Omemo)
#endif                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   

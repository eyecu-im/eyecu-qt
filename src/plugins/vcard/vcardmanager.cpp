#include "vcardmanager.h"

#include <QFile>
#include <QFileInfo>
#include <QClipboard>
#include <QDomDocument>
#include <QApplication>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/shortcuts.h>
#include <definitions/namespaces.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/vcardvaluenames.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/multiuserdataroles.h>
#include <definitions/xmppurihandlerorders.h>
#include <definitions/rosterdataholderorders.h>
#include <utils/widgetmanager.h>
#include <utils/textmanager.h>
#include <utils/xmpperror.h>
#include <utils/shortcuts.h>
#include <utils/options.h>
#include <utils/stanza.h>
#include <utils/action.h>
#include <utils/logger.h>

#define DIR_VCARDS                "vcards"
#define VCARD_TIMEOUT             60000

#define MAX_VCARD_IMAGE_SIZE      QSize(96,96)
#define DEFAULT_IMAGE_FORMAT     "PNG"

#define UPDATE_VCARD_DAYS         7
#define UPDATE_TIMEOUT            5000

#define ADR_STREAM_JID            Action::DR_StreamJid
#define ADR_CONTACT_JID           Action::DR_Parametr1
#define ADR_CLIPBOARD_DATA        Action::DR_Parametr1

static const QList<int> VCardRosterKinds = QList<int>() << RIK_STREAM_ROOT << RIK_CONTACT << RIK_AGENT << RIK_METACONTACT << RIK_METACONTACT_ITEM;

VCardManager::VCardManager()
{
	FPluginManager = NULL;
	FXmppStreamManager = NULL;
	FRosterManager = NULL;
	FRostersModel = NULL;
	FRostersView = NULL;
	FRostersViewPlugin = NULL;
	FStanzaProcessor = NULL;
	FMultiChatManager = NULL;
	FDiscovery = NULL;
	FXmppUriQueries = NULL;
	FMessageWidgets = NULL;
	FRosterSearch = NULL;
	FOptionsManager = NULL;

	FUpdateTimer.setSingleShot(true);
	FUpdateTimer.start(UPDATE_TIMEOUT);
	connect(&FUpdateTimer,SIGNAL(timeout()),SLOT(onUpdateTimerTimeout()));
}

VCardManager::~VCardManager()
{

}

void VCardManager::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Visit Card Manager");
	APluginInfo->description = tr("Allows to obtain personal contact information");
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->version = "1.0";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool VCardManager::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	FPluginManager = APluginManager;

	IPlugin *plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamActiveChanged(IXmppStream *, bool)),SLOT(onXmppStreamActiveChanged(IXmppStream *, bool)));
		}
	}

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
		if (FRosterManager)
		{
			connect(FRosterManager->instance(),SIGNAL(rosterOpened(IRoster *)),SLOT(onRosterOpened(IRoster *)));
			connect(FRosterManager->instance(),SIGNAL(rosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)),
				SLOT(onRosterItemReceived(IRoster *, const IRosterItem &, const IRosterItem &)));
			connect(FRosterManager->instance(),SIGNAL(rosterClosed(IRoster *)),SLOT(onRosterClosed(IRoster *)));
		}
	}

	plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
	if (plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (FRostersViewPlugin)
		{
			FRostersView = FRostersViewPlugin->rostersView();
			connect(FRostersView->instance(),SIGNAL(indexContextMenu(const QList<IRosterIndex *> &, quint32, Menu *)), 
				SLOT(onRostersViewIndexContextMenu(const QList<IRosterIndex *> &, quint32, Menu *)));
			connect(FRostersViewPlugin->rostersView()->instance(),SIGNAL(indexClipboardMenu(const QList<IRosterIndex *> &, quint32, Menu *)),
				SLOT(onRostersViewIndexClipboardMenu(const QList<IRosterIndex *> &, quint32, Menu *)));
		}
	}

	plugin = APluginManager->pluginInterface("IMultiUserChatManager").value(0,NULL);
	if (plugin)
	{
		FMultiChatManager = qobject_cast<IMultiUserChatManager *>(plugin->instance());
		if (FMultiChatManager)
		{
			connect(FMultiChatManager->instance(),SIGNAL(multiUserContextMenu(IMultiUserChatWindow *,IMultiUser *, Menu *)),
				SLOT(onMultiUserContextMenu(IMultiUserChatWindow *,IMultiUser *, Menu *)));
		}
	}

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IXmppUriQueries").value(0,NULL);
	if (plugin)
	{
		FXmppUriQueries = qobject_cast<IXmppUriQueries *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(), SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onMessageNormalWindowCreated(IMessageNormalWindow *)));
			connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onMessageChatWindowCreated(IMessageChatWindow *)));
		}
	}

	plugin = APluginManager->pluginInterface("IRosterSearch").value(0,NULL);
	if (plugin)
	{
		FRosterSearch = qobject_cast<IRosterSearch *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	connect(Shortcuts::instance(),SIGNAL(shortcutActivated(const QString &, QWidget *)),SLOT(onShortcutActivated(const QString &, QWidget *)));

	return true;
}

bool VCardManager::initObjects()
{
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_SHOWVCARD, tr("Show contact profile"), tr("Ctrl+I","Show contact profile"));
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_SHOWVCARD, tr("Show contact profile"), tr("Ctrl+I","Show contact profile"), Shortcuts::WidgetShortcut);

	FVCardFilesDir.setPath(FPluginManager->homePath());
	if (!FVCardFilesDir.exists(DIR_VCARDS))
		FVCardFilesDir.mkdir(DIR_VCARDS);
	FVCardFilesDir.cd(DIR_VCARDS);

	if (FRostersView)
	{
		Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_SHOWVCARD,FRostersView->instance());
	}
	if (FDiscovery)
	{
		registerDiscoFeatures();
	}
	if (FXmppUriQueries)
	{
		FXmppUriQueries->insertUriHandler(XUHO_DEFAULT,this);
	}
	if (FRostersModel)
	{
		FRostersModel->insertRosterDataHolder(RDHO_VCARD_SEARCH,this);
	}
	if (FRosterSearch)
	{
		FRosterSearch->insertSearchField(RDR_VCARD_SEARCH,tr("User Profile"));
	}
	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}
	return true;
}

bool VCardManager::initSettings()
{
	Options::setDefaultValue(OPV_COMMON_RESTRICT_VCARD_IMAGES_SIZE,true);

	return true;
}

QList<int> VCardManager::rosterDataRoles(int AOrder) const
{
	if (AOrder == RDHO_VCARD_SEARCH)
	{
		static const QList<int> dataRoles = QList<int>() << RDR_VCARD_SEARCH;
		return dataRoles;
	}
	return QList<int>();
}

QVariant VCardManager::rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const
{
	if (AOrder==RDHO_VCARD_SEARCH && ARole==RDR_VCARD_SEARCH)
	{
		Jid contactJid = AIndex->data(RDR_PREP_BARE_JID).toString();
		if (FSearchStrings.contains(contactJid))
		{
			return FSearchStrings.value(contactJid);
		}
		else if (hasVCard(contactJid))
		{
			VCard *vcard = new VCard(const_cast<VCardManager *>(this),contactJid);

			QStringList strings;
			strings += vcard->value(VVN_FULL_NAME);
			strings += vcard->value(VVN_FAMILY_NAME);
			strings += vcard->value(VVN_GIVEN_NAME);
			strings += vcard->value(VVN_MIDDLE_NAME);
			strings += vcard->value(VVN_NICKNAME);
			strings += vcard->value(VVN_ORG_NAME);
			strings += vcard->value(VVN_ORG_UNIT);
			strings += vcard->value(VVN_TITLE);
			strings += vcard->value(VVN_ROLE);
			strings += vcard->value(VVN_DESCRIPTION);

			static const QStringList emailTagList = QStringList() << "HOME" << "WORK" << "INTERNET" << "X400";
			strings += vcard->values(VVN_EMAIL,emailTagList).keys();

			static const QStringList phoneTagList = QStringList() << "HOME" << "WORK" << "CELL" << "MODEM";
			strings += vcard->values(VVN_TELEPHONE,phoneTagList).keys();

			static const QStringList tagHome = QStringList() << "HOME";
			static const QStringList tagWork = QStringList() << "WORK";
			static const QStringList tagsAdres = tagHome+tagWork;
			strings += vcard->value(VVN_ADR_COUNTRY,tagHome,tagsAdres);
			strings += vcard->value(VVN_ADR_REGION,tagHome,tagsAdres);
			strings += vcard->value(VVN_ADR_CITY,tagHome,tagsAdres);
			strings += vcard->value(VVN_ADR_STREET,tagHome,tagsAdres);
			strings += vcard->value(VVN_ADR_COUNTRY,tagWork,tagsAdres);
			strings += vcard->value(VVN_ADR_REGION,tagWork,tagsAdres);
			strings += vcard->value(VVN_ADR_CITY,tagWork,tagsAdres);
			strings += vcard->value(VVN_ADR_STREET,tagWork,tagsAdres);

			delete vcard;

			strings.removeAll(QString());
			FSearchStrings.insert(contactJid, strings);

			return strings;
		}
	}
	return QVariant();
}

bool VCardManager::setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole)
{
	Q_UNUSED(AOrder); Q_UNUSED(AValue); Q_UNUSED(AIndex); Q_UNUSED(ARole);
	return false;
}

void VCardManager::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	Q_UNUSED(AStreamJid);
	if (FVCardRequestId.contains(AStanza.id()))
	{
		Jid fromJid = FVCardRequestId.take(AStanza.id());
		QDomElement elem = AStanza.firstElement(VCARD_TAGNAME,NS_VCARD_TEMP);
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(AStreamJid,QString("User vCard loaded, jid=%1, id=%2").arg(fromJid.full(),AStanza.id()));
			saveVCardFile(fromJid,elem);
			emit vcardReceived(fromJid);
		}
		else
		{
			XmppStanzaError err(AStanza);
			LOG_STRM_WARNING(AStreamJid,QString("Failed to load user vCard, jid=%1, id=%2: %3").arg(fromJid.full(),AStanza.id(),err.condition()));
			saveVCardFile(fromJid,QDomElement());
			emit vcardError(fromJid,err);
		}
	}
	else if (FVCardPublishId.contains(AStanza.id()))
	{
		Stanza stanza = FVCardPublishId.take(AStanza.id());
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(AStreamJid,QString("Self vCard published, id=%1").arg(AStanza.id()));
			saveVCardFile(AStreamJid.bare(),stanza.element().firstChildElement(VCARD_TAGNAME));
			emit vcardPublished(AStreamJid);
		}
		else
		{
			XmppStanzaError err(AStanza);
			LOG_STRM_WARNING(AStreamJid,QString("Failed to publish self vCard, id=%1: %2").arg(AStanza.id(),err.condition()));
			emit vcardError(AStreamJid,err);
		}
	}
}

QMultiMap<int, IOptionsDialogWidget *> VCardManager::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager && ANodeId==OPN_COMMON)
	{
		widgets.insertMulti(OWO_COMMON_VCARDIMAGE,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_COMMON_RESTRICT_VCARD_IMAGES_SIZE),tr("Restrict maximum vCard images size"),AParent));
	}
	return widgets;
}

bool VCardManager::xmppUriOpen(const Jid &AStreamJid, const Jid &AContactJid, const QString &AAction, const QMultiMap<QString, QString> &AParams)
{
	Q_UNUSED(AParams);
	if (AAction == "vcard")
		return showVCardDialog(AStreamJid, AContactJid)!=NULL;
	return false;
}

QString VCardManager::vcardFileName(const Jid &AContactJid) const
{
	return FVCardFilesDir.absoluteFilePath(Jid::encode(AContactJid.pFull())+".xml");
}

bool VCardManager::hasVCard(const Jid &AContactJid) const
{
	return QFile::exists(vcardFileName(AContactJid));
}

IVCard *VCardManager::getVCard(const Jid &AContactJid)
{
	VCardItem &vcardItem = FVCards[AContactJid];
	if (vcardItem.vcard == NULL)
		vcardItem.vcard = new VCard(this,AContactJid);
	vcardItem.locks++;
	return vcardItem.vcard;
}

bool VCardManager::requestVCard(const Jid &AStreamJid, const Jid &AContactJid)
{
	if (FStanzaProcessor && AContactJid.isValid())
	{
		if (FVCardRequestId.key(AContactJid).isEmpty())
		{
			Stanza stanza(STANZA_KIND_IQ);
			stanza.setType(STANZA_TYPE_GET).setTo(AContactJid.full()).setUniqueId();
			stanza.addElement(VCARD_TAGNAME,NS_VCARD_TEMP);
			if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,stanza,VCARD_TIMEOUT))
			{
				LOG_STRM_INFO(AStreamJid,QString("User vCard load request sent to=%1, id=%2").arg(stanza.to(),stanza.id()));
				FVCardRequestId.insert(stanza.id(),AContactJid);
				return true;
			}
			else
			{
				LOG_STRM_WARNING(AStreamJid,QString("Failed to send user vCard load request to=%1").arg(stanza.to()));
			}
			return false;
		}
		return true;
	}
	else if (!AContactJid.isValid())
	{
		REPORT_ERROR("Failed to request user vCard: Invalid params");
	}
	return false;
}

bool VCardManager::publishVCard(const Jid &AStreamJid, IVCard *AVCard)
{
	if (FStanzaProcessor && AVCard->isValid())
	{
		restrictVCardImagesSize(AVCard);

		Stanza stanza(STANZA_KIND_IQ);
		stanza.setType(STANZA_TYPE_SET).setTo(AStreamJid.bare()).setUniqueId();
		QDomElement elem = stanza.element().appendChild(AVCard->vcardElem().cloneNode(true)).toElement();
		removeEmptyChildElements(elem);
		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,stanza,VCARD_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Self vCard publish request sent, id=%1").arg(stanza.id()));
			FVCardPublishId.insert(stanza.id(),stanza);
			return true;
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,"Failed to send self vCard publish request");
		}
	}
	else if (!AVCard->isValid())
	{
		REPORT_ERROR("Failed to publish self vCard: Invalid params");
	}
	return false;
}

QDialog *VCardManager::showVCardDialog(const Jid &AStreamJid, const Jid &AContactJid, QWidget *AParent)
{
	if (FVCardDialogs.contains(AContactJid))
	{
		VCardDialog *dialog = FVCardDialogs.value(AContactJid);
		WidgetManager::showActivateRaiseWindow(dialog);
		return dialog;
	}
	else if (AStreamJid.isValid() && AContactJid.isValid())
	{
		VCardDialog *dialog = new VCardDialog(this,AStreamJid,AContactJid, AParent);
		connect(dialog,SIGNAL(destroyed(QObject *)),SLOT(onVCardDialogDestroyed(QObject *)));
		FVCardDialogs.insert(AContactJid,dialog);
		WidgetManager::showActivateRaiseWindow(dialog);
		return dialog;
	}
	else
	{
		REPORT_ERROR("Failed to show vCard dialog: Invalid params");
		return NULL;
	}
}

void VCardManager::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = false;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_VCARD);
	dfeature.var = NS_VCARD_TEMP;
	dfeature.name = tr("Contact Profile");
	dfeature.description = tr("Supports the requesting of the personal contact information");
	FDiscovery->insertDiscoFeature(dfeature);
}

void VCardManager::unlockVCard(const Jid &AContactJid)
{
	VCardItem &vcardItem = FVCards[AContactJid];
	vcardItem.locks--;
	if (vcardItem.locks <= 0)
	{
		VCard *vcardCopy = vcardItem.vcard;
		FVCards.remove(AContactJid);
		delete vcardCopy;
	}
}

void VCardManager::restrictVCardImagesSize(IVCard *AVCard)
{
	static const struct { const char *value; const char *type; } imageTags[] = {
		{ VVN_LOGO_VALUE,  VVN_LOGO_TYPE  },
		{ VVN_PHOTO_VALUE, VVN_PHOTO_TYPE },
		{ NULL,            NULL           },
	};

	if (Options::node(OPV_COMMON_RESTRICT_VCARD_IMAGES_SIZE).value().toBool())
	{
		for (int i=0; imageTags[i].value!=NULL; i++)
		{
			QByteArray data = QByteArray::fromBase64(AVCard->value(imageTags[i].value).toLatin1());
			if (data.size() > 8*1024)
			{
				QImage image = QImage::fromData(data);
				if (image.width()>MAX_VCARD_IMAGE_SIZE.width() || image.height()>MAX_VCARD_IMAGE_SIZE.height())
				{
					QByteArray scaledData;
					QBuffer buffer(&scaledData);
					buffer.open(QIODevice::WriteOnly);

					image = image.scaled(MAX_VCARD_IMAGE_SIZE,Qt::KeepAspectRatio,Qt::SmoothTransformation);
					if (!image.isNull() && image.save(&buffer,DEFAULT_IMAGE_FORMAT))
					{
						AVCard->setValueForTags(imageTags[i].value,scaledData.toBase64());
						AVCard->setValueForTags(imageTags[i].type,QString("image/%1").arg(DEFAULT_IMAGE_FORMAT));
					}
				}
			}
		}
	}
}

void VCardManager::saveVCardFile(const Jid &AContactJid,const QDomElement &AElem) const
{
	if (AContactJid.isValid())
	{
		QDomDocument doc;
		QDomElement rootElem = doc.appendChild(doc.createElement(VCARD_TAGNAME)).toElement();
		rootElem.setAttribute("jid",AContactJid.full());
		rootElem.setAttribute("dateTime",QDateTime::currentDateTime().toString(Qt::ISODate));

		QFile file(vcardFileName(AContactJid));
		if (!AElem.isNull() && file.open(QIODevice::WriteOnly|QIODevice::Truncate))
		{
			rootElem.appendChild(AElem.cloneNode(true));
			file.write(doc.toByteArray());
			file.close();
		}
		else if (AElem.isNull() && !file.exists() && file.open(QIODevice::WriteOnly|QIODevice::Truncate))
		{
			file.write(doc.toByteArray());
			file.close();
		}
		else if (AElem.isNull() && file.exists() && file.open(QIODevice::ReadWrite))
		{
			char data;
			if (file.getChar(&data))
			{
				file.seek(0);
				file.putChar(data);
			}
			file.close();
		}
		else
		{
			REPORT_ERROR(QString("Failed to save vCard to file: %1").arg(file.errorString()));
		}
		FSearchStrings.remove(AContactJid.bare());
	}
	else
	{
		REPORT_ERROR("Failed to save vCard to file: Invalid params");
	}
}

void VCardManager::removeEmptyChildElements(QDomElement &AElem) const
{
	static const QStringList tagList = QStringList() << "HOME" << "WORK" << "INTERNET" << "X400" << "CELL" << "MODEM";

	QDomElement curChild = AElem.firstChildElement();
	while (!curChild.isNull())
	{
		removeEmptyChildElements(curChild);
		QDomElement nextChild = curChild.nextSiblingElement();
		if (curChild.text().isEmpty() && !tagList.contains(curChild.tagName()))
			curChild.parentNode().removeChild(curChild);
		curChild = nextChild;
	}
}

void VCardManager::insertMessageToolBarAction(IMessageToolBarWidget *AWidget)
{
	if (AWidget && AWidget->messageWindow()->contactJid().isValid())
	{
		Action *action = new Action(AWidget->instance());
		action->setText(tr("Show Profile"));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_VCARD);
		action->setShortcutId(SCT_MESSAGEWINDOWS_SHOWVCARD);
		connect(action,SIGNAL(triggered(bool)),SLOT(onShowVCardDialogByMessageWindowAction(bool)));
		AWidget->toolBarChanger()->insertAction(action,TBG_MWTBW_VCARD_VIEW);
	}
}

QList<Action *> VCardManager::createClipboardActions(const QSet<QString> &AStrings, QObject *AParent) const
{
	QList<Action *> actions;
	foreach(const QString &string, AStrings)
	{
		if (!string.isEmpty())
		{
			Action *action = new Action(AParent);
			action->setText(TextManager::getElidedString(string,Qt::ElideRight,50));
			action->setData(ADR_CLIPBOARD_DATA,string);
			connect(action,SIGNAL(triggered(bool)),SLOT(onCopyToClipboardActionTriggered(bool)));
			actions.append(action);
		}
	}
	return actions;
}

void VCardManager::onCopyToClipboardActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		QApplication::clipboard()->setText(action->data(ADR_CLIPBOARD_DATA).toString());
}

void VCardManager::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	if (FRostersView && AWidget==FRostersView->instance())
	{
		QList<IRosterIndex *> indexes = FRostersView->selectedRosterIndexes();
		if (AId==SCT_ROSTERVIEW_SHOWVCARD && indexes.count()==1)
		{
			IRosterIndex *index = indexes.first();
			if (index!=NULL && VCardRosterKinds.contains(index->kind()))
				showVCardDialog(index->data(RDR_STREAM_JID).toString(),index->data(RDR_PREP_BARE_JID).toString());
		}
	}
}

void VCardManager::onRostersViewIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId==AdvancedDelegateItem::DisplayId && AIndexes.count()==1)
	{
		IRosterIndex *index = AIndexes.first();
		Jid streamJid = index->data(RDR_STREAM_JID).toString();
		Jid contactJid = index->data(RDR_FULL_JID).toString();
		IXmppStream *stream = FXmppStreamManager!=NULL ? FXmppStreamManager->findXmppStream(streamJid) : NULL;

		bool canShowDialog = hasVCard(contactJid);
		canShowDialog = canShowDialog || (stream!=NULL && stream->isOpen() && VCardRosterKinds.contains(index->kind()));
		canShowDialog = canShowDialog || (FDiscovery!=NULL && FDiscovery->discoInfo(streamJid,contactJid.bare()).features.contains(NS_VCARD_TEMP));

		if (canShowDialog)
		{
			Action *action = new Action(AMenu);
			action->setText(streamJid.pBare()==contactJid.pBare() ? tr("Edit Profile") : tr("Show Profile"));
			action->setIcon(RSR_STORAGE_MENUICONS,MNI_VCARD);
			action->setData(ADR_STREAM_JID,streamJid.full());
			action->setData(ADR_CONTACT_JID,contactJid.bare());
			action->setShortcutId(SCT_ROSTERVIEW_SHOWVCARD);
			AMenu->addAction(action,AG_RVCM_VCARD_SHOW,true);
			connect(action,SIGNAL(triggered(bool)),SLOT(onShowVCardDialogByAction(bool)));
		}
	}
}

void VCardManager::onRostersViewIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == AdvancedDelegateItem::DisplayId)
	{
		foreach(IRosterIndex *index, AIndexes)
		{
			Jid contactJid = index->data(RDR_PREP_BARE_JID).toString();
			if (hasVCard(contactJid))
			{
				IVCard *vcard = getVCard(contactJid);

				QSet<QString> commonStrings;
				commonStrings += vcard->value(VVN_FULL_NAME);
				commonStrings += vcard->value(VVN_NICKNAME);
				commonStrings += vcard->value(VVN_ORG_NAME);
				commonStrings += vcard->value(VVN_ORG_UNIT);
				commonStrings += vcard->value(VVN_TITLE);
				commonStrings += vcard->value(VVN_DESCRIPTION);

				static const QStringList emailTagList = QStringList() << "HOME" << "WORK" << "INTERNET" << "X400";
				QSet<QString> emailStrings = vcard->values(VVN_EMAIL,emailTagList).keys().toSet();

				static const QStringList phoneTagList = QStringList() << "HOME" << "WORK" << "CELL" << "MODEM";
				QSet<QString> phoneStrings = vcard->values(VVN_TELEPHONE,phoneTagList).keys().toSet();

				foreach(Action *action, createClipboardActions(commonStrings,AMenu))
					AMenu->addAction(action,AG_RVCBM_VCARD_COMMON,true);

				foreach(Action *action, createClipboardActions(emailStrings,AMenu))
					AMenu->addAction(action,AG_RVCBM_VCARD_EMAIL,true);

				foreach(Action *action, createClipboardActions(phoneStrings,AMenu))
					AMenu->addAction(action,AG_RVCBM_VCARD_PHONE,true);

				vcard->unlock();
			}
		}
	}
}

void VCardManager::onMultiUserContextMenu(IMultiUserChatWindow *AWindow, IMultiUser *AUser, Menu *AMenu)
{
	Q_UNUSED(AWindow);
	Action *action = new Action(AMenu);
	action->setText(tr("Show Profile"));
	action->setIcon(RSR_STORAGE_MENUICONS,MNI_VCARD);
	action->setData(ADR_STREAM_JID,AUser->streamJid().full());
	if (AUser->realJid().isValid())
		action->setData(ADR_CONTACT_JID,AUser->realJid().bare());
	else
		action->setData(ADR_CONTACT_JID,AUser->userJid().full());
	AMenu->addAction(action,AG_MUCM_VCARD_SHOW,true);
	connect(action,SIGNAL(triggered(bool)),SLOT(onShowVCardDialogByAction(bool)));
}

void VCardManager::onShowVCardDialogByAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid contactJid = action->data(ADR_CONTACT_JID).toString();
		showVCardDialog(streamJid,contactJid);
	}
}

void VCardManager::onShowVCardDialogByMessageWindowAction(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		IMessageToolBarWidget *widget = qobject_cast<IMessageToolBarWidget *>(action->parent());
		if (widget)
		{
			bool isMucUser = false;
			Jid contactJid = widget->messageWindow()->contactJid();
			QList<IMultiUserChatWindow *> windows = FMultiChatManager!=NULL ? FMultiChatManager->multiChatWindows() : QList<IMultiUserChatWindow *>();
			for (int i=0; !isMucUser && i<windows.count(); i++)
				isMucUser = windows.at(i)->findPrivateChatWindow(contactJid)!=NULL;
			showVCardDialog(widget->messageWindow()->streamJid(), isMucUser ? contactJid : contactJid.bare());
		}
	}
}

void VCardManager::onVCardDialogDestroyed(QObject *ADialog)
{
	VCardDialog *dialog = static_cast<VCardDialog *>(ADialog);
	FVCardDialogs.remove(FVCardDialogs.key(dialog));
}

void VCardManager::onXmppStreamActiveChanged(IXmppStream *AXmppStream, bool AActive)
{
	if (AActive)
	{
		foreach(VCardDialog *dialog, FVCardDialogs.values())
			if (dialog->streamJid() == AXmppStream->streamJid())
				delete dialog;
	}
}

void VCardManager::onMessageNormalWindowCreated(IMessageNormalWindow *AWindow)
{
	insertMessageToolBarAction(AWindow->toolBarWidget());
}

void VCardManager::onMessageChatWindowCreated(IMessageChatWindow *AWindow)
{
	insertMessageToolBarAction(AWindow->toolBarWidget());
}

void VCardManager::onUpdateTimerTimeout()
{
	bool requestSent = false;
	QMultiMap<Jid,Jid>::iterator it=FUpdateQueue.begin();
	while(!requestSent && it!=FUpdateQueue.end())
	{
		QFileInfo info(vcardFileName(it.value()));
		if (!info.exists() || info.lastModified().daysTo(QDateTime::currentDateTime())>UPDATE_VCARD_DAYS)
		{
			if (requestVCard(it.key(),it.value()))
			{
				requestSent = true;
				FUpdateTimer.start();
			}
		}
		it = FUpdateQueue.erase(it);
	}
}

void VCardManager::onRosterOpened(IRoster *ARoster)
{
	IRosterItem emptyItem;
	foreach(const IRosterItem &item, ARoster->items())
		onRosterItemReceived(ARoster,item,emptyItem);
}

void VCardManager::onRosterClosed(IRoster *ARoster)
{
	FUpdateQueue.remove(ARoster->streamJid());
}

void VCardManager::onRosterItemReceived(IRoster *ARoster, const IRosterItem &AItem, const IRosterItem &ABefore)
{
	if (ARoster->isOpen() && ABefore.isNull())
	{
		if (!FUpdateQueue.contains(ARoster->streamJid(),AItem.itemJid))
		{
			if (!FUpdateTimer.isActive())
				FUpdateTimer.start();
			FUpdateQueue.insertMulti(ARoster->streamJid(),AItem.itemJid);
		}
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_vcard, VCardManager)
#endif

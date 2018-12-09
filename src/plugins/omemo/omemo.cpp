#include <QDebug>

#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/messagewindowwidgets.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/shortcuts.h>

#include <utils/options.h>
#include "omemo.h"

#define TAG_NAME_ROOT	"list"
#define TAG_NAME_ITEM	"device"

Omemo::Omemo(): FPepManager(nullptr),
				FXmppStreamManager(nullptr),
//				FMessageProcessor(nullptr),
				FDiscovery(nullptr),
				FMessageWidgets(nullptr),
				FMessageStyleManager(nullptr),
				FIconStorage(nullptr),
				FOmemoHandlerIn(0),
				FOmemoHandlerOut(0)
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

	IPlugin *plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onStreamClosed(IXmppStream *)));
		}
	}

	plugin = APluginManager->pluginInterface("IPEPManager").value(0);
	if (plugin)
		FPepManager = qobject_cast<IPEPManager *>(plugin->instance());
	else return false;

//	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
//	if (plugin)
//		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
//	else return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
			connect(FMessageWidgets->instance(),SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onNormalWindowCreated(IMessageNormalWindow *)));
		}
	}

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
	if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

	//AInitOrder = 100;   // This one should be initialized AFTER ....!
	return true;
}

bool Omemo::initObjects()
{
//	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_LINKDIALOG, tr("\"Add link\" dialog"), SGO_MESSAGEWINDOWS_LINKDIALOG);
//	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_LINKDIALOG_OK, tr("Ok"), tr("Ctrl+Return", "Ok"), Shortcuts::WidgetShortcut);
//	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_OOB, tr("Out-of-Band data"), SGO_MESSAGEWINDOWS_OOB);
//	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_OOB_INSERTLINK, tr("Add link"), tr("Alt+O", "Add and OOB link"), Shortcuts::WindowShortcut);
//	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_OOB_DELETELINK, tr("Delete link"), tr("Delete", "Delete OOB link"), Shortcuts::WidgetShortcut);
//	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_OOB_EDITLINK, tr("Edit link"), tr("Return", "Edit OOB link"), Shortcuts::WidgetShortcut);
//	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	if (FDiscovery) registerDiscoFeatures();

//	if (FMessageProcessor)
//	{
//		FMessageProcessor->insertMessageWriter(MWO_OOB, this);
//		FMessageProcessor->insertMessageEditor(MEO_OOB, this);
//	}

	FPepManager->insertNodeHandler(QString(NS_PEP_OMEMO), this);

	return true;
}

bool Omemo::initSettings()
{
	return true;
}

bool Omemo::processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.type()!="error")
	{
		Jid contactJid = AStanza.from();
		QDomElement event  = AStanza.firstElement("event", NS_PUBSUB_EVENT);
		QDomElement items  = event.firstChildElement("items");
		if(!items.isNull())
		{
			bool stop=false;
			QDomElement item  = items.firstChildElement("item");
			if(!item.isNull())
			{
				QDomElement list = item.firstChildElement(TAG_NAME_ROOT);
				if(!list.isNull())
				{
					for (QDomElement device = list.firstChildElement(TAG_NAME_ITEM);
						   !device.isNull(); device = device.nextSiblingElement(TAG_NAME_ITEM))
					{
						QString id = device.attribute("id");
						qDebug() << "id=" << id;
					}
//					if(contactJid.bare() == AStreamJid.bare())
//						FIdHash.insert("AStreamJid.bare()", item.attribute("id"));

//					ActivityData activityData;
//					for (QDomElement e = list.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
//						if (e.tagName() == "text")
//							activityData.text = e.text();
//						else
//						{
//							activityData.nameBasic = e.tagName();
//							QDomElement detailed = e.firstChildElement();
//							if (!detailed.isNull())
//								activityData.nameDetailed = detailed.tagName();
//						}

//					if (activityData.isEmpty())
//						stop=true;
//					else
//					{
//						if (activityData!=(FActivityHash.value(contactJid.bare())))
//						{
//							FActivityHash.insert(contactJid.bare(), activityData);
//							updateChatWindows(contactJid, AStreamJid);
//							updateDataHolder(contactJid);
//							displayNotification(AStreamJid, contactJid);
//							if(contactJid.bare() == AStreamJid.bare())
//								FCurrentActivity = activityData;
//						}
//						return true;
//					}
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
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.var = NS_PEP_OMEMO;
//	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK);
	dfeature.name = tr("OMEMO");
	dfeature.description = tr("XEP-0384: OMEMO Encryption");
	FDiscovery->insertDiscoFeature(dfeature);

	dfeature.var.append(NODE_NOTIFY_SUFFIX);
	dfeature.name = tr("OMEMO Notification");
	dfeature.description = tr("Receives notifications about devices, which support OMEMO");
	FDiscovery->insertDiscoFeature(dfeature);
}

bool Omemo::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery==NULL || !FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
							|| FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_JABBER_OOB_X);
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
//	new OmemoLinkList(FIconStorage, AWindow->instance());
//	updateChatWindowActions(AWindow);
//	connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void Omemo::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
	if (window)
		updateChatWindowActions(window);
}

//OmemoLinkList *Omemo::findLinkList(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType)
//{
//	if (AMessageType==Message::Chat)
//		return getLinkList(FMessageWidgets->findChatWindow(AStreamJid, AContactJid));
//	else
//		return getLinkList(FMessageWidgets->findNormalWindow(AStreamJid, AContactJid));
//}

//OmemoLinkList *Omemo::getLinkList(const IMessageChatWindow *AWindow) const
//{
//	return AWindow?qobject_cast<OmemoLinkList *>(AWindow->messageWidgetsBox()->widgetByOrder(MCWW_OOBLINKLISTWIDGET)):NULL;
//}

//OmemoLinkList *Omemo::getLinkList(const IMessageNormalWindow *AWindow) const
//{
//	return AWindow?qobject_cast<OmemoLinkList *>(AWindow->messageWidgetsBox()->widgetByOrder(MNWW_OOBLINKLISTWIDGET)):NULL;
//}

void Omemo::updateChatWindowActions(IMessageChatWindow *AChatWindow)
{
	QList<QAction *> actions = AChatWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_OOB_VIEW);
	QAction *handle=actions.value(0, NULL);
	if (isSupported(AChatWindow->streamJid(), AChatWindow->contactJid()))
	{
		if (!handle)
		{
//			OmemoLinkList *linkList=getLinkList(AChatWindow);
//			if (linkList->topLevelItemCount())
//				linkList->show();
//			Action *action = new Action(linkList);
//			action->setText(tr("Add link"));
//			action->setIcon(RSR_STORAGE_MENUICONS, MNI_LINK_ADD);
//			action->setShortcutId(SCT_MESSAGEWINDOWS_OOB_INSERTLINK);
//			connect(action, SIGNAL(triggered(bool)), SLOT(onInsertLink(bool)));
//			AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_OOB_VIEW);
		}
	}
	else
	{
		if (handle)
		{
//			AChatWindow->toolBarWidget()->toolBarChanger()->removeItem(handle);
//			handle->deleteLater();
//			getLinkList(AChatWindow)->hide();
		}
	}
}

//void Omemo::onInsertLink(bool)
//{
//	Action *action = qobject_cast<Action *>(sender());
//	if (action)
//	{
//		NewLink *newLink = new NewLink(tr("Add a new link"),
//									   FIconStorage->getIcon(MNI_LINK), QString(), QString(),
//									   action->parentWidget()->parentWidget());
//		if(newLink->exec()== QDialog::Accepted)
//		{
//			OmemoLinkList *list=qobject_cast<OmemoLinkList *>(action->parent());
//			if (list)
//				list->addLink(newLink->getUrl(), newLink->getDescription());
//		}
//		newLink->deleteLater();
//	}
//}

//void Omemo::appendLinks(Message &AMessage, OmemoLinkList *ALinkList)
//{
//	QDomDocument doc;
//	int count=ALinkList->topLevelItemCount();
//	for (int i=0; i<count; i++)
//	{
//		QTreeWidgetItem *item=ALinkList->topLevelItem(i);
//		QDomElement oob=doc.createElementNS(NS_JABBER_OOB_X, "x");
//		QDomElement url=doc.createElementNS(NS_JABBER_OOB_X, "url");
//		url.appendChild(doc.createTextNode(item->data(1, OmemoLinkList::IDR_URL).toUrl().toEncoded()));
//		QDomElement desc=doc.createElementNS(NS_JABBER_OOB_X, "desc");
//		desc.appendChild(doc.createTextNode(item->data(1, OmemoLinkList::IDR_DESCRIPTION).toString()));
//		oob.appendChild(url);
//		oob.appendChild(desc);
//		AMessage.stanza().element().appendChild(oob);
//	}
//}

void Omemo::onStreamOpened(IXmppStream *AXmppStream)
{
	FStreamOmemo.insert(AXmppStream->streamJid(), NULL);
}

void Omemo::onStreamClosed(IXmppStream *AXmppStream)
{
	FStreamOmemo.remove(AXmppStream->streamJid());
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

//bool Omemo::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
//{
//	Q_UNUSED(AOrder)

//	if (ADirection==IMessageProcessor::DirectionOut)
//	{
//		OmemoLinkList *list=findLinkList(AStreamJid, AMessage.to(), AMessage.type());
//		if (list)
//		{
//			if (!list->isHidden())
//			{
//				appendLinks(AMessage, list);
//				list->hide();
//			}
//			list->clear();
//		}
//	}
//	return false;
//}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_omemo, Omemo)
#endif

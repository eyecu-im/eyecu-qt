#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/messagechatwindowwidgets.h>
#include <definitions/messagenormalwindowwidgets.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/shortcuts.h>

#include <utils/options.h>
#include "oob.h"

Oob::Oob(): FXmppStreamManager(NULL),
            FMessageProcessor(NULL),
            FDiscovery(NULL),
            FMessageWidgets(NULL),
            FMessageStyleManager(NULL),
            FIconStorage(NULL),
            FOobHandlerIn(0),
            FOobHandlerOut(0)
{}

Oob::~Oob()
{}

void Oob::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Out of Band Data");
    APluginInfo->description = tr("Implements XEP-0066: Out of Band Data");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
}

bool Oob::initConnections(IPluginManager *APluginManager, int &AInitOrder)
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
    plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
    else  return false;


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

bool Oob::initObjects()
{
    Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_LINKDIALOG, tr("\"Add link\" dialog"), SGO_MESSAGEWINDOWS_LINKDIALOG);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_LINKDIALOG_OK, tr("Ok"), tr("Ctrl+Return", "Ok"), Shortcuts::WidgetShortcut);
	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_OOB, tr("Out-of-Band data"), SGO_MESSAGEWINDOWS_OOB);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_OOB_INSERTLINK, tr("Add link"), tr("Alt+O", "Add and OOB link"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_OOB_DELETELINK, tr("Delete link"), tr("Delete", "Delete OOB link"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_OOB_EDITLINK, tr("Edit link"), tr("Return", "Edit OOB link"), Shortcuts::WidgetShortcut);
    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
    if (FDiscovery) registerDiscoFeatures();

    if (FMessageProcessor)
    {
        FMessageProcessor->insertMessageWriter(MWO_OOB, this);
        FMessageProcessor->insertMessageEditor(MEO_OOB, this);
    }

    return true;
}

bool Oob::initSettings()
{
    return true;
}

void Oob::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.active = true;
    dfeature.var = NS_JABBER_OOB_X;
    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK);
    dfeature.name = tr("Out-of-Band Data");
    dfeature.description = tr("XEP-0066: Out of Band Data");
    FDiscovery->insertDiscoFeature(dfeature);
}

bool Oob::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FDiscovery==NULL || !FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
                            || FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_JABBER_OOB_X);
}

void Oob::onNormalWindowCreated(IMessageNormalWindow *AWindow)
{
    if(AWindow->mode()==IMessageNormalWindow::WriteMode && isSupported(AWindow->streamJid(), AWindow->contactJid()))
    {
        Action *action = new Action(new OobLinkList(FIconStorage, AWindow->instance()));
        action->setText(tr("Add link"));
        action->setIcon(RSR_STORAGE_MENUICONS, MNI_LINK_ADD);
        action->setShortcutId(SCT_MESSAGEWINDOWS_OOB_INSERTLINK);
        connect(action, SIGNAL(triggered(bool)), SLOT(onInsertLink(bool)));
        AWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_OOB_VIEW);
    }
}

void Oob::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    new OobLinkList(FIconStorage, AWindow->instance());
    updateChatWindowActions(AWindow);
    connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void Oob::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

    IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
    IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
    if (window)
        updateChatWindowActions(window);
}

OobLinkList *Oob::findLinkList(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType)
{
    if (AMessageType==Message::Chat)
        return getLinkList(FMessageWidgets->findChatWindow(AStreamJid, AContactJid));
    else
        return getLinkList(FMessageWidgets->findNormalWindow(AStreamJid, AContactJid));
}

OobLinkList *Oob::getLinkList(const IMessageChatWindow *AWindow) const
{
    return AWindow?qobject_cast<OobLinkList *>(AWindow->messageWidgetsBox()->widgetByOrder(MCWW_OOBLINKLISTWIDGET)):NULL;
}

OobLinkList *Oob::getLinkList(const IMessageNormalWindow *AWindow) const
{
    return AWindow?qobject_cast<OobLinkList *>(AWindow->messageWidgetsBox()->widgetByOrder(MNWW_OOBLINKLISTWIDGET)):NULL;
}

void Oob::updateChatWindowActions(IMessageChatWindow *AChatWindow)
{
    QList<QAction *> actions = AChatWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_OOB_VIEW);
    QAction *handle=actions.value(0, NULL);
    if (isSupported(AChatWindow->streamJid(), AChatWindow->contactJid()))
    {
        if (!handle)
        {
            OobLinkList *linkList=getLinkList(AChatWindow);
            if (linkList->topLevelItemCount())
                linkList->show();
            Action *action = new Action(linkList);
            action->setText(tr("Add link"));
            action->setIcon(RSR_STORAGE_MENUICONS, MNI_LINK_ADD);
            action->setShortcutId(SCT_MESSAGEWINDOWS_OOB_INSERTLINK);
            connect(action, SIGNAL(triggered(bool)), SLOT(onInsertLink(bool)));
            AChatWindow->toolBarWidget()->toolBarChanger()->insertAction(action, TBG_MWTBW_OOB_VIEW);
        }
    }
    else
    {
        if (handle)
        {
            AChatWindow->toolBarWidget()->toolBarChanger()->removeItem(handle);
            handle->deleteLater();
            getLinkList(AChatWindow)->hide();
        }
    }
}

void Oob::onInsertLink(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    if (action)
    {
        NewLink *newLink = new NewLink(tr("Add a new link"),
                                       FIconStorage->getIcon(MNI_LINK), QString(), QString(),
                                       action->parentWidget()->parentWidget());
        if(newLink->exec()== QDialog::Accepted)
        {
            OobLinkList *list=qobject_cast<OobLinkList *>(action->parent());
            if (list)
                list->addLink(newLink->getUrl(), newLink->getDescription());
        }
        newLink->deleteLater();
    }
}

void Oob::appendLinks(Message &AMessage, OobLinkList *ALinkList)
{
    QDomDocument doc;
    int count=ALinkList->topLevelItemCount();
    for (int i=0; i<count; i++)
    {
        QTreeWidgetItem *item=ALinkList->topLevelItem(i);
        QDomElement oob=doc.createElementNS(NS_JABBER_OOB_X, "x");
        QDomElement url=doc.createElementNS(NS_JABBER_OOB_X, "url");
        url.appendChild(doc.createTextNode(item->data(1, OobLinkList::IDR_URL).toUrl().toEncoded()));
        QDomElement desc=doc.createElementNS(NS_JABBER_OOB_X, "desc");
        desc.appendChild(doc.createTextNode(item->data(1, OobLinkList::IDR_DESCRIPTION).toString()));
        oob.appendChild(url);
        oob.appendChild(desc);
        AMessage.stanza().element().appendChild(oob);
    }
}

void Oob::onStreamOpened(IXmppStream *AXmppStream)
{
    FStreamOob.insert(AXmppStream->streamJid(), NULL);
}

void Oob::onStreamClosed(IXmppStream *AXmppStream)
{
    FStreamOob.remove(AXmppStream->streamJid());
}

bool Oob::parseOOB(Stanza &AStanza, QTextDocument *ADocument)
{
    bool noRuler=true;
    QTextCursor cursor(ADocument);
    QString         icon;
    QTextCharFormat origFormat;
    QTextCharFormat linkFormat;

    for(QDomElement e=AStanza.firstElement("x",NS_JABBER_OOB_X); !e.isNull(); e=e.nextSiblingElement("x"))
        if(!e.firstChildElement("url").text().isEmpty())
        {
            if (noRuler)    // No ruler inserted so far
            {
                QTextBlockFormat origBlockFormat = cursor.blockFormat();    // Original block format
                icon = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\" /> ")
                               .arg(FIconStorage->fileFullName(MNI_LINK))
                               .arg(tr("Link"));

                cursor.movePosition(QTextCursor::End);
                origFormat = cursor.charFormat();
                linkFormat = origFormat;
                linkFormat.setAnchor(true);

                cursor.insertHtml("<hr>");
                cursor.insertBlock();
                cursor.setBlockFormat(origBlockFormat);                     // Restore original format
                noRuler=false;  // Flag ruler inserted
            }
            else
            {
                cursor.setCharFormat(origFormat);
                cursor.insertHtml("<br>");
            }

            cursor.insertHtml(icon);
            QUrl url = QUrl::fromEncoded(e.firstChildElement("url").text().toLatin1());
            linkFormat.setAnchorHref(url.toEncoded());
            linkFormat.setToolTip(url.toString());
            cursor.setCharFormat(linkFormat);

            QString desc = !e.firstChildElement("desc").text().isEmpty()?e.firstChildElement("desc").text():"";
            cursor.insertText(desc.isEmpty()?url.toString():desc);
        }
    return noRuler;
}


void Oob::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(ALang)

    if (AOrder == MWO_OOB)
        parseOOB(AMessage.stanza(), ADocument);
}

void Oob::writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(AMessage)
	Q_UNUSED(ADocument)
	Q_UNUSED(ALang)
}  // Nothing to do right now

bool Oob::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

    if (ADirection==IMessageProcessor::DirectionOut)
    {
        OobLinkList *list=findLinkList(AStreamJid, AMessage.to(), AMessage.type());
        if (list)
        {
            if (!list->isHidden())
            {
                appendLinks(AMessage, list);
                list->hide();
            }
            list->clear();
        }
    }
    return false;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_oob, Oob)
#endif

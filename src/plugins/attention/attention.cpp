#include <definitions/archivehandlerorders.h>
#include <definitions/messagehandlerorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messagedataroles.h>
#include <definitions/notificationtypes.h>
#include <definitions/notificationdataroles.h>
#include <definitions/notificationtypeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/rosternotifyorders.h>
#include <definitions/soundfiles.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/tabpagenotifypriorities.h>
#include <definitions/toolbargroups.h>
#include <utils/options.h>
#include <utils/shortcuts.h>
#include <utils/textmanager.h>
#include <utils/widgetmanager.h>
#include <utils/qt4qt5compat.h>

#include "attention.h"

#define ADR_STREAM_JID          Action::DR_StreamJid
#define ADR_CONTACT_JID         Action::DR_Parametr4

Attention::Attention():
        FMessageProcessor(NULL),
        FMessageArchiver(NULL),
        FOptionsManager(NULL),
        FDiscovery(NULL),
        FMessageWidgets(NULL),
        FMessageStyleManager(NULL),
        FIconStorage(NULL),
        FStatusIcons(NULL),
        FMainWindowPlugin(NULL),        
        FMainWindow(NULL),
        FNotifications(NULL)
{}

Attention::~Attention()
{}

void Attention::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Attention");
    APluginInfo->description = tr("Implements XEP-0224: Attention");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
    APluginInfo->dependences.append(MESSAGEWIDGETS_UUID);
    APluginInfo->dependences.append(MESSAGESTYLES_UUID);
}

bool Attention::initConnections(IPluginManager *APluginManager, int & /*AInitOrder*/)
{
    IPlugin *plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
    if (!FMessageProcessor)
        return false;

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
    }
    else
        return false;

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
    if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());
    if (!FMessageStyleManager)
        return false;

    plugin = APluginManager->pluginInterface("IMessageArchiver").value(0);
    if (plugin)
        FMessageArchiver = qobject_cast<IMessageArchiver *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IAvatars").value(0,NULL);
    if (plugin)
        FAvatars = qobject_cast<IAvatars *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IStatusIcons").value(0,NULL);
    if (plugin)
        FStatusIcons = qobject_cast<IStatusIcons *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
    if (plugin)
        FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());

    plugin = APluginManager->pluginInterface("INotifications").value(0,NULL);
    if (plugin)
    {
        FNotifications = qobject_cast<INotifications *>(plugin->instance());
        if (FNotifications)
        {
            connect(FNotifications->instance(),SIGNAL(notificationAppended(int,INotification)), SLOT(onNotificationAppended(int,INotification)));
            connect(FNotifications->instance(),SIGNAL(notificationRemoved(int)), SLOT(onNotificationRemoved(int)));
        }
    }

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

    // AInitOrder = 200;   // This one should be initialized AFTER !

    return true;
}

bool Attention::initObjects()
{    
    if (FMessageProcessor)
    {
        FMessageProcessor->insertMessageHandler(MHO_ATTENTION, this);
        FMessageProcessor->insertMessageWriter(MWO_ATTENTION, this);
    }

    if (FMainWindowPlugin)
        FMainWindow=FMainWindowPlugin->mainWindow()->instance();

    if (FDiscovery)
        registerDiscoFeatures();

    if (FMessageArchiver)
        FMessageArchiver->insertArchiveHandler(AHO_DEFAULT, this);

    if (FNotifications)
    {
        INotificationType notifyType;
        notifyType.order = NTO_ATTENTION_NOTIFY;
		notifyType.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_ATTENTION);
        notifyType.title = tr("When contact attempts to attract user's attention");
        notifyType.kindMask = INotification::RosterNotify|INotification::TrayNotify|INotification::TrayAction|
                              INotification::PopupWindow|INotification::SoundPlay|INotification::AlertWidget|
                              INotification::TabPageNotify|INotification::ShowMinimized|INotification::AutoActivate;
        notifyType.kindDefs = notifyType.kindMask & ~(INotification::AutoActivate|INotification::PopupWindow);
        FNotifications->registerNotificationType(NNT_ATTENTION, notifyType);
    }

    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_CHAT_ATTENTION, tr("Attention"), tr("Shift+Return","Attention"), Shortcuts::WindowShortcut);

    return true;
}

bool Attention::initSettings()
{
    Options::setDefaultValue(OPV_ATTENTION_NOTIFICATIONPOPUP, true);
    Options::setDefaultValue(OPV_ATTENTION_AYWAYSPLAYSOUND, true);
    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Attention::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_NOTIFICATIONS )
	{
		widgets.insertMulti(OHO_ATTENTION, FOptionsManager->newOptionsDialogHeader(tr("Attention"), AParent));
		widgets.insertMulti(OWO_ATTENTION_NOTIFICATIONPOPUP, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ATTENTION_NOTIFICATIONPOPUP), tr("Notification pop-up"), AParent));
		widgets.insertMulti(OWO_ATTENTION_AYWAYSPLAYSOUND, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ATTENTION_AYWAYSPLAYSOUND), tr("Always play sound"), AParent));
	}
    return widgets;
}

void Attention::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.var = NS_ATTENTION;
    dfeature.active = true;
	dfeature.icon = FIconStorage->getIcon(MNI_ATTENTION);
    dfeature.name = tr("Attention");
    dfeature.description = tr("Implements XEP-0224: Allows to attract user's attention");
    FDiscovery->insertDiscoFeature(dfeature);
}

bool Attention::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FDiscovery==NULL || !FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
                            || FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_ATTENTION);
}

//IMeassageHandler
bool Attention::messageCheck(int AOrder, const Message &AMessage, int ADirection)
{
    Q_UNUSED(AOrder); Q_UNUSED(ADirection);
    return AMessage.type()==Message::Headline && !AMessage.stanza().firstElement("attention", NS_ATTENTION).isNull();
}

bool Attention::messageDisplay(const Message &AMessage, int ADirection)
{
    IMessageChatWindow *window = NULL;
    if (ADirection == IMessageProcessor::DirectionIn)
        window = getWindow(AMessage.to(),AMessage.from());
    else
        window = getWindow(AMessage.from(),AMessage.to());
    if (window)
        showStyledMessage(window, AMessage);
    return window!=NULL;
}

INotification Attention::messageNotify(INotifications *ANotifications, const Message &AMessage, int ADirection)
{
    INotification notify;
    if (ADirection == IMessageProcessor::DirectionIn)
    {
        int kinds = ANotifications->enabledTypeNotificationKinds(NNT_ATTENTION);
        if (Options::node(OPV_ATTENTION_NOTIFICATIONPOPUP).value().toBool())
            kinds |= AttentionPopup;
        if (kinds > 0)
        {
            IMessageChatWindow *window = getWindow(AMessage.to(), AMessage.from());
            if (window)
            {
                if (!window->isActiveTabPage() || (kinds&INotification::SoundPlay && Options::node(OPV_ATTENTION_AYWAYSPLAYSOUND).value().toBool()))
                {
                    notify.typeId = NNT_ATTENTION;
                    notify.data.insert(NDR_SOUND_FILE, SDF_ATTENTION_ALARM);

                    if (window->isActiveTabPage())
                        notify.kinds  = INotification::SoundPlay;
                    else
                    {
						QIcon icon   = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_ATTENTION);
                        QString name = ANotifications->contactName(AMessage.to(),AMessage.from());

                        notify.kinds  = kinds;

                        notify.data.insert(NDR_ICON, icon);
                        notify.data.insert(NDR_TOOLTIP, tr("Attention from %1").arg(name));
                        notify.data.insert(NDR_STREAM_JID, AMessage.to());
                        notify.data.insert(NDR_CONTACT_JID, AMessage.from());

                        // Popup data - used by AttentionDialog
                        if (FAvatars)
                        {
                            QString avatarFileName=FAvatars->avatarFileName(FAvatars->avatarHash(AMessage.from()));
                            notify.data.insert(NDR_ATTENTION_DIALOG_AVATAR_FILE_NAME, avatarFileName);
                        }
                        notify.data.insert(NDR_POPUP_CAPTION, tr("ATTENTION!!!"));
                        notify.data.insert(NDR_POPUP_TITLE, name);
						notify.data.insert(NDR_POPUP_HTML, HTML_ESCAPE(AMessage.body()));
                        notify.data.insert(NDR_ROSTER_ORDER, RNO_ATTENTION);
                        notify.data.insert(NDR_ROSTER_FLAGS, IRostersNotify::Blink|
                                                             IRostersNotify::AllwaysVisible|
                                                             IRostersNotify::HookClicks);
                        notify.data.insert(NDR_ROSTER_CREATE_INDEX, true);

                        notify.data.insert(NDR_ALERT_WIDGET, (qint64)window->instance());
                        notify.data.insert(NDR_TABPAGE_WIDGET, (qint64)window->instance());
                        notify.data.insert(NDR_TABPAGE_PRIORITY, TPNP_ATTENTION);
                        notify.data.insert(NDR_TABPAGE_ICONBLINK, true);
                        notify.data.insert(NDR_SHOWMINIMIZED_WIDGET, (qint64)window->instance());

                        FNotifiedMessages.insertMulti(window, AMessage.data(MDR_MESSAGE_ID).toInt());
                        updateWindow(window);
                    }
                }
            }
        }
    }
    return notify;
}

bool Attention::messageShowWindow(int AMessageId)
{
    IMessageChatWindow *window = FNotifiedMessages.key(AMessageId);
    if (window)
    {        
        window->showTabPage();
        return true;
    }
    return false;
}

bool Attention::messageShowWindow(int AOrder, const Jid &AStreamJid, const Jid &AContactJid, Message::MessageType AType, int AShowMode)
{
    Q_UNUSED(AOrder);
    if (AType == Message::Headline)
    {
        IMessageChatWindow *window = getWindow(AStreamJid, AContactJid);
        if (window)
        {
            if (AShowMode == IMessageHandler::SM_ASSIGN)
                window->assignTabPage();
            else if (AShowMode == IMessageHandler::SM_SHOW)
                window->showTabPage();
            else if (AShowMode == IMessageHandler::SM_MINIMIZED)
                window->showMinimizedTabPage();
            return true;
        }
    }
    return false;
}

//IArchiveHandler
bool Attention::archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
    Q_UNUSED(AOrder); Q_UNUSED(ADirectionIn); Q_UNUSED(AStreamJid);
    if (AMessage.type()==Message::Headline && !AMessage.stanza().firstElement("attention", NS_ATTENTION).isNull())
        return true;
    else
        return false;
}

void Attention::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

    if (AMessage.type()==Message::Headline)
        if (!AMessage.stanza().firstElement("attention", NS_ATTENTION).isNull())
        {
            QTextCursor cursor(ADocument);
            QString html = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\" />")
							.arg(FIconStorage->fileFullName(MNI_ATTENTION))
                            .arg(tr("Attention"));

            cursor.insertHtml(html);            
            cursor.insertText(" ");
            cursor.movePosition(QTextCursor::NextBlock);
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat charFormat;
            charFormat.setFontWeight(QFont::Bold);
            cursor.mergeCharFormat(charFormat);
        }
}

IMessageChatWindow *Attention::getWindow(const Jid &AStreamJid, const Jid &AContactJid)
{
    IMessageChatWindow *window = NULL;
    if (AStreamJid.isValid() && AContactJid.isValid())
        if (FMessageWidgets)
        {
            window = FMessageWidgets->findChatWindow(AStreamJid, AContactJid);
            if(!window)
            {
                FMessageProcessor->createMessageWindow(AStreamJid, AContactJid, Message::Chat, IMessageHandler::SM_ASSIGN);
                window = FMessageWidgets->findChatWindow(AStreamJid,AContactJid);
            }
            if (window)
                connect(window->instance(), SIGNAL(tabPageActivated()), SLOT(onWindowActivated()));
        }
    return window;
}

void Attention::updateWindow(IMessageChatWindow *AWindow)
{
    QIcon icon;
    if (AWindow->instance()->isWindow() && FNotifiedMessages.contains(AWindow))
		icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_ATTENTION);
    else if (FStatusIcons)
        icon = FStatusIcons->iconByJid(AWindow->streamJid(),AWindow->contactJid());
    QString contactName = AWindow->infoWidget()->fieldValue(IMessageInfoWidget::Name).toString();
    AWindow->updateWindow(icon, contactName, tr("%1 - Chat").arg(contactName), QString::null);
}

void Attention::removeNotifiedMessages(IMessageChatWindow *AWindow)
{
    if (FNotifiedMessages.contains(AWindow))
    {
        QList<int> messageIds=FNotifiedMessages.values(AWindow);
        for (QList<int>::const_iterator it=messageIds.constBegin(); it!=messageIds.constEnd(); it++ )
            FMessageProcessor->removeMessageNotify(*it);
        FNotifiedMessages.remove(AWindow);
        updateWindow(AWindow);
    }
}

void Attention::setMessageStyle(IMessageChatWindow *AWindow)
{
    IMessageStyleOptions soptions = FMessageStyleManager->styleOptions(Message::Headline);
    if (AWindow->viewWidget()->messageStyle()==NULL || !AWindow->viewWidget()->messageStyle()->changeOptions(AWindow->viewWidget()->styleWidget(),soptions,true))
    {
        IMessageStyle *style = FMessageStyleManager->styleForOptions(soptions);
        AWindow->viewWidget()->setMessageStyle(style,soptions);
        FWindowStatus[AWindow].lastDateSeparator = QDate();
    }
}

void Attention::fillContentOptions(IMessageChatWindow *AWindow, IMessageStyleContentOptions &AOptions) const
{
	if (AOptions.direction == IMessageStyleContentOptions::DirectionIn)
    {
        AOptions.senderId = AWindow->contactJid().full();
		AOptions.senderName = HTML_ESCAPE(FMessageStyleManager->contactName(AWindow->streamJid(),AWindow->contactJid()));
		AOptions.senderAvatar = FMessageStyleManager->contactAvatar(AWindow->contactJid());
        AOptions.senderIcon = FMessageStyleManager->contactIcon(AWindow->streamJid(),AWindow->contactJid());
        AOptions.senderColor = "blue";
    }
    else
    {
        AOptions.senderId = AWindow->streamJid().full();
        if (AWindow->streamJid() && AWindow->contactJid())
			AOptions.senderName = HTML_ESCAPE(!AWindow->streamJid().resource().isEmpty() ? AWindow->streamJid().resource() : AWindow->streamJid().node());
		else
			AOptions.senderName = HTML_ESCAPE(FMessageStyleManager->contactName(AWindow->streamJid()));
        AOptions.senderAvatar = FMessageStyleManager->contactAvatar(AWindow->streamJid());
        AOptions.senderIcon = FMessageStyleManager->contactIcon(AWindow->streamJid());
        AOptions.senderColor = "red";
    }
}

void Attention::showDateSeparator(IMessageChatWindow *AWindow, const QDateTime &ADateTime)
{
    if (Options::node(OPV_MESSAGES_SHOWDATESEPARATORS).value().toBool())
    {
        QDate sepDate = ADateTime.date();
        WindowStatus &wstatus = FWindowStatus[AWindow];
        if (FMessageStyleManager && sepDate.isValid() && wstatus.lastDateSeparator!=sepDate)
        {
			IMessageStyleContentOptions options;
			options.kind = IMessageStyleContentOptions::KindStatus;
            if (wstatus.createTime > ADateTime)
				options.type |= IMessageStyleContentOptions::TypeHistory;
			options.status = IMessageStyleContentOptions::StatusDateSeparator;
			options.direction = IMessageStyleContentOptions::DirectionIn;
            options.time.setDate(sepDate);
            options.time.setTime(QTime(0,0));
            options.timeFormat = " ";
            wstatus.lastDateSeparator = sepDate;
            AWindow->viewWidget()->appendText(FMessageStyleManager->dateSeparator(sepDate),options);
        }
    }
}

void Attention::showStyledMessage(IMessageChatWindow *AWindow, const Message &AMessage)
{
	IMessageStyleContentOptions options;
	options.kind = IMessageStyleContentOptions::KindMessage;
	options.type = IMessageStyleContentOptions::TypeEvent;
    options.time = AMessage.dateTime();

    if (Options::node(OPV_MESSAGES_SHOWDATESEPARATORS).value().toBool())
        options.timeFormat = FMessageStyleManager->timeFormat(options.time, options.time);
    else
        options.timeFormat = FMessageStyleManager->timeFormat(options.time);

    if (AWindow->streamJid() && AWindow->contactJid() ? AWindow->contactJid()!=AMessage.to() : !(AWindow->contactJid() && AMessage.to()))
		options.direction = IMessageStyleContentOptions::DirectionIn;
    else
		options.direction = IMessageStyleContentOptions::DirectionOut;

    fillContentOptions(AWindow,options);
    showDateSeparator(AWindow,options.time);
    AWindow->viewWidget()->appendMessage(AMessage, options);
}

// SLOTS
void Attention::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    if (isSupported(AWindow->streamJid(), AWindow->contactJid()) && AWindow->toolBarWidget() && AWindow->viewWidget())
    {
        Jid contactJid = AWindow->contactJid();
        Jid streamJid = AWindow->streamJid();
        Action *action = new Action(AWindow->toolBarWidget()->instance());
        action->setText(tr("Attention"));
		action->setIcon(RSR_STORAGE_MENUICONS, MNI_ATTENTION);
        action->setData(ADR_CONTACT_JID, contactJid.full());
        action->setData(ADR_STREAM_JID, streamJid.full());
        action->setShortcutId(SCT_MESSAGEWINDOWS_CHAT_ATTENTION);
        connect(action,SIGNAL(triggered(bool)),SLOT(onSetAttentionByAction(bool)));
        AWindow->editWidget()->editToolBarChanger()->insertAction(action,TBG_MWTBW_ATTENTION_VIEW);
    }
}

void  Attention::onSetAttentionByAction(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    if (action)
    {
        Jid contactJid = action->data(ADR_CONTACT_JID).toString();
        Jid streamJid = action->data(ADR_STREAM_JID).toString();
        Stanza stanza("message");
        stanza.setTo(contactJid.full()).setFrom(streamJid.full());
        stanza.setType("headline");
        stanza.addElement("attention", NS_ATTENTION);

        IMessageChatWindow *window = FMessageWidgets->findChatWindow(streamJid,contactJid);
        if (FMessageProcessor && window)
        {
			IMessageStyleContentOptions options;
            options.time = QDateTime::currentDateTime();
            options.timeFormat = FMessageStyleManager->timeFormat(options.time);
			options.type = IMessageStyleContentOptions::TypeNotification;
			options.kind = IMessageStyleContentOptions::KindMessage;
			options.direction = IMessageStyleContentOptions::DirectionOut;
            options.senderId = window->streamJid().full();
			options.senderName   = HTML_ESCAPE(FMessageStyleManager->contactName(window->streamJid(), window->contactJid()));
			options.senderAvatar = FMessageStyleManager->contactAvatar(window->streamJid());

            Message message;
            message.setStanza(stanza);
            FMessageProcessor->textToMessage(message,window->editWidget()->document());
            QString textsms = window->editWidget()->textEdit()->toPlainText();
            if (FMessageProcessor->sendMessage(window->streamJid(),message,IMessageProcessor::DirectionOut))
                if (!message.body().isEmpty())
                    window->editWidget()->textEdit()->clear();
        }
    }
}

void Attention::onWindowActivated()
{
    IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
    if (window)
        removeNotifiedMessages(window);
}

void Attention::onNotificationAppended(int ANotifyId, const INotification &ANotification)
{
    if (ANotification.typeId==NNT_ATTENTION)
    {
        if (ANotification.kinds & AttentionPopup)
        {
            AttentionDialog *dialog=new AttentionDialog(ANotifyId, ANotification, FNotifications);
			dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
#if Q_WS_X11
			dialog->setWindowFlags(Qt::X11BypassWindowManagerHint);
#endif
            FAttentionDialogs.insert(ANotifyId, dialog);
			WidgetManager::showActivateRaiseWindow(dialog);
        }
    }
}

void Attention::onNotificationRemoved(int ANotifyId)
{
    if (FAttentionDialogs.contains(ANotifyId))
        FAttentionDialogs.take(ANotifyId)->accept();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_attention, Attention)
#endif

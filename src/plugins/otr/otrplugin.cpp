#include "otrplugin.h"

#include <definitions/toolbargroups.h>
#include <definitions/archivehandlerorders.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <utils/logger.h>

#include "psiotrclosure.h"

#include <QtCore/QPair>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>

namespace psiotr
{

#define SHO_OTR             500
#define SHC_PRESENCE        "/presence"
#define SHC_MESSAGE         "/message"

OtrPlugin::OtrPlugin() :
    m_otrConnection(NULL),
    m_onlineUsers(),
    FOptionsManager(NULL),
    FAccountManager(NULL),
    FPresenceManager(NULL),
    FMessageProcessor(NULL),
    m_inboundCatcher(NULL),
    m_outboundCatcher(NULL)
{
}

OtrPlugin::~OtrPlugin()
{
    delete m_otrConnection;
}

void OtrPlugin::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Off-the-Record Messaging Plugin");
    APluginInfo->description = tr("Off-the-Record (OTR) Messaging allows you to have private conversations over instant messaging");
    APluginInfo->version = "1.0.3";
    APluginInfo->author = "John Smith";
    APluginInfo->homePage = "https://github.com/xnamed";    
    APluginInfo->dependences.append(MESSAGEPROCESSOR_UUID);
    APluginInfo->dependences.append(ACCOUNTMANAGER_UUID);
    APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
    APluginInfo->dependences.append(XMPPSTREAMS_UUID);
}

bool OtrPlugin::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    m_homePath = APluginManager->homePath();

    IPlugin *plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
    if (plugin)
    {
        FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
    }

    plugin = APluginManager->pluginInterface("IPresenceManager").value(0);
    if (plugin)
    {
        FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
        if (FPresenceManager)
        {
            connect(FPresenceManager->instance(),SIGNAL(presenceOpened(IPresence *)),SLOT(onPresenceOpened(IPresence *)));
        }
    }

    plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
    if (plugin)
    {
        IXmppStreamManager *FXmppStreams = qobject_cast<IXmppStreamManager *>(plugin->instance());
        if (FXmppStreams)
        {
            connect(FXmppStreams->instance(), SIGNAL(opened(IXmppStream *)), SLOT(onStreamOpened(IXmppStream *)));
            connect(FXmppStreams->instance(), SIGNAL(closed(IXmppStream *)), SLOT(onStreamClosed(IXmppStream *)));
        }
    }

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
    {
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
        if (FOptionsManager)
        {
            connect(FOptionsManager->instance(),SIGNAL(profileOpened(const QString &)),SLOT(onProfileOpened(const QString &)));
        }
    }

    plugin = APluginManager->pluginInterface("IMessageArchiver").value(0);
    if (plugin)
        FMessageArchiver = qobject_cast<IMessageArchiver *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
    FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        if (FMessageWidgets)
        {
            connect(FMessageWidgets->instance(), SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)), SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
            connect(FMessageWidgets->instance(), SIGNAL(messageWindowCreated(IMessageWindow *)), SLOT(onMessageWindowCreated(IMessageWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(messageWindowDestroyed(IMessageWindow *)), SLOT(onMessageWindowDestroyed(IMessageWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)), SLOT(onChatWindowCreated(IMessageChatWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowDestroyed(IMessageChatWindow *)), SLOT(onChatWindowDestroyed(IMessageChatWindow *)));
        }
    }

    return (FStanzaProcessor != NULL);

    return true;
}

bool OtrPlugin::initObjects()
{
    /*if (FMessageArchiver)
    {
    //    qDebug("archive processor registered");
        FMessageArchiver->insertArchiveHandler(AHO_DEFAULT,this);
    }*/

    return true;
}

QObject * OtrPlugin::instance()
{
    return this;
}

bool OtrPlugin::initSettings()
{
    Options::setDefaultValue(OPTION_POLICY, OTR_POLICY_ENABLED);
    Options::setDefaultValue(OPTION_END_WHEN_OFFLINE, DEFAULT_END_WHEN_OFFLINE);
    if (FOptionsManager)
    {
        IOptionsDialogNode otrNode = { ONO_OTR, OPN_OTR, MNI_OTR_ENCRYPTED, tr("OTR Messaging") };
        FOptionsManager->insertOptionsDialogNode(otrNode);
        FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> OtrPlugin::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
    Q_UNUSED(AParent);
    QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_OTR)
    {
        widgets.insertMulti(ONO_OTR, new ConfigDialog(m_otrConnection, this, AParent));
    }
    return widgets;
}

bool OtrPlugin::archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
    Q_UNUSED(AOrder);
    Q_UNUSED(AStreamJid);
    Q_UNUSED(ADirectionIn);

    return AMessage.stanza().attribute(SkipOtrCatcherFlag()) != "true";
}

void OtrPlugin::onStreamOpened( IXmppStream *AXmppStream )
{
    Q_UNUSED(AXmppStream);
}

void OtrPlugin::onStreamClosed( IXmppStream *AXmppStream )
{
    //Q_UNUSED(AXmppStream);
    QString account = FAccountManager->findAccountByStream(AXmppStream->streamJid())->accountId();

    if (m_onlineUsers.contains(account))
    {
        foreach(QString contact, m_onlineUsers.value(account).keys())
        {
            m_otrConnection->endSession(account, contact);
            m_onlineUsers[account][contact]->setIsLoggedIn(false);
            //m_onlineUsers[account][contact]->updateMessageState();
        }
    }
}

void OtrPlugin::onToolBarWidgetCreated(IMessageToolBarWidget *)
{
}

void OtrPlugin::onMessageWindowCreated(IMessageWindow *)
{
}

void OtrPlugin::onMessageWindowDestroyed(IMessageWindow *)
{
}

void OtrPlugin::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    QString account = FAccountManager->findAccountByStream(AWindow->streamJid())->accountId().toString();
    QString contact = AWindow->contactJid().uFull();
    OtrStateWidget *widget = new OtrStateWidget(this, m_otrConnection,AWindow, account, contact,
                                          AWindow->toolBarWidget()->toolBarChanger()->toolBar());
    AWindow->toolBarWidget()->toolBarChanger()->insertWidget(widget,TBG_MWTBW_CHATSTATES);
    widget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    widget->setPopupMode(QToolButton::InstantPopup);
}

void OtrPlugin::onChatWindowDestroyed(IMessageChatWindow *AWindow)
{
    Q_UNUSED(AWindow)
}

void OtrPlugin::onProfileOpened(const QString &AProfile)
{
    m_homePath = FOptionsManager->profilePath(AProfile);
    m_otrConnection = new OtrMessaging(this, policy());
}

void OtrPlugin::onPresenceOpened(IPresence *APresence)
{
    Q_UNUSED(APresence)
    m_inboundCatcher = new InboundStanzaCatcher(m_otrConnection, FAccountManager, this);
    m_outboundCatcher = new OutboundStanzaCatcher(m_otrConnection, FAccountManager, this);

    if (FStanzaProcessor)
    {
        IStanzaHandle shandle;
        shandle.handler = this;
        shandle.order = SHO_OTR;
        shandle.direction = IStanzaHandle::DirectionIn;
        shandle.conditions.append(SHC_PRESENCE);
        FSHIPresence = FStanzaProcessor->insertStanzaHandle(shandle);
        //
        IStanzaHandle handle_in;
        handle_in.handler = m_inboundCatcher;
        handle_in.order = -32767; // SHO
        handle_in.direction = IStanzaHandle::DirectionIn;
        //handle_in.streamJid = AXmppStream->streamJid();
        handle_in.conditions.append(SHC_MESSAGE);

        IStanzaHandle handle_out;
        handle_out.handler = m_outboundCatcher;
        handle_out.order = 32767; // SHO
        handle_out.direction = IStanzaHandle::DirectionOut;
        //handle_out.streamJid = AXmppStream->streamJid();
        handle_out.conditions.append(SHC_MESSAGE);

        FSHIMessage = FStanzaProcessor->insertStanzaHandle(handle_in);
        FSHOMessage = FStanzaProcessor->insertStanzaHandle(handle_out);
    }
}

//-----------------------------------------------------------------------------

void OtrPlugin::authenticateContact(const QString &account, const QString &contact)
{
    if (!m_onlineUsers.value(account).contains(contact))
    {
        m_onlineUsers[account][contact] = new PsiOtrClosure(account,
                                                            contact,
                                                            m_otrConnection);
    }
    m_onlineUsers[account][contact]->authenticateContact();
}

//-----------------------------------------------------------------------------

OtrPolicy OtrPlugin::policy() const
{
    QVariant policyOption = Options::node(OPTION_POLICY).value();
    return static_cast<OtrPolicy>(policyOption.toInt());
}

//-----------------------------------------------------------------------------

void OtrPlugin::optionChanged(const QString&)
{
    QVariant policyOption = Options::node(OPTION_POLICY).value();
    m_otrConnection->setPolicy(static_cast<OtrPolicy>(policyOption.toInt()));
}

//-----------------------------------------------------------------------------

QString OtrPlugin::dataDir()
{
    return m_homePath;
}

//-----------------------------------------------------------------------------

void OtrPlugin::sendMessage(const QString &account, const QString &contact, const QString& messagetxt)
{
    Message message;
    //QString id=AMessage.id();
    message.setType(Message::Chat).setBody(messagetxt);

    if (!message.body().isEmpty())
    {
        message.setTo(contact);//.setId(id);
        message.stanza().setAttribute(SkipOtrCatcherFlag(), "true");
        FMessageProcessor->sendMessage(FAccountManager->findAccountById(account)->streamJid(), message, IMessageProcessor::DirectionOut);
    }

}

//-----------------------------------------------------------------------------

bool OtrPlugin::isLoggedIn(const QString &account, const QString &contact)
{
    if (m_onlineUsers.contains(account) &&
        m_onlineUsers.value(account).contains(contact))
    {
        return m_onlineUsers.value(account).value(contact)->isLoggedIn();
    }

    return false;
}

//-----------------------------------------------------------------------------

void OtrPlugin::notifyUser(const QString &account, const QString &contact,
                              const QString& message, const OtrNotifyType& type)
{
    Q_UNUSED(message);
    Q_UNUSED(type);

    LOG_STRM_INFO(FAccountManager->findAccountById(account)->streamJid(),QString("OTR notifyUser, contact=%1").arg(contact));
}

//-----------------------------------------------------------------------------

bool OtrPlugin::displayOtrMessage(const QString &account,
                                     const QString &contact,
                                     const QString& message)
{
    Jid contactJid(contact);
    notifyInChatWindow(FAccountManager->findAccountById(account)->streamJid(),contactJid, message);
    LOG_STRM_INFO(FAccountManager->findAccountById(account)->streamJid(),QString("OTR displayOtrMessage, contact=%1").arg(contact));
    return true;
}

//-----------------------------------------------------------------------------

void OtrPlugin::stateChange(const QString &account, const QString &contact,
                               OtrStateChange change)
{
    LOG_STRM_INFO(FAccountManager->findAccountById(account)->streamJid(),QString("OTR stateChange, contact=%1").arg(contact));

    if (!m_onlineUsers.value(account).contains(contact))
    {
        m_onlineUsers[account][contact] = new PsiOtrClosure(account,
                                                            contact,
                                                            m_otrConnection);
    }

    bool verified  = m_otrConnection->isVerified(account, contact);
    bool encrypted = m_onlineUsers[account][contact]->encrypted();
    QString msg;

    switch (change)
    {
        case OTR_STATECHANGE_GOINGSECURE:
            msg = encrypted?
                      tr("Attempting to refresh the private conversation")
                    : tr("Attempting to start a private conversation");
            break;

        case OTR_STATECHANGE_GONESECURE:
            msg  = verified? tr("Private conversation started")
                           : tr("Unverified conversation started");
            break;

        case OTR_STATECHANGE_GONEINSECURE:
            msg  = tr("Private conversation lost");
            break;

        case OTR_STATECHANGE_CLOSE:
            msg  = tr("Private conversation closed");
            break;

        case OTR_STATECHANGE_REMOTECLOSE:
            msg  = tr("%1 has ended the private conversation with you; "
                      "you should do the same.")
                      .arg(humanContact(account, contact));
            break;

        case OTR_STATECHANGE_STILLSECURE:
            msg  = verified? tr("Private conversation refreshed")
                           : tr("Unverified conversation refreshed");
            break;

        case OTR_STATECHANGE_TRUST:
            msg  = verified? tr("Contact authenticated")
                           : tr("Contact not authenticated");
            break;
    }

    Jid contactJid(contact);
    notifyInChatWindow(FAccountManager->findAccountById(account)->streamJid(),contactJid, msg);
    emit otrStateChanged(FAccountManager->findAccountById(account)->streamJid(),contactJid);
}

//-----------------------------------------------------------------------------

void OtrPlugin::receivedSMP(const QString &account, const QString &contact,
                               const QString& question)
{
    LOG_STRM_INFO(FAccountManager->findAccountById(account)->streamJid(),QString("OTR receivedSMP, contact=%1").arg(contact));

    if (m_onlineUsers.contains(account) &&
        m_onlineUsers.value(account).contains(contact))
    {
        m_onlineUsers[account][contact]->receivedSMP(question);
    }
}

//-----------------------------------------------------------------------------

void OtrPlugin::updateSMP(const QString &account, const QString &contact,
                             int progress)
{
    LOG_STRM_INFO(FAccountManager->findAccountById(account)->streamJid(),QString("OTR updateSMP, contact=%1").arg(contact));

    if (m_onlineUsers.contains(account) &&
        m_onlineUsers.value(account).contains(contact))
    {
        m_onlineUsers[account][contact]->updateSMP(progress);
    }
}

//-----------------------------------------------------------------------------

QString OtrPlugin::humanAccount(const QString& accountId)
{
    /*QString human(FAccountManager->findAccountById(accountId)->accountId());

    return human.isEmpty()? accountId : human;*/
    return FAccountManager->findAccountById(accountId)->streamJid().bare();
}

//-----------------------------------------------------------------------------

QString OtrPlugin::humanAccountPublic(const QString& accountId)
{
    return FAccountManager->findAccountById(accountId)->streamJid().bare();
}

//-----------------------------------------------------------------------------

QString OtrPlugin::humanContact(const QString& accountId,
                                   const QString &AContactJid)
{
    Q_UNUSED(accountId)
    return AContactJid;
}

//-----------------------------------------------------------------------------

void OtrPlugin::notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const
{
    IMessageChatWindow *window = FMessageWidgets!=NULL ? FMessageWidgets->findChatWindow(AStreamJid,AContactJid,true) : NULL;
    if (window)
    {
        IMessageStyleContentOptions options;
        options.kind = IMessageStyleContentOptions::KindStatus;
        options.type |= IMessageStyleContentOptions::TypeEvent;
        options.direction = IMessageStyleContentOptions::DirectionIn;
        options.time = QDateTime::currentDateTime();
        window->viewWidget()->appendText(AMessage,options);
    }
}

bool OtrPlugin::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
    Q_UNUSED(AAccept)

    if (AHandlerId == FSHIPresence)
    {
        QDomElement xml = AStanza.document().firstChildElement("presence");
        if (!xml.isNull())
        {
            QString contact = AStanza.from();
            QString account = FAccountManager->findAccountByStream(AStreamJid)->accountId();

            if (AStanza.type() == PRESENCE_TYPE_AVAILABLE)
            {
                if (!m_onlineUsers.value(account).contains(contact))
                {
                    m_onlineUsers[account][contact] = new PsiOtrClosure(account,
                                                                        contact,
                                                                        m_otrConnection);
                }

                m_onlineUsers[account][contact]->setIsLoggedIn(true);
            }
            else if (AStanza.type() == PRESENCE_TYPE_UNAVAILABLE)
            {
                if (m_onlineUsers.contains(account) &&
                    m_onlineUsers.value(account).contains(contact))
                {
                    if (Options::node(OPTION_END_WHEN_OFFLINE).value().toBool())
                    {
                        m_otrConnection->expireSession(account, contact);
                    }
                    m_onlineUsers[account][contact]->setIsLoggedIn(false);
                    Jid contactJid(AStanza.from());
                    emit otrStateChanged(AStreamJid,contactJid);
                }
            }
        }
    }
    return false;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_otrplugin, OtrPlugin)
#endif

} // namespace psiotr
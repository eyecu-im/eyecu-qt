#include "otr.h"
#include "otrclosure.h"
#include "otroptions.h"

#include <definitions/toolbargroups.h>
#include <definitions/archivehandlerorders.h>
#include <definitions/optionvalues.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <utils/logger.h>

#include <QPair>
#include <QMenu>
#include <QToolButton>

#define SHO_OTR             500
#define SHC_PRESENCE        "/presence"
#define SHC_MESSAGE         "/message"

Otr::Otr() :
	FOtrMessaging(new OtrMessaging(this)),
	FOnlineUsers(),
    FOptionsManager(NULL),
    FAccountManager(NULL),
    FPresenceManager(NULL),
    FMessageProcessor(NULL),
	FInboundCatcher(NULL),
	FOutboundCatcher(NULL)
{
}

Otr::~Otr()
{
	delete FOtrMessaging;
}

void Otr::pluginInfo(IPluginInfo *APluginInfo)
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

bool Otr::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

//	FHomePath = APluginManager->homePath();

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
			connect(FXmppStreams->instance(), SIGNAL(streamOpened(IXmppStream *)), SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreams->instance(), SIGNAL(streamClosed(IXmppStream *)), SLOT(onStreamClosed(IXmppStream *)));
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
			connect(FMessageWidgets->instance(), SIGNAL(normalWindowCreated(IMessageNormalWindow *)), SLOT(onMessageWindowCreated(IMessageNormalWindow *)));
			connect(FMessageWidgets->instance(), SIGNAL(normalWindowDestroyed(IMessageNormalWindow *)), SLOT(onMessageWindowDestroyed(IMessageNormalWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)), SLOT(onChatWindowCreated(IMessageChatWindow *)));
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowDestroyed(IMessageChatWindow *)), SLOT(onChatWindowDestroyed(IMessageChatWindow *)));
        }
    }

    return (FStanzaProcessor != NULL);

    return true;
}

bool Otr::initObjects()
{
    return true;
}

bool Otr::initSettings()
{
	Options::setDefaultValue(OPV_OTR_POLICY, PolicyEnabled);
	Options::setDefaultValue(OPV_OTR_ENDWHENOFFLINE, false);
    if (FOptionsManager)
    {
        IOptionsDialogNode otrNode = { ONO_OTR, OPN_OTR, MNI_OTR_ENCRYPTED, tr("OTR Messaging") };
        FOptionsManager->insertOptionsDialogNode(otrNode);
        FOptionsManager->insertOptionsDialogHolder(this);		
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Otr::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
    Q_UNUSED(AParent);
    QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_OTR)
		widgets.insertMulti(ONO_OTR, new OtrOptions(FOtrMessaging, AParent));
    return widgets;
}

bool Otr::archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn)
{
    Q_UNUSED(AOrder);
    Q_UNUSED(AStreamJid);
    Q_UNUSED(ADirectionIn);

    return AMessage.stanza().attribute(SkipOtrCatcherFlag()) != "true";
}

void Otr::onStreamOpened( IXmppStream *AXmppStream )
{
    Q_UNUSED(AXmppStream);
}

void Otr::onStreamClosed( IXmppStream *AXmppStream )
{
    //Q_UNUSED(AXmppStream);
	QString account = FAccountManager->findAccountByStream(AXmppStream->streamJid())->accountId().toString();

	if (FOnlineUsers.contains(account))
    {
		foreach(QString contact, FOnlineUsers.value(account).keys())
        {
			FOtrMessaging->endSession(account, contact);
			FOnlineUsers[account][contact]->setIsLoggedIn(false);
            //m_onlineUsers[account][contact]->updateMessageState();
        }
    }
}

void Otr::onToolBarWidgetCreated(IMessageToolBarWidget *)
{
}

void Otr::onMessageWindowCreated(IMessageNormalWindow *)
{
}

void Otr::onMessageWindowDestroyed(IMessageNormalWindow *)
{
}

void Otr::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    QString account = FAccountManager->findAccountByStream(AWindow->streamJid())->accountId().toString();
    QString contact = AWindow->contactJid().uFull();
	OtrStateWidget *widget = new OtrStateWidget(this, FOtrMessaging,AWindow, account, contact,
                                          AWindow->toolBarWidget()->toolBarChanger()->toolBar());
    AWindow->toolBarWidget()->toolBarChanger()->insertWidget(widget,TBG_MWTBW_CHATSTATES);
    widget->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    widget->setPopupMode(QToolButton::InstantPopup);
}

void Otr::onChatWindowDestroyed(IMessageChatWindow *AWindow)
{
    Q_UNUSED(AWindow)
}

void Otr::onProfileOpened(const QString &AProfile)
{
	FHomePath = FOptionsManager->profilePath(AProfile);
	FOtrMessaging->init();
//	FOtrMessaging = new OtrMessaging(this, Policy(Options::node(OPV_OTR_POLICY).value().toInt()));
}

void Otr::onPresenceOpened(IPresence *APresence)
{
    Q_UNUSED(APresence)
	FInboundCatcher = new InboundStanzaCatcher(FOtrMessaging, FAccountManager, this);
	FOutboundCatcher = new OutboundStanzaCatcher(FOtrMessaging, FAccountManager, this);

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
		handle_in.handler = FInboundCatcher;
        handle_in.order = -32767; // SHO
        handle_in.direction = IStanzaHandle::DirectionIn;
        //handle_in.streamJid = AXmppStream->streamJid();
        handle_in.conditions.append(SHC_MESSAGE);

        IStanzaHandle handle_out;
		handle_out.handler = FOutboundCatcher;
        handle_out.order = 32767; // SHO
        handle_out.direction = IStanzaHandle::DirectionOut;
        //handle_out.streamJid = AXmppStream->streamJid();
        handle_out.conditions.append(SHC_MESSAGE);

        FSHIMessage = FStanzaProcessor->insertStanzaHandle(handle_in);
        FSHOMessage = FStanzaProcessor->insertStanzaHandle(handle_out);
    }
}

//-----------------------------------------------------------------------------

void Otr::authenticateContact(const QString &AAccount, const QString &AContact)
{
	if (!FOnlineUsers.value(AAccount).contains(AContact))
    {
		FOnlineUsers[AAccount][AContact] = new OtrClosure(AAccount,
															AContact,
															FOtrMessaging);
    }
	FOnlineUsers[AAccount][AContact]->authenticateContact();
}

//-----------------------------------------------------------------------------

//void Otr::optionChanged(const QString &AOption)
//{
//	QVariant policyOption = Options::node(OPV_OTR_POLICY).value();
//	FOtrMessaging->setPolicy(static_cast<Policy>(policyOption.toInt()));
//}

//-----------------------------------------------------------------------------

QString Otr::dataDir()
{
	return FHomePath;
}

//-----------------------------------------------------------------------------

void Otr::sendMessage(const QString &account, const QString &contact, const QString& AMessage)
{
    Message message;
    //QString id=AMessage.id();
	message.setType(Message::Chat).setBody(AMessage);

    if (!message.body().isEmpty())
    {
        message.setTo(contact);//.setId(id);
        message.stanza().setAttribute(SkipOtrCatcherFlag(), "true");
        FMessageProcessor->sendMessage(FAccountManager->findAccountById(account)->streamJid(), message, IMessageProcessor::DirectionOut);
    }

}

//-----------------------------------------------------------------------------

bool Otr::isLoggedIn(const QString &AAccount, const QString &AContact)
{
	if (FOnlineUsers.contains(AAccount) &&
		FOnlineUsers.value(AAccount).contains(AContact))
    {
		return FOnlineUsers.value(AAccount).value(AContact)->isLoggedIn();
    }

    return false;
}

//-----------------------------------------------------------------------------

void Otr::notifyUser(const QString &AAccount, const QString &AContact,
							  const QString& AMessage, const NotifyType& AType)
{
	Q_UNUSED(AMessage);
	Q_UNUSED(AType);

	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR notifyUser, contact=%1").arg(AContact));
}

//-----------------------------------------------------------------------------

bool Otr::displayOtrMessage(const QString &AAccount,
									 const QString &AContact,
									 const QString& AMessage)
{
	Jid contactJid(AContact);
	notifyInChatWindow(FAccountManager->findAccountById(AAccount)->streamJid(),contactJid, AMessage);
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR displayOtrMessage, contact=%1").arg(AContact));
    return true;
}

//-----------------------------------------------------------------------------

void Otr::stateChange(const QString &AAccount, const QString &AContact,
							   StateChange AChange)
{
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR stateChange, contact=%1").arg(AContact));

	if (!FOnlineUsers.value(AAccount).contains(AContact))
    {
		FOnlineUsers[AAccount][AContact] = new OtrClosure(AAccount,
															AContact,
															FOtrMessaging);
    }

	bool verified  = FOtrMessaging->isVerified(AAccount, AContact);
	bool encrypted = FOnlineUsers[AAccount][AContact]->encrypted();
    QString msg;

	switch (AChange)
    {
		case StateChangeGoingSecure:
            msg = encrypted?
                      tr("Attempting to refresh the private conversation")
                    : tr("Attempting to start a private conversation");
            break;

		case StateChangeGoneSecure:
            msg  = verified? tr("Private conversation started")
                           : tr("Unverified conversation started");
            break;

		case StateChangeGoneInsecure:
            msg  = tr("Private conversation lost");
            break;

		case StateChangeClose:
            msg  = tr("Private conversation closed");
            break;

		case StateChangeRemoteClose:
            msg  = tr("%1 has ended the private conversation with you; "
                      "you should do the same.")
					  .arg(humanContact(AAccount, AContact));
            break;

		case StateChangeStillSecure:
            msg  = verified? tr("Private conversation refreshed")
                           : tr("Unverified conversation refreshed");
            break;

		case StateChangeTrust:
            msg  = verified? tr("Contact authenticated")
                           : tr("Contact not authenticated");
            break;
    }

	Jid contactJid(AContact);
	notifyInChatWindow(FAccountManager->findAccountById(AAccount)->streamJid(),contactJid, msg);
	emit otrStateChanged(FAccountManager->findAccountById(AAccount)->streamJid(),contactJid);
}

//-----------------------------------------------------------------------------

void Otr::receivedSMP(const QString &AAccount, const QString &AContact,
							   const QString& AQuestion)
{
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR receivedSMP, contact=%1").arg(AContact));

	if (FOnlineUsers.contains(AAccount) &&
		FOnlineUsers.value(AAccount).contains(AContact))
    {
		FOnlineUsers[AAccount][AContact]->receivedSMP(AQuestion);
    }
}

//-----------------------------------------------------------------------------

void Otr::updateSMP(const QString &AAccount, const QString &AContact,
							 int AProgress)
{
	LOG_STRM_INFO(FAccountManager->findAccountById(AAccount)->streamJid(),QString("OTR updateSMP, contact=%1").arg(AContact));

	if (FOnlineUsers.contains(AAccount) &&
		FOnlineUsers.value(AAccount).contains(AContact))
    {
		FOnlineUsers[AAccount][AContact]->updateSMP(AProgress);
    }
}

//-----------------------------------------------------------------------------

QString Otr::humanAccount(const QString& AAccountId)
{
    /*QString human(FAccountManager->findAccountById(accountId)->accountId());

    return human.isEmpty()? accountId : human;*/
	return FAccountManager->findAccountById(AAccountId)->streamJid().bare();
}

//-----------------------------------------------------------------------------

QString Otr::humanAccountPublic(const QString& AAccountId)
{
	return FAccountManager->findAccountById(AAccountId)->streamJid().bare();
}

//-----------------------------------------------------------------------------

QString Otr::humanContact(const QString& AAccountId,
								   const QString &AContactJid)
{
	Q_UNUSED(AAccountId)
    return AContactJid;
}

//-----------------------------------------------------------------------------

void Otr::notifyInChatWindow(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessage) const
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

bool Otr::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
    Q_UNUSED(AAccept)

    if (AHandlerId == FSHIPresence)
    {
        QDomElement xml = AStanza.document().firstChildElement("presence");
        if (!xml.isNull())
        {
            QString contact = AStanza.from();
			QString account = FAccountManager->findAccountByStream(AStreamJid)->accountId().toString();

            if (AStanza.type() == PRESENCE_TYPE_AVAILABLE)
            {
				if (!FOnlineUsers.value(account).contains(contact))
                {
					FOnlineUsers[account][contact] = new OtrClosure(account,
                                                                        contact,
																		FOtrMessaging);
                }

				FOnlineUsers[account][contact]->setIsLoggedIn(true);
            }
            else if (AStanza.type() == PRESENCE_TYPE_UNAVAILABLE)
            {
				if (FOnlineUsers.contains(account) &&
					FOnlineUsers.value(account).contains(contact))
                {
					if (Options::node(OPV_OTR_ENDWHENOFFLINE).value().toBool())
                    {
						FOtrMessaging->expireSession(account, contact);
                    }
					FOnlineUsers[account][contact]->setIsLoggedIn(false);
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

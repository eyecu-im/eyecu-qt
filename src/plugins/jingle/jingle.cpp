#include "jingle.h"

#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <interfaces/ipresencemanager.h>
#include <utils/xmpperror.h>
#include <utils/logger.h>

#include <QList>

#define SHC_REQUEST "/iq[@type='set']/jingle[@xmlns='" NS_JINGLE "']"
#define SHC_RESULT  "/iq[@type='result']"
#define SHC_ERROR  "/iq[@type='error']"

#define SHO_DAFAULT  1000

Jingle::Jingle(QObject *parent):
	QObject(parent),
	FStanzaProcessor(nullptr),
	FServiceDiscovery(nullptr),
	FOptionsManager(nullptr)
{
	JingleSession::setJingle(this);
}

Jingle::~Jingle()
{}

bool Jingle::sendStanzaOut(Stanza &AStanza)
{
	return FStanzaProcessor?FStanzaProcessor->sendStanzaOut(Jid(AStanza.from()), AStanza):false;
}

void Jingle::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Jingle");
	APluginInfo->description = tr("Allows to establish Jingle sessions for media exchange");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences << SERVICEDISCOVERY_UUID
							 << STANZAPROCESSOR_UUID;
}

bool Jingle::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{	
	Q_UNUSED(AInitOrder)

	IPlugin *plugin= APluginManager->pluginInterface("IStanzaProcessor").value(0,nullptr);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,nullptr);
	if (plugin)
		FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,nullptr);
	if (plugin)
		JingleSession::setPresenceManager(qobject_cast<IPresenceManager *>(plugin->instance()));

	// Find application and transport plugins
	QList<IPlugin *>plugins = APluginManager->pluginInterface("IJingleApplication");
	if (plugins.isEmpty())
		return false;

	for (QList<IPlugin *>::Iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		IJingleApplication *application=qobject_cast<IJingleApplication *>((*it)->instance());
		FApplications.insert(application->ns(), application);

		connect(this,SIGNAL(connectionOpened(IJingleContent*)),(*it)->instance(),
				SLOT(onConnectionEstablished(IJingleContent*)), Qt::QueuedConnection);
		connect(this,SIGNAL(connectionFailed(IJingleContent*)),(*it)->instance(),
				SLOT(onConnectionFailed(IJingleContent*)), Qt::QueuedConnection);
	}

	plugins = APluginManager->pluginInterface("IJingleTransport");
	if (plugins.isEmpty())
		return false;

	for (QList<IPlugin *>::Iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		IJingleTransport *transort=qobject_cast<IJingleTransport *>((*it)->instance());
		FTransports.insertMulti(transort->priority(), transort);
		connect((*it)->instance(),SIGNAL(connectionOpened(IJingleContent*)),
								  SLOT(onConnectionOpened(IJingleContent*)),
				Qt::QueuedConnection);
		connect((*it)->instance(),SIGNAL(connectionError(IJingleContent*)),
								  SLOT(onConnectionFailed(IJingleContent*)),
				Qt::QueuedConnection);
		connect((*it)->instance(),SIGNAL(incomingTransportFilled(IJingleContent*)),
								  SLOT(onIncomingTransportFilled(IJingleContent*)),
				Qt::QueuedConnection);
		connect((*it)->instance(),SIGNAL(incomingTransportFillFailed(IJingleContent*)),
								  SLOT(onIncomingTransportFillFailed(IJingleContent*)),
				Qt::QueuedConnection);
	}

	return true;
}

bool Jingle::initObjects()
{
//    XmppError::registerErrorString(NS_XMPP_STANZAS,"service-unavailable",tr("Service unavailable"));
//    XmppError::registerErrorString(NS_XMPP_STANZAS,"redirect",tr("Redirected"));
//    XmppError::registerErrorString(NS_XMPP_STANZAS,"resource-constraint",tr("Busy"));
//    XmppError::registerErrorString(NS_XMPP_STANZAS,"bad-request",tr("Error in request"));

	if (FServiceDiscovery)
		registerDiscoFeatures();    // Register discovery features
	else
		return false;

	if (FOptionsManager)
	{
		IOptionsDialogNode dnode = {ONO_JINGLETRANSPORTS, OPN_JINGLETRANSPORTS, MNI_JINGLE, tr("Jingle transports")};
		FOptionsManager->insertOptionsDialogNode(dnode);
	}

	if (FStanzaProcessor)
	{   // Register Stanza handlers
		IStanzaHandle requestHandle;
		requestHandle.handler = this;
		requestHandle.order = SHO_DEFAULT;
		requestHandle.direction = IStanzaHandle::DirectionIn;
		requestHandle.conditions.append(SHC_REQUEST);
		FSHIRequest = FStanzaProcessor->insertStanzaHandle(requestHandle);

		requestHandle.conditions.clear();
		requestHandle.conditions.append(SHC_RESULT);
		FSHIResult = FStanzaProcessor->insertStanzaHandle(requestHandle);

		requestHandle.conditions.clear();
		requestHandle.conditions.append(SHC_ERROR);
		FSHIError = FStanzaProcessor->insertStanzaHandle(requestHandle);
	}
	else
		return false;

	return true;
}

bool Jingle::initSettings()
{
	return true;
}

void Jingle::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.var = NS_JINGLE;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_JINGLE);
	dfeature.name = tr("Jingle");
	dfeature.description = tr("Supports XEP-0166: Jingle");
	FServiceDiscovery->insertDiscoFeature(dfeature);
}

void Jingle::onConnectionOpened(IJingleContent *AContent)
{
	emit connectionOpened(AContent);
}

void Jingle::onConnectionFailed(IJingleContent *AContent)
{
	emit connectionFailed(AContent);
}

void Jingle::onIncomingTransportFilled(IJingleContent *AContent)
{
	JingleSession *session = JingleSession::sessionBySessionId(AContent->sid());
	if (session)
	{
		if (session->isOutgoing() && session->status() == Initiating)
		{
			if (!session->initiate())
				LOG_ERROR("Session initiate failed!");
//TODO: Notify application about failure
		}
		else if (!session->isOutgoing() && session->status() == Accepting)
		{
			if (!session->accept())
				LOG_ERROR("Session accept failed!");
//TODO: Notify application about failure
		}
	}
	else
		LOG_ERROR(QString("Session not found! sid=%1").arg(AContent->sid()));
}

void Jingle::onIncomingTransportFillFailed(IJingleContent *AContent)
{
	LOG_DEBUG(QString("Jingle::onIncomingTransportFillFailed(%1)")
			  .arg(int(AContent), 8, 16));
	JingleSession *session = JingleSession::sessionBySessionId(AContent->sid());
	if (session)
	{
		if (session->status() == Initiating) // Didn't send session-initiate request yet
			session->setTerminated(FailedTransport);
		else
			session->terminate(FailedTransport);
	}
}

bool Jingle::processSessionInitiate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	LOG_DEBUG(QString("Jingle::processSessionInitiate(%1, %2, %3)").arg(AStreamJid.full()).arg(AStanza.toString()).arg(AAccept));
	JingleSession *session = new JingleSession(AStanza);

	if (!session->isOk())
	{
		LOG_WARNING("Session exists! Replying with error!");
		Stanza ack = AStanza.ack(IJingle::ResourceConstraint);
		FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		session->deleteLater();
		return false;
	}
	else if (!session->isValid())
	{
		LOG_WARNING("Invalid session! Replying with error!");
		Stanza ack = AStanza.ack(IJingle::BadRequest);
		FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		session->deleteLater();
		return false;
	}

	AAccept=true;

	IJingleApplication *app(nullptr);
	QString appns;
	QHash<QString, JingleContent *> contents=session->contents();
	bool wrong(false);
	for (QHash<QString, JingleContent *>::ConstIterator it=contents.constBegin();
		 it!=contents.constEnd(); it++)
	{
		bool supported(false);

		QDomElement descr = (*it)->description();
		if (descr.isNull())
		{
			wrong = true;
			break;
		}
		QString appns_=descr.namespaceURI();
		if (appns_.isNull())
		{
			wrong = true;
			break;
		}
		if (appns.isNull())
		{
			appns = appns_;
			app = FApplications.value(appns);
			if (app && app->checkSupported(descr))
				supported = true;
			else
				break;
		}
		else
			if (appns != appns_)
			{
				wrong = true;
				break;
			}

		if (supported)  // Ok, let's check, if transport is supported
		{
			supported = false;
			for (QMap<int, IJingleTransport*>::ConstIterator itt=FTransports.constBegin();
				 itt != FTransports.constEnd(); ++itt)
				if ((*itt)->ns() == (*it)->transportNS())
				{
					supported = true;
					break;
				}
		}
		if (!supported)
			session->deleteContent(it.key());
	}

	if (wrong || session->contents().isEmpty())
	{
		LOG_WARNING("No contents! Replying with error!");
		Stanza ack = AStanza.ack(BadRequest);
		FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		delete session;
		return false;
	}

	Stanza ack = AStanza.ack(Acknowledge);
	if (FStanzaProcessor->sendStanzaOut(AStreamJid, ack))
	{
		session->setInitiated(app);
		return true;
	}
	LOG_FATAL("Something gone wrong! Returning false.");
	return false;
}

bool Jingle::processSessionAccept(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	AAccept=true;
	JingleSession *session = JingleSession::sessionBySessionId(AStanza.sid());
	if (session)
	{
		QDomElement jingle = AStanza.jingleElement();
		for (QDomElement contentElement = jingle.firstChildElement("content");
			 !contentElement.isNull();
			 contentElement=contentElement.nextSiblingElement("content"))
		{
			JingleContent *content = session->getContent(contentElement.attribute("name"));
			if (content)
			{
				QDomElement transport = contentElement.firstChildElement("transport");
				if (transport.isNull())
					LOG_ERROR("Outgoing transport is NULL!");
				else
				{
					if (content->setOutgoingTransport(transport))
						LOG_DEBUG("Outgoing transport set successfuly!");
					else
						LOG_ERROR("Outgoing transport set error!");
				}
			}
			else
				LOG_WARNING(QString("Content with name=\"%1\" found!").arg(contentElement.attribute("name")));
		}

		Stanza ack=AStanza.ack(Acknowledge);
		if (FStanzaProcessor->sendStanzaOut(AStreamJid, ack))
		{
			session->setAccepted();
			LOG_DEBUG("returning true");
			return true;
		}
	}
	else
		LOG_WARNING(QString("session not found: sid=%1").arg(AStanza.sid()));
	LOG_DEBUG("returning false");
	return false;
}

bool Jingle::processSessionTerminate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	AAccept=true;
	bool result;
	JingleSession *session=JingleSession::sessionBySessionId(AStanza.sid());
	if (session)
	{
		Stanza ack=AStanza.ack(Acknowledge);
		result=FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		session->setTerminated(AStanza.reason());
	}
	else
		result=false;
	return result;
}

bool Jingle::processSessionInfo(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	AAccept=true;
	bool result;
	JingleSession *session=JingleSession::sessionBySessionId(AStanza.sid());
	if (session)
	{
		Stanza ack=AStanza.ack(Acknowledge);
		result=FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		session->inform(AStanza);
	}
	else
		result=false;
	return result;
}

IJingleTransport *Jingle::transportByNs(const QString &ANameSpace)
{
	for (QMap<int, IJingleTransport*>::ConstIterator it=FTransports.constBegin();
		 it != FTransports.constEnd(); ++it)
		if ((*it)->ns() == ANameSpace)
			return *it;
	return nullptr;
}

bool Jingle::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (AHandleId==FSHIRequest)
	{
		JingleStanza stanza(AStanza);

		switch (stanza.action())
		{
			case IJingle::SessionInitiate:
				return processSessionInitiate(AStreamJid, stanza, AAccept);

			case IJingle::SessionAccept:
				return processSessionAccept(AStreamJid, stanza, AAccept);

			case IJingle::SessionTerminate:
				return processSessionTerminate(AStreamJid, stanza, AAccept);

			case IJingle::SessionInfo:
				return processSessionInfo(AStreamJid, stanza, AAccept);
			default:
				break;
		}
	}
	else if (AHandleId==FSHIResult || AHandleId==FSHIError)
	{
		JingleSession *session=JingleSession::sessionByStanzaId(AStanza.id());
		if (session)
		{
			if (AHandleId==FSHIResult)
				session->acknowledge(Acknowledge, Jid());
			else    // Error!
			{
				LOG_WARNING(QString("Error! sid=%1").arg(session->sid()));
				QDomElement error=AStanza.firstElement("error");
				if (!error.isNull())
					session->acknowledge(BadRequest, Jid());
			}
		}
	}
	AAccept=true;
	return false;
}

QString Jingle::sessionCreate(const Jid &AStreamJid, const Jid &AContactJid, const QString &AApplicationNS)
{
	JingleSession *session = new JingleSession(AStreamJid, AContactJid, AApplicationNS);
	return session->sid();
}

bool Jingle::sessionAccept(const QString &ASid)
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->accept():false;
}

bool Jingle::sessionTerminate(const QString &ASid, Reason AReason)
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->terminate(AReason):false;
}

bool Jingle::sessionDestroy(const QString &ASid)
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	if (session) {
		session->deleteLater();
		return true;
	}
	else
		return false;
}

bool Jingle::sendAction(const QString &ASid, IJingle::Action AAction,
						const QDomElement &AJingleElement)
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->sendAction(AAction, AJingleElement):false;
}

bool Jingle::sendAction(const QString &ASid, IJingle::Action AAction,
						const QDomNodeList &AJingleElements)
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->sendAction(AAction, AJingleElements):false;
}

// Sessions
IJingleContent *Jingle::contentAdd(const QString &ASid, const QString &AName,
								   const QString &AMediaType, int AComponentCount,
								   IJingleTransport::Type ATransportType,
								   bool AFromResponder)
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	if (session)
	{
		for (QMap<int, IJingleTransport*>::ConstIterator it=FTransports.constBegin();
			 it != FTransports.constEnd(); ++it)
			if ((*it)->types().testFlag(ATransportType) &&
				FServiceDiscovery->discoInfo(session->thisParty(),
											 session->otherParty()).features.contains((*it)->ns()))
			{
				IJingleContent *content = session->addContent(AName, AMediaType,
															  AComponentCount,
															  *it, AFromResponder);
				if (content)
				{
					LOG_DEBUG("Jingle content added!");
					return content;
				}
				else
					LOG_ERROR("Content creation failed!");
			}

		// An appropriate transport not found
		session->deleteContent(AName);
	}
	return nullptr;
}

IJingle::SessionStatus Jingle::sessionStatus(const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->status():IJingle::None;
}

bool Jingle::isOutgoing(const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->isOutgoing():false;
}

Jid Jingle::contactJid(const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->otherParty():Jid();
}

Jid Jingle::streamJid(const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(ASid);
	return session?session->thisParty():Jid();
}

QString Jingle::errorMessage(IJingle::Reason AReason) const
{
	switch (AReason)
	{
		case Success:
			return tr("Success");
		case Busy:
			return tr("Busy");
		case Cancel:
			return tr("Cancelled");
		case AlternativeSession:
			return tr("Alternative session");
		case ConnectivityError:
			return tr("Connectivity error");
		case Decline:
			return tr("Declined");
		case Expired:
			return tr("Session expired");
		case FailedApplication:
			return tr("Application failure");
		case FailedTransport:
			return tr("Transport failure");
		case GeneralError:
			return tr("General error");
		case Gone:
			return tr("Session gone");
		case IncompatibleParameters:
			return tr("Incompatible parameters");
		case MediaError:
			return tr("Media error");
		case SecurityError:
			return tr("Security error");
		case Timeout:
			return tr("Timeout");
		case UnsupportedApplications:
			return tr("Application not supported");
		case UnsupportedTransports:
			return tr("Transport not supported");
		default:
			return QString::null;
	}
}

QHash<QString, IJingleContent *> Jingle::contents(const QString &ASid) const
{
	QHash<QString, IJingleContent *> rc;
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	if (session)
		for (QHash<QString, JingleContent *>::ConstIterator it=session->contents().constBegin(); it!=session->contents().constEnd(); it++)
			rc.insert(it.key(), *it);
	return rc;
}

IJingleContent *Jingle::content(const QString &ASid, const QString &AName) const
{
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	return session?session->getContent(AName):nullptr;
}

IJingleContent *Jingle::content(const QString &ASid, QIODevice *ADevice) const
{
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	return session?session->getContent(ADevice):nullptr;
}

bool Jingle::selectTransportCandidate(const QString &ASid, const QString &AContentName, const QString &ACandidateId)
{
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	if (session)
		return session->selectTransportCandidate(AContentName, ACandidateId);
	return false;
}

bool Jingle::connectContent(const QString &ASid, const QString &AName)
{
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	if (session)
	{
		IJingleContent *content =  session->getContent(AName);
		if (content)
		{
			IJingleTransport *transport(transportByNs(content->transportNS()));
			if (transport)
				return transport->openConnection(content);
			else
				LOG_ERROR("Invalid transport!");
		}
		else
			LOG_ERROR("No content!");
	}
	else
		LOG_ERROR("No session!");
	return false;
}

bool Jingle::setConnected(const QString &ASid)
{
	LOG_DEBUG(QString("Jingle::setConnected(%1)").arg(ASid));
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	if (session)
	{
		session->setConnected();
		return true;
	}
	else
		LOG_ERROR("Session not found!");
	return false;
}

bool Jingle::setAccepting(const QString &ASid)
{
	LOG_DEBUG(QString("Jingle::setAccepting(%1)").arg(ASid));
	JingleSession *session = JingleSession::sessionBySessionId(ASid);
	if (session)
	{
		session->setAccepting();
		return true;
	}
	else
		LOG_ERROR("Session not found!");
	return false;

}

bool Jingle::fillIncomingTransport(IJingleContent *AContent)
{
	IJingleTransport *transport(transportByNs(AContent->transportNS()));
	if (transport)
		return transport->fillIncomingTransport(AContent);
	else
	{
		LOG_ERROR("Invalid transport");
		return false;
	}
}

void Jingle::freeIncomingTransport(IJingleContent *AContent)
{
	IJingleTransport *transport(transportByNs(AContent->transportNS()));
	if (transport)
		transport->freeIncomingTransport(AContent);
	else
		LOG_ERROR("Invalid transport");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_Jingle,Jingle)
#endif

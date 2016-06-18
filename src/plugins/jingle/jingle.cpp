#include "jingle.h"
#include "definitions/namespaces.h"
#include "definitions/menuicons.h"
#include "definitions/resources.h"
#include "definitions/stanzahandlerorders.h"
#include "definitions/optionnodes.h"
#include "definitions/optionnodeorders.h"
#include "utils/xmpperror.h"

#include <QList>

#define SHC_REQUEST "/iq[@type='set']/jingle[@xmlns='" NS_JINGLE "']"
#define SHC_RESULT  "/iq[@type='result']"
#define SHC_ERROR  "/iq[@type='error']"

#define SHO_DAFAULT  1000

Jingle::Jingle(QObject *parent):
	QObject(parent),
	FStanzaProcessor(NULL),
	FServiceDiscovery(NULL),
	FOptionsManager(NULL)
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
	APluginInfo->description = tr("Implements XEP-0166: Jingle");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences << SERVICEDISCOVERY_UUID
							 << STANZAPROCESSOR_UUID;
}

bool Jingle::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{	
	Q_UNUSED(AInitOrder)

	IPlugin *plugin= APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	// Find application and transport plugins
	QList<IPlugin *>plugins = APluginManager->pluginInterface("IJingleApplication");
	if (plugins.isEmpty())
		return false;

	for (QList<IPlugin *>::iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		IJingleApplication *application=qobject_cast<IJingleApplication *>((*it)->instance());
		FApplications.insert(application->ns(), application);
		connect(this,SIGNAL(startSendData(IJingleContent *)),(*it)->instance(), SLOT(onConnectionEstablished(IJingleContent *)),Qt::QueuedConnection);
		connect(this,SIGNAL(connectionFailed(IJingleContent *)),(*it)->instance(), SLOT(onConnectionFailed(IJingleContent *)),Qt::QueuedConnection);
	}

	plugins = APluginManager->pluginInterface("IJingleTransport");
	if (plugins.isEmpty())
		return false;

	for (QList<IPlugin *>::iterator it=plugins.begin(); it!=plugins.end(); it++)
	{
		IJingleTransport *transort=qobject_cast<IJingleTransport *>((*it)->instance());
		FTransports.insert(transort->ns(), transort);
		connect((*it)->instance(),SIGNAL(connectionOpened(IJingleContent*)),
								  SLOT(onConnectionOpened(IJingleContent*)),Qt::QueuedConnection);
		connect((*it)->instance(),SIGNAL(connectionError(IJingleContent*)),
								  SLOT(onConnectionFailed(IJingleContent*)),Qt::QueuedConnection);
		connect((*it)->instance(),SIGNAL(incomingTransportFilled(IJingleContent*)),
								  SLOT(onIncomingTransportFilled(IJingleContent*)),Qt::QueuedConnection);
		connect((*it)->instance(),SIGNAL(incomingTransportFillFailed(IJingleContent*)),
								  SLOT(onIncomingTransportFillFailed(IJingleContent*)),Qt::QueuedConnection);
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
	emit startSendData(AContent);
}

void Jingle::onConnectionFailed(IJingleContent *AContent)
{
	emit connectionFailed(AContent);
}

void Jingle::onIncomingTransportFilled(IJingleContent *AContent)
{
	qDebug() << "Jingle::onIncomingTransportFilled(" << AContent << ")";
	QDomElement incoming = AContent->transportIncoming();
	if (incoming.isNull())
		qWarning() << "incoming is NULL!";
	else
	{
		QDomElement candidate = incoming.firstChildElement("candidate");
		if (candidate.isNull())
			qWarning() << "candidate is NULL!";
		else
		{
			qDebug() << "ip=" << candidate.attribute("ip");
			qDebug() << "port=" << candidate.attribute("port");
		}
	}
	if (FPendingContents.contains(AContent))
	{
		FPendingContents.removeAll(AContent);
		qDebug() << "emitting contentAdded(" << AContent->name() << ")";
		emit contentAdded(AContent);
	}
	else
	{
		qDebug() << "incomingTransportFilled(" << AContent->name() << ")";
		emit incomingTransportFilled(AContent);
	}
	qDebug() << "Jingle::onIncomingTransportFilled(): Finished!";
}

void Jingle::onIncomingTransportFillFailed(IJingleContent *AContent)
{
	qDebug() << "Jingle::onIncomingTransportFillFailed(" << AContent << ")";
	if (FPendingContents.contains(AContent))
	{
		FPendingContents.removeAll(AContent);
		emit contentAddFailed(AContent);
		JingleSession::sessionBySessionId(AContent->streamJid(), AContent->sid())->deleteContent(AContent->name());
	}
	else
		emit incomingTransportFillFailed(AContent);
}

bool Jingle::processSessionInitiate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	qDebug() << "Jingle::processSessionInitiate(" << AStreamJid.full() << "," << AStanza.toString() << "," << AAccept << ")";
	JingleSession *session = new JingleSession(AStanza);

	if (!session->isOk())
	{
		qWarning() << "Session exists! Replying with error!";
		Stanza ack = AStanza.ack(IJingle::ResourceConstraint);
		FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		session->deleteLater();
		return false;
	}
	else if (!session->isValid())
	{
		qWarning() << "Invalid session! Replying with error!";
		Stanza ack = AStanza.ack(IJingle::BadRequest);
		FStanzaProcessor->sendStanzaOut(AStreamJid, ack);
		session->deleteLater();
		return false;
	}

	AAccept=true;

	IJingleApplication *app=NULL;
	QString appns;
	QHash<QString, JingleContent *> contents=session->contents();
	bool    wrong=false;
	for (QHash<QString, JingleContent *>::const_iterator it=contents.constBegin(); it!=contents.constEnd(); it++)
	{
		bool    supported=false;

		QDomElement descr = (*it)->description();
		if (descr.isNull())
		{
			wrong = true;
			break;
		}
		QString appns_=descr.namespaceURI();
		if (appns_.isNull())
		{
			wrong=true;
			break;
		}
		if (appns.isNull())
		{
			appns=appns_;
			app=FApplications.value(appns);
			if (app && app->checkSupported(descr))
				supported=true;
			else
				break;
		}
		else
			if (appns!=appns_)
			{
				wrong=true;
				break;
			}

		if (supported)  // Ok, let's check, if transport is supported
		{
			supported=false;
			if (FTransports.contains((*it)->transportNS()))
			{
				supported=true;
				break;
			}
		}
		if (!supported)
			session->deleteContent(it.key());
	}

	if (wrong || session->contents().isEmpty())
	{
		qWarning() << "No contents! Replying with error!";
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
	qWarning() << "Something gone wrong! Returning false.";
	return false;
}

bool Jingle::processSessionAccept(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	qDebug() << "Jingle::processSessionAccept(" << AStreamJid.full() << "," << AStanza.toString() << "," << AAccept << ")";
	AAccept=true;
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, AStanza.sid());
	if (session)
	{
		QDomElement jingle = AStanza.jingleElement();
		for (QDomElement contentElement = jingle.firstChildElement("content"); !contentElement.isNull(); contentElement=contentElement.nextSiblingElement("content"))
		{
			JingleContent *content = session->getContent(contentElement.attribute("name"));
			if (content)
			{
				QDomElement transport = contentElement.firstChildElement("transport");
				if (!transport.isNull())
					if (content->setOutgoingTransport(transport))
						qDebug() << "Outgoing transport set successfuly!";
					else
						qWarning() << "Outgoing transport set error!";
				else
					qWarning() << "Outgoing transport is NULL!";
			}
			else
				qWarning() << "Content with name=\"" << contentElement.attribute("name") << "\" found!";
		}

		Stanza ack=AStanza.ack(Acknowledge);
		if (FStanzaProcessor->sendStanzaOut(AStreamJid, ack))
		{
			session->setAccepted();
			return true;
		}
	}
	else
		qWarning() << "session not found: sid=" << AStanza.sid();
	return false;
}

bool Jingle::processSessionTerminate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept)
{
	AAccept=true;
	bool result;
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, AStanza.sid());
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
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, AStanza.sid());
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

bool Jingle::stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
//	qDebug() << "Jingle::stanzaReadWrite(" << AHandleId << "," << AStreamJid.full() << "," << AStanza.toString() << "," << AAccept << ")";
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
		JingleSession *session=JingleSession::sessionByStanzaId(AStreamJid, AStanza.id());
		if (session)
		{
			if (AHandleId==FSHIResult)
				session->acknowledge(Acknowledge, Jid());
			else    // Error!
			{
				qWarning() << "Error! sid=" << session->sid();
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

bool Jingle::sessionInitiate(const Jid &AStreamJid, const QString &ASid)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->initiate():false;
}

bool Jingle::sessionAccept(const Jid &AStreamJid, const QString &ASid)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->accept():false;
}

bool Jingle::sessionTerminate(const Jid &AStreamJid, const QString &ASid, Reason AReason)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->terminate(AReason):false;
}

bool Jingle::sendAction(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, const QDomElement &AJingleElement)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->sendAction(AAction, AJingleElement):false;
}

bool Jingle::sendAction(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, const QDomNodeList &AJingleElements)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->sendAction(AAction, AJingleElements):false;
}

// Sessions
IJingleContent *Jingle::contentAdd(const Jid &AStreamJid, const QString &ASid, const QString &AName, const QString &AMediaType, const QString &ATransportNameSpace, bool AFromResponder)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	if (session)
	{
		IJingleContent *content=session->addContent(AName, AMediaType, ATransportNameSpace, AFromResponder);
		if (content)
		{
			if (FTransports[ATransportNameSpace]->fillIncomingTransport(content))
			{
				FPendingContents.append(content);
				return content;
			}
			else
				session->deleteContent(AName);
		}
	}
	return NULL;
}

IJingle::SessionStatus Jingle::sessionStatus(const Jid &AStreamJid, const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->status():IJingle::None;
}

bool Jingle::isOutgoing(const Jid &AStreamJid, const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->isOutgoing():false;
}

Jid Jingle::contactJid(const Jid &AStreamJid, const QString &ASid) const
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	return session?session->otherParty():Jid();
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

QHash<QString, IJingleContent *> Jingle::contents(const Jid &AStreamJid, const QString &ASid) const
{
	QHash<QString, IJingleContent *> rc;
	JingleSession *session = JingleSession::sessionBySessionId(AStreamJid, ASid);
	if (session)
		for (QHash<QString, JingleContent *>::ConstIterator it=session->contents().constBegin(); it!=session->contents().constEnd(); it++)
			rc.insert(it.key(), *it);
	return rc;
}

IJingleContent *Jingle::content(const Jid &AStreamJid, const QString &ASid, const QString &AName) const
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	IJingleContent *content = session?session->getContent(AName):NULL;
	return content;
}

bool Jingle::selectTransportCandidate(const Jid &AStreamJid, const QString &ASid, const QString &AContentName, const QString &ACandidateId)
{
	JingleSession *session=JingleSession::sessionBySessionId(AStreamJid, ASid);
	if (session)
		return session->selectTransportCandidate(AContentName, ACandidateId);
	return false;
}

bool Jingle::connectContent(const Jid &AStreamJid, const QString &ASid, const QString &AName)
{
	JingleSession *session = JingleSession::sessionBySessionId(AStreamJid, ASid);
	if (session)
	{
		IJingleContent *content =  session->getContent(AName);
		if (content)
			return FTransports[content->transportNS()]->openConnection(content);
		else
			qWarning() << "No content!";
	}
	else
		qWarning() << "No session!";
	return false;
}

bool Jingle::setConnected(const Jid &AStreamJid, const QString &ASid)
{
	JingleSession *session = JingleSession::sessionBySessionId(AStreamJid, ASid);
	if (session)
	{
		session->setConnected();
		return true;
	}
	return false;
}

bool Jingle::fillIncomingTransport(IJingleContent *AContent)
{
	return FTransports[AContent->transportNS()]->fillIncomingTransport(AContent);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_Jingle,Jingle)
#endif

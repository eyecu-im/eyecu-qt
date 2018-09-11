#include <QDebug>
#include <QNetworkInterface>
#include <QpLog>
#include <definitions/version.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <utils/logger.h>

#include "jingletransporticeudp.h"
#include "icethread.h"
#include "iceoptions.h"

JingleTransportIceUdp::JingleTransportIceUdp(QObject *parent) :
	QObject(parent),
	FOptionsManager(nullptr),
	FServiceDiscovery(nullptr),
	FJingle(nullptr)
{
}

void JingleTransportIceUdp::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Jingle ICE-UDP Transport");
    APluginInfo->description = tr("Implements XEP-0176: Jingle ICE-UDP transport Method");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(JINGLE_UUID);
}

bool JingleTransportIceUdp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{    
    Q_UNUSED(AInitOrder);

	IPlugin *plugin= APluginManager->pluginInterface("IJingle").value(0, nullptr);
	if (plugin)
		FJingle = qobject_cast<IJingle *>(plugin->instance());
	else
		return false;

	plugin= APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, nullptr);
    if (plugin)
        FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),
								SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(OptionsNode)),
								SLOT(onOptionsChanged(OptionsNode)));

    return true;
}

bool JingleTransportIceUdp::initObjects()
{

    if (FServiceDiscovery)
        registerDiscoFeatures();

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

	FIceCfg.stunConfig.softwareName = QString("%1 %2").arg(CLIENT_NAME).arg(CLIENT_VERSION_FULL);

	qpIceInit();
	QpLog::setLevel(1);

    return true;
}

bool JingleTransportIceUdp::initSettings()
{
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_ICE_AGGRESSIVE, false);
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_ICE_SERVERS_STUN,
							 QStringList() << "numb.viagenie.ca:");
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_ICE_SERVERS_TURN,
							 QStringList() << "13.250.13.83::YzYNCouZM1mhqhmseWk6:YzYNCouZM1mhqhmseWk6");
    return true;
}

bool JingleTransportIceUdp::openConnection(IJingleContent *AContent)
{
	IceThread *iceThread(nullptr);
	for (QList<IceThread*>::ConstIterator it=FIceThreads.constBegin();
		 it != FIceThreads.constEnd(); ++it)
		if ((*it)->content() == AContent) {
			iceThread = *it;
			break;
		}

	if (iceThread) {
		if (iceThread->state() == QPIceTransport::StateSessionReady)
		{
			QDomElement outgoingTransport = AContent->transportOutgoing();
			if (!outgoingTransport.hasAttribute("ufrag") ||
				!outgoingTransport.hasAttribute("pwd")) {
				LOG_ERROR("Broket <transport/> element! returning false!");
				return false;
			}

			QHash<QString, QPIceCandidate> candidates;
			for (QDomElement candidate=outgoingTransport.firstChildElement("candidate");
				 !candidate.isNull();
				 candidate=candidate.nextSiblingElement("candidate"))
				if (candidate.hasAttribute("id") &&
					candidate.hasAttribute("component") &&
					candidate.hasAttribute("type") &&
					candidate.hasAttribute("foundation") &&
					candidate.hasAttribute("ip") &&
					candidate.hasAttribute("port") &&
					candidate.hasAttribute("priority"))
				{
					QPIceCandidate cand;
					cand.addr = QPSocketAddress(candidate.attribute("ip"),
											quint16(candidate.attribute("port").toInt()));
					cand.type = QPIceCandidate::getTypeByName(candidate.attribute("type"));
					if (cand.type == QPIceCandidate::Host)
						cand.relAddr = cand.addr;
					else if (candidate.hasAttribute("rel-addr") &&
							 candidate.hasAttribute("rel-port"))
						cand.relAddr = QPSocketAddress(candidate.attribute("rel-addr"),
													   quint16(candidate.attribute("rel-port").toInt()));
					else {
						LOG_ERROR(QString("Invalid candidate! "
										  "No related address specified for type %1. "
										  "Skipped.").arg(candidate.attribute("type")));
						continue;
					}

					int network = candidate.attribute("network").toInt(); // Don't need it right now
					Q_UNUSED(network)

					QString id = candidate.attribute("id");
					cand.componentId = quint8(candidate.attribute("component").toInt());
					cand.foundation = candidate.attribute("foundation");

					cand.prio = unsigned(candidate.attribute("priority").toInt());
					cand.transportId = 0; // Not sure if we need it
					cand.status = QP_NO_ERROR;
					cand.localPreference = 0;
					cand.setBaseAddr();
					candidates.insert(id, cand);
				} else
					LOG_WARNING("Invalid candidate! Skipped.");

			if (candidates.isEmpty())
				LOG_ERROR("Remote candidate list is empty!");
			else {
				// Look for an approprite thread
				QString remoteUfrag = outgoingTransport.attribute("ufrag");
				QString remotePwd = outgoingTransport.attribute("pwd");
				if (iceThread->startIce(remoteUfrag, remotePwd.toLatin1(), candidates))
					return true;
				else
					LOG_ERROR("ICE session negotiation start failed!");
			}
		}
		else
			LOG_ERROR(QString("ICE thread is in invalid state: %1")
					  .arg(QPIceTransport::stateName(iceThread->state())));
	}
	else
		qCritical() << "ICE thread for the content not found!";

    return false;
}

bool JingleTransportIceUdp::fillIncomingTransport(IJingleContent *AContent)
{
	QPIceSession::Role role = FJingle->isOutgoing(AContent->sid())?
				QPIceSession::Controlling:QPIceSession::Controlled;

	IceThread *iceThread = new IceThread(FIceCfg, role, AContent);
	connect(iceThread, SIGNAL(iceSuccess(int)), SLOT(onIceSuccess(int)));
	connect(iceThread, SIGNAL(finished()), SLOT(onIceThreadFinished()));
	FIceThreads.append(iceThread);

	iceThread->start();

	return true;
}

void JingleTransportIceUdp::freeIncomingTransport(IJingleContent *AContent)
{
	IceThread *iceThread(nullptr);
	for (QList<IceThread *>::ConstIterator it=FIceThreads.constBegin();
		 it != FIceThreads.constEnd(); ++it)
		if ((*it)->content() == AContent)
		{
			iceThread = *it;
			break;
		}
	if (iceThread)
	{
		FIceThreads.removeOne(iceThread);
		if (iceThread->state() == QPIceTransport::StateRunning) { // Close all components
			int count = iceThread->componentCount();
			for (int i=1; i<= count; ++i)
				iceThread->component(i)->close();
		}
		iceThread->destroy();	// Terminate ICE
	}
}

QMultiMap<int, IOptionsDialogWidget *> JingleTransportIceUdp::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_JINGLETRANSPORTS)
	{
		widgets.insertMulti(OHO_JINGLETRANSPORTS_ICE,
							FOptionsManager->newOptionsDialogHeader(tr("ICE"), AParent));
		widgets.insertMulti(OWO_JINGLETRANSPORTS_ICE, new IceOptions(AParent));
	}
	return widgets;
}

void JingleTransportIceUdp::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.active = true;
    dfeature.var = NS_JINGLE_TRANSPORTS_ICE_UDP;
//    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_JINGLE_RTP);
    dfeature.name = tr("Jingle ICE-UDP Transport");
    dfeature.description = tr("Allows using ICE-UDP transport in Jingle sesions");
	FServiceDiscovery->insertDiscoFeature(dfeature);
}

int JingleTransportIceUdp::readCandidates(IceThread *AIceThread)
{
	QHash<QHostAddress, int> networkByIp = networksByIp();

	// Enumerate local candidates
	int compCnt = AIceThread->componentCount();
	QHash<QString,QPIceCandidate> cand;
	for (int i=0; i<compCnt; ++i) {
		int status = AIceThread->enumCandidates(i+1, cand);
		if (status != QP_NO_ERROR)
			return status;
	}

	// Need to calculate "network" attribute
	QHash<QHostAddress,QHostAddress> relAddrByAddr;
	for (QHash<QString,QPIceCandidate>::ConstIterator it=cand.constBegin();
		 it!=cand.constEnd(); ++it)
		if (static_cast<QHostAddress>(it->addr) !=
			static_cast<QHostAddress>(it->relAddr))
			relAddrByAddr.insert(it->addr, it->relAddr);

	QDomElement incomingTransport = AIceThread->content()->transportIncoming();
	incomingTransport.setAttribute("ufrag", AIceThread->localUfrag());
	incomingTransport.setAttribute("pwd", AIceThread->localPwd());

	for (QHash<QString,QPIceCandidate>::ConstIterator it=cand.constBegin();
		 it!=cand.constEnd(); ++it)
	{
		int network=-1;

		// Calculate "network" attribute
		for(QHostAddress relAddr = it->relAddr;;) {
			if (networkByIp.contains(relAddr)) {
				network = networkByIp[relAddr];
				break;
			} else {
				if (relAddrByAddr.contains(relAddr))
					relAddr = relAddrByAddr[relAddr];
				else
					break;
			}
		}

//		Q_ASSERT(network!=-1);

		QDomElement candidate = incomingTransport.ownerDocument().createElement("candidate");
		candidate.setAttribute("protocol", "udp");
		candidate.setAttribute("component", QString::number(it->componentId));
		candidate.setAttribute("type", it->getTypeName());
		candidate.setAttribute("foundation", it->foundation);
		candidate.setAttribute("generation", "0");
		candidate.setAttribute("id", it.key());
		candidate.setAttribute("network", QString::number(network));
		candidate.setAttribute("ip", it->addr.toString());
		candidate.setAttribute("port", QString::number(it->addr.port()));
		candidate.setAttribute("priority", QString::number(it->prio));
		if (it->type != QPIceCandidate::Host) {
			candidate.setAttribute("rel-addr", it->relAddr.toString());
			candidate.setAttribute("rel-port", QString::number(it->relAddr.port()));
		}
		incomingTransport.appendChild(candidate);
	}

	return QP_NO_ERROR;
}

QHash<QHostAddress, int> JingleTransportIceUdp::networksByIp()
{
	QHash<QHostAddress, int> networkByIp;
	int i=0;
	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	for (QList<QNetworkInterface>::ConstIterator it=interfaces.constBegin();
		 it!=interfaces.constEnd(); ++it) {
		QList<QNetworkAddressEntry> addresses = it->addressEntries();
		for (QList<QNetworkAddressEntry>::ConstIterator ita=addresses.constBegin();
			 ita!=addresses.constEnd(); ++ita) {
			networkByIp.insert(ita->ip(), i);
		}
		++i;
	}
	return networkByIp;
}

void JingleTransportIceUdp::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_JINGLE_TRANSPORT_ICE_AGGRESSIVE));
	onOptionsChanged(Options::node(OPV_JINGLE_TRANSPORT_ICE_SERVERS_STUN));
	onOptionsChanged(Options::node(OPV_JINGLE_TRANSPORT_ICE_SERVERS_TURN));
}

void JingleTransportIceUdp::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path()==OPV_JINGLE_TRANSPORT_ICE_AGGRESSIVE) // Aggressive nomination
		FIceCfg.options.aggressive = ANode.value().toBool();
	else if (ANode.path()==OPV_JINGLE_TRANSPORT_ICE_SERVERS_STUN) // STUN servers
	{
		FIceCfg.stunTransportCfg.clear();
		QStringList servers = ANode.value().toStringList();
		for (QStringList::ConstIterator it=servers.constBegin();
			 it!=servers.constEnd(); ++it) {
			QStringList parts = (*it).split(':');
			if (parts.size() == 2)
			{
				QPIceTransport::StunConfig stunCfg;
				stunCfg.server = parts[0];
				if (parts[1].isEmpty())
					stunCfg.port = QP_STUN_PORT;
				else
				{
					bool ok;
					stunCfg.port = quint16(parts[1].toInt(&ok));
					if (!ok || !stunCfg.port)
						LOG_ERROR("Invalid port number!");
				}

				FIceCfg.stunTransportCfg.append(stunCfg);
			}
			else
				LOG_ERROR("Invalid STUN server record!");
		}
	}
	else if (ANode.path()==OPV_JINGLE_TRANSPORT_ICE_SERVERS_TURN) // TURN servers
	{
		FIceCfg.turnTransportCfg.clear();
		QStringList servers = ANode.value().toStringList();
		for (QStringList::ConstIterator it=servers.constBegin();
			 it!=servers.constEnd(); ++it) {
			QStringList parts = (*it).split(':');
			if (parts.size() == 4) {
				QPIceTransport::TurnConfig turnCfg;

				turnCfg.server = parts[0];
				if (parts[1].isEmpty())
					turnCfg.port = QP_STUN_PORT;
				else
				{
					bool ok;
					turnCfg.port = quint16(parts[1].toInt(&ok));
					if (!ok || !turnCfg.port)
						LOG_ERROR("Invalid port number!");
				}
				turnCfg.authCredential = QPStunAuthCred(turnCfg.server, parts[2],
														QPStunAuthCred::PasswordPlain,
														parts[3].toLatin1());
				FIceCfg.turnTransportCfg.append(turnCfg);
			}
			else
				LOG_ERROR("Invalid TURN server record!");
		}
	}
}

void JingleTransportIceUdp::onIceSuccess(int AOperation)
{
	LOG_DEBUG(QString("JingleTransportIceUdp::onIceSuccess(%1)").arg(AOperation));
	IceThread *iceThread = qobject_cast<IceThread*>(sender());
	IJingleContent *content = iceThread->content();
	switch (AOperation)
	{
		case QPIceTransport::OperationInit:
			if (readCandidates(iceThread) == QP_NO_ERROR)
				emit incomingTransportFilled(content);
			else
				emit incomingTransportFillFailed(content);
			break;

		case QPIceTransport::OperationNegotiation:
		{
			int count = iceThread->componentCount();
			content->setComponentCount(count);
			for (int i=1; i<=count; ++i) {
				QPIceComponent *comp = iceThread->component(i);
				comp->open(QIODevice::ReadOnly|QIODevice::WriteOnly);
				content->setIoDevice(i, comp);
			}
			emit connectionOpened(content);
			break;
		}

		case QPIceTransport::OperationAddressChange:
			LOG_INFO("Address changed");
			break;

		case QPIceTransport::OperationKeepAlive:
			LOG_INFO("Keep-Alive");
			break;
	}
}

void JingleTransportIceUdp::onIceThreadFinished()
{
	IceThread *iceThread = qobject_cast<IceThread*>(sender());
	if (FIceThreads.contains(iceThread)) {
		LOG_DEBUG("ICE thread unexpectedly finished!");
		FIceThreads.removeOne(iceThread);
		emit incomingTransportFillFailed(iceThread->content());
		delete iceThread;
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportIceUdp,JingleTransportIceUdp)
#endif

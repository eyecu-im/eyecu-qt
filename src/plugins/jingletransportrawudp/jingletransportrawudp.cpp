#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QpUtil>
#include <utils/options.h>
#include <utils/logger.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>

#include "jingletransportrawudp.h"
#include "rawudpoptions.h"

JingleTransportRawUdp::JingleTransportRawUdp(QObject *AParent) :
	QObject(AParent),
	FJingle(nullptr),
	FServiceDiscovery(nullptr),
	FOptionsManager(nullptr)
{}

void JingleTransportRawUdp::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Jingle RAW-UDP Transport");
    APluginInfo->description = tr("Implements XEP-0177: Jingle RAW-UDP transport method");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(JINGLE_UUID);
}

bool JingleTransportRawUdp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

	IPlugin *plugin= APluginManager->pluginInterface("IJingle").value(0, nullptr);
    if (plugin)
        FJingle = qobject_cast<IJingle *>(plugin->instance());
    else
        return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0, nullptr);
    if (plugin)
        FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    //AInitOrder = 200;   // This one should be initialized AFTER ...
    return true;
}

bool JingleTransportRawUdp::initObjects()
{
    if (FServiceDiscovery)
        registerDiscoFeatures();

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

    return true;
}

bool JingleTransportRawUdp::initSettings()
{
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_RAWUDP_IP, QVariant());
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST, 6666);
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST, 8888);
    return true;
}

bool JingleTransportRawUdp::openConnection(IJingleContent *AContent)
{
	qDebug() << "JingleTransportRawUdp::openConnection(" << AContent << ")";
    int candidates = 0; // Total candidates
    int successful = 0; // Successfuly connected candidates
	int compCnt = AContent->componentCount();

	QSet<int> comps;
    QDomElement incomingTransport = AContent->transportIncoming();
    for (QDomElement candidate = incomingTransport.firstChildElement("candidate"); !candidate.isNull(); candidate=candidate.nextSiblingElement("candidate"))
        if (candidate.hasAttribute("id") &&
            candidate.hasAttribute("ip") &&
            candidate.hasAttribute("port"))
        {
            candidates++;
//			QString id = candidate.attribute("id");
			int comp = candidate.attribute("component").toInt();
			if (comp < 1 || comp > compCnt)
			{
				LOG_WARNING("Invalid component!");
				continue;
			}

			if (comps.contains(comp))
			{
				LOG_WARNING("Candidate for the component exists already!");
				continue;
			}
			comps.insert(comp);
			successful++;
            QHostAddress address(candidate.attribute("ip"));
			quint16 port = static_cast<quint16>(candidate.attribute("port").toInt());
			QUdpSocket *socket = qobject_cast<QUdpSocket *>(AContent->inputDevice(comp));
            if (!socket)
			{
				AContent->setInputDevice(comp, socket = new QUdpSocket(this));
				socket->setProxy(QNetworkProxy::NoProxy);
			}
            if (socket->state() != QUdpSocket::UnconnectedState &&
               (socket->state() != QUdpSocket::BoundState ||
                socket->localAddress() != address ||
                socket->localPort() != port))
                socket->disconnectFromHost();

            if (socket->state() == QUdpSocket::UnconnectedState)
				if (!socket->bind(address, port, QUdpSocket::DontShareAddress))
					LOG_WARNING(QString("Failed to bind input socket on %1:%2").arg(address.toString()).arg(port));

			if (socket->state() != QUdpSocket::BoundState)
			{
				AContent->setInputDevice(comp, nullptr); // Remove broken socket
				successful--;
			}
        }
        else
			LOG_WARNING("Incoming candidate is broken!");

    if (!candidates)
    {
		LOG_FATAL("No input candidates found!");
        return false;
    }

    if (candidates > successful)
    {
		LOG_WARNING("Some input candidates are failed!");
        return false;
    }

	if (comps.size() != AContent->componentCount())
	{
		LOG_WARNING("Component count for the content do not match!");
		return false;
	}

	comps.clear();
    QDomElement outgoingTransport = AContent->transportOutgoing();
	for (QDomElement candidate = outgoingTransport.firstChildElement("candidate");
		 !candidate.isNull(); candidate=candidate.nextSiblingElement("candidate"))
        if (candidate.hasAttribute("id") &&
            candidate.hasAttribute("ip") &&
            candidate.hasAttribute("port"))
        {
//			QString id = candidate.attribute("id");
			int comp = candidate.attribute("component").toInt();
			if (comp < 1 || comp > compCnt)
			{
				LOG_WARNING("Invalid component!");
				continue;
			}

			if (comps.contains(comp))
			{
				LOG_WARNING("Candidate for the component exists already!");
				continue;
			}
			comps.insert(comp);
            QHostAddress address(candidate.attribute("ip"));
			quint16 port = static_cast<quint16>(candidate.attribute("port").toInt());
			QUdpSocket *socket = qobject_cast<QUdpSocket *>(AContent->outputDevice(comp));
            if (!socket)
			{
				AContent->setOutputDevice(comp, socket = new QUdpSocket(this));
				socket->setProxy(QNetworkProxy::NoProxy);
			}

            if (socket->state() != QUdpSocket::UnconnectedState &&
               (socket->state() != QUdpSocket::ConnectedState ||
                socket->peerAddress() != address ||
                socket->peerPort() != port))
                socket->disconnectFromHost();

            if (socket->state() == QUdpSocket::UnconnectedState)
                socket->connectToHost(address, port, QIODevice::WriteOnly|QIODevice::Unbuffered);

            if (socket->state() == QUdpSocket::ConnectedState &&
                socket->openMode() == (QIODevice::WriteOnly|QIODevice::Unbuffered))
                continue;

            qWarning() << "Failed to connect output socket!";
			AContent->setOutputDevice(comp, nullptr); // Remove broken socket
        }
        else
            qWarning() << "Incoming candidate is broken!";

    if (!candidates)
    {
        qWarning() << "No output candidates found!";
        return false;
    }

    if (candidates > successful)
    {
        qWarning() << "Some output candidates are failed!";
        return false;
    }

	if (comps.size() != compCnt)
	{
		qWarning() << "Wrong number of components!";
		return false;
	}

	emit connectionOpened(AContent);
	qDebug() << "JingleTransportRawUdp::openConnection(): return true";
    return true;
}

bool JingleTransportRawUdp::fillIncomingTransport(IJingleContent *AContent)
{
    QUdpSocket *socket = new QUdpSocket(this);
	socket->setProxy(QNetworkProxy::NoProxy);
    QHostAddress localAddress;

    QDomElement outgoingTransport = AContent->transportOutgoing();
    for (QDomElement candidate = outgoingTransport.firstChildElement("candidate");
         !candidate.isNull();
         candidate = candidate.nextSiblingElement("candidate"))
    {
		if (candidate.hasAttribute("ip") && candidate.hasAttribute("port") &&
			candidate.hasAttribute("component") && candidate.hasAttribute("generation") &&
			candidate.hasAttribute("id"))
        {
			QHostAddress targetAddress(candidate.attribute("ip"));
			quint16 port = static_cast<quint16>(candidate.attribute("port").toUInt());
			socket->connectToHost(targetAddress, port,
                                  QIODevice::ReadOnly|QIODevice::Unbuffered);			
            localAddress =  socket->localAddress();

			socket->disconnectFromHost();
			socket->close();
        }
        else
            qWarning() << "Candidate is broken!";
        break;
    }

	delete socket;

	if (localAddress.isNull()) {
		QVariant addr = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).value();
		if (addr.isNull())
			localAddress = QpUtil::getHostIp(QAbstractSocket::UnknownNetworkLayerProtocol);
		else
			localAddress.setAddress(addr.toString());
	}

	if (!localAddress.isNull())
	{
		for (int component=1; component<=AContent->componentCount(); ++component)
		{
			socket = getSocket(localAddress);
			if (socket)
			{
				int id=100;
				QString candidateId=QString("id%1").arg(id);
				QDomElement incomingTransport = AContent->transportIncoming();
				if (!incomingTransport.isNull())
				{
					QDomElement candidate=incomingTransport.ownerDocument().createElement("candidate");
					QString ip = socket->localAddress().toString();
					QString port = QString::number(socket->localPort());
					incomingTransport.appendChild(candidate);
					candidate.setAttribute("component", QString().setNum(component));
					candidate.setAttribute("generation", 0);
					candidate.setAttribute("id", candidateId);
					candidate.setAttribute("ip", ip);
					candidate.setAttribute("port", port);
					LOG_INFO(QString("About to set input device: QUdpSocket(%1:%2)").arg(socket->localAddress().toString()).arg(socket->localPort()));
					AContent->setInputDevice(component, socket);
				}
				else
				{
					qWarning() << "Candidate 1 is in wrong state!";
					socket->deleteLater();
					return false;
				}
			}
			else
				qWarning() << "Failed to get a local socket!";

			qDebug() << "emitting incomingTransportFilled(" << AContent << ")";
			emit incomingTransportFilled(AContent);
			return true;
		}
	}
	else
		qWarning() << "Failed to determine local address!";
	return false;
}

//TODO: Get rid of it
void JingleTransportRawUdp::freeIncomingTransport(IJingleContent *AContent)
{
//	QStringList candidateIds = AContent->candidateIds();
//	for (QStringList::ConstIterator it=candidateIds.constBegin();
//		 it!=candidateIds.constEnd(); ++it)
	int compCnt = AContent->componentCount();
	for (int comp = 1; comp <= compCnt; ++comp)
		AContent->setInputDevice(comp, nullptr);
}

QMultiMap<int, IOptionsDialogWidget *> JingleTransportRawUdp::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_JINGLETRANSPORTS)
	{
		widgets.insertMulti(OHO_JINGLETRANSPORTS_RAWUDP, FOptionsManager->newOptionsDialogHeader(tr("Raw UDP"), AParent));
		widgets.insertMulti(OWO_JINGLETRANSPORTS_RAWUDP, new RawUdpOptions(AParent));
	}
	return widgets;
}

void JingleTransportRawUdp::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.active = true;
    dfeature.var = NS_JINGLE_TRANSPORTS_RAW_UDP;
//	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_JINGLE_RTP);
    dfeature.name = tr("Jingle RAW-UDP Transport");
    dfeature.description = tr("Allows using RAW-UDP transport in Jingle sesions");
	FServiceDiscovery->insertDiscoFeature(dfeature);
}

QUdpSocket *JingleTransportRawUdp::getSocket(const QHostAddress &ALocalAddress)
{
	qDebug() << "JingleTransportRawUdp::getSocket(" << ALocalAddress << ")";
	QUdpSocket *socket = new QUdpSocket(this);
	socket->setProxy(QNetworkProxy::NoProxy);
	quint16 first = static_cast<quint16>(
				Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).value().toUInt());
	quint16 last = static_cast<quint16>(
				Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).value().toUInt());
	for (quint16 port = first; port<=last; port++)
		if (!FPorts.contains(port) && !FPorts.contains(port+1))
		{
			if (socket->bind(ALocalAddress, port, QUdpSocket::DontShareAddress))
			{
				FPorts.insert(port, socket);
				connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
								SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
				return socket;
			}
			else
				qWarning() << "Failed to bind socket!";
		}
	delete socket;
	return nullptr;
}

void JingleTransportRawUdp::onSocketStateChanged(QAbstractSocket::SocketState ASocketState)
{
	qDebug() << "JingleTransportRawUdp::onSocketStateChanged(" << ASocketState << ")";
	if (ASocketState != QAbstractSocket::BoundState)
	{
		QUdpSocket *socket = qobject_cast<QUdpSocket *>(sender());
		if (socket)
		{
			QList<quint16> ports = FPorts.keys(socket);
			for (QList<quint16>::ConstIterator it = ports.constBegin(); it != ports.constEnd(); ++it)
				FPorts.remove(*it);
		}
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportRawUdp,JingleTransportRawUdp)
#endif

#include <QNetworkInterface>
#include <QNetworkProxy>
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
	FJingle(NULL),
	FServiceDiscovery(NULL),
	FOptionsManager(NULL)
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

    IPlugin *plugin= APluginManager->pluginInterface("IJingle").value(0,NULL);
    if (plugin)
        FJingle = qobject_cast<IJingle *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
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
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_RAWUDP_IP, QNetworkInterface::allAddresses().first().toString());
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST, 6666);
	Options::setDefaultValue(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST, 8888);
    return true;
}

bool JingleTransportRawUdp::openConnection(IJingleContent *AContent)
{
	qDebug() << "JingleTransportRawUdp::openConnection(" << AContent << ")";
    int candidates = 0; // Total candidates
    int successful = 0; // Successfuly connected candidates

    QDomElement incomingTransport = AContent->transportIncoming();
    for (QDomElement candidate = incomingTransport.firstChildElement("candidate"); !candidate.isNull(); candidate=candidate.nextSiblingElement("candidate"))
        if (candidate.hasAttribute("id") &&
            candidate.hasAttribute("ip") &&
            candidate.hasAttribute("port"))
        {
            candidates++;
            successful++;
            QString id = candidate.attribute("id");
            QHostAddress address(candidate.attribute("ip"));
            int port = candidate.attribute("port").toInt();
            QUdpSocket *socket = qobject_cast<QUdpSocket *>(AContent->inputDevice(id));
            if (!socket)
			{
                AContent->setInputDevice(id, socket = new QUdpSocket(this));
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
				AContent->setInputDevice(id, NULL); // Remove broken socket
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

    QDomElement outgoingTransport = AContent->transportOutgoing();
    for (QDomElement candidate = outgoingTransport.firstChildElement("candidate"); !candidate.isNull(); candidate=candidate.nextSiblingElement("candidate"))
        if (candidate.hasAttribute("id") &&
            candidate.hasAttribute("ip") &&
            candidate.hasAttribute("port"))
        {
            QString id = candidate.attribute("id");
            QHostAddress address(candidate.attribute("ip"));
            int port = candidate.attribute("port").toInt();
            QUdpSocket *socket = qobject_cast<QUdpSocket *>(AContent->outputDevice(id));
			qDebug() << "socket=" << socket;
            if (!socket)
			{
                AContent->setOutputDevice(id, socket = new QUdpSocket(this));
				socket->setProxy(QNetworkProxy::NoProxy);
			}

			qDebug() << "socket->state()=" << socket->state();
			qDebug() << "address=" << address;
			qDebug() << "port=" << port;

			qDebug() << "local address=" << address;
			qDebug() << "port=" << port;

            if (socket->state() != QUdpSocket::UnconnectedState &&
               (socket->state() != QUdpSocket::ConnectedState ||
                socket->peerAddress() != address ||
                socket->peerPort() != port))
			{
				qDebug() << "A";
                socket->disconnectFromHost();
			}

			qDebug() << "socket->state()=" << socket->state();

            if (socket->state() == QUdpSocket::UnconnectedState)
			{
				qDebug() << "B";
                socket->connectToHost(address, port, QIODevice::WriteOnly|QIODevice::Unbuffered);
			}

            if (socket->state() == QUdpSocket::ConnectedState &&
                socket->openMode() == (QIODevice::WriteOnly|QIODevice::Unbuffered))
			{
				qDebug() << "C";
				qDebug() << "Peer: " << socket->peerAddress() << ":" << socket->peerPort();
				qDebug() << "Local: " << socket->localAddress() << ":" << socket->localPort();
                continue;
			}

            qWarning() << "Failed to connect output socket!";
            AContent->setOutputDevice(id, NULL); // Remove broken socket
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

	emit connectionOpened(AContent);
	qDebug() << "JingleTransportRawUdp::openConnection(): return true";
    return true;
}

bool JingleTransportRawUdp::fillIncomingTransport(IJingleContent *AContent)
{
	qDebug() << "JingleTransportRawUdp::fillIncomingTransport(" << AContent << ")";
    QUdpSocket *socket = new QUdpSocket(this);
	socket->setProxy(QNetworkProxy::NoProxy);
    QHostAddress localAddress;

    QDomElement outgoingTransport = AContent->transportOutgoing();
    for (QDomElement candidate = outgoingTransport.firstChildElement("candidate");
         !candidate.isNull();
         candidate = candidate.nextSiblingElement("candidate"))
    {
		qDebug() << "candidate found!";
        if (candidate.hasAttribute("ip") && candidate.hasAttribute("port") && candidate.hasAttribute("component") && candidate.hasAttribute("generation") && candidate.hasAttribute("id"))
        {
            socket->connectToHost(QHostAddress(candidate.attribute("ip")),
                                  candidate.attribute("port").toUInt(),
                                  QIODevice::ReadOnly|QIODevice::Unbuffered);
            localAddress =  socket->localAddress();
			qDebug() << "localAddress=" << localAddress;
			socket->disconnectFromHost();
			socket->close();
        }
        else
            qWarning() << "Candidate is broken!";
        break;
    }

	delete socket;

    if (localAddress.isNull())
		localAddress = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).value().toString();

	if (!localAddress.isNull())
	{
		socket = getSocket(localAddress);
		qDebug() << "Got socket:" << socket;
		if (socket)
        {
			qDebug() << "socket is bound:" << socket;
			qDebug() << "Host:" << socket->localAddress();
			qDebug() << "Port:" << socket->localPort();

            int component=1;
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
                AContent->setInputDevice(candidateId, socket);
            }
            else
            {
                qWarning() << "Candidate 1 is is wrong state!";
                socket->deleteLater();
				qDebug() << "JingleTransportRawUdp::fillIncomingTransport(): return false(A)";
                return false;
            }
			emit incomingTransportFilled(AContent);
			qDebug() << "JingleTransportRawUdp::fillIncomingTransport(): return true(B)";
			return true;
        }		
		else
			qWarning() << "Failed to get a local socket!";
    }
    else
        qWarning() << "Failed to determine local address!";
	qDebug() << "JingleTransportRawUdp::fillIncomingTransport(): return false(C)";
	return false;
}

//TODO: Get rid of it
void JingleTransportRawUdp::freeIncomingTransport(IJingleContent *AContent)
{
	qDebug() << "JingleTransportRawUdp::freeIncomingTransport(" << AContent << ")";
	QStringList candidateIds = AContent->candidateIds();
	qDebug() << "candidateIds=" << candidateIds;
	for (QStringList::ConstIterator it=candidateIds.constBegin(); it!=candidateIds.constEnd(); ++it)
		AContent->setInputDevice(*it, NULL);
	qDebug() << "JingleTransportRawUdp::freeIncomingTransport(): done!";
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
	qDebug() << "JingleTransportRawUdp::getPort(" << ALocalAddress << ")";
	QUdpSocket *socket = new QUdpSocket(this);
	socket->setProxy(QNetworkProxy::NoProxy);
	int first = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).value().toInt();
	int last = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).value().toInt();
	for (quint16 port = first; port<=last; port+=2)
		if (!FPorts.contains(port) && !FPorts.contains(port+1))
		{
			if (socket->bind(ALocalAddress, port, QUdpSocket::DontShareAddress))
			{
				FPorts.insert(port, socket);
				FPorts.insert(port+1, socket);
				connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
				return socket;
			}
			else
				qWarning() << "Failed to bind socket!";
		}
	delete socket;
	qDebug() << "returning NULL";
	return NULL;
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
			{
				qDebug() << "freeing port:" << *it;
				FPorts.remove(*it);
			}
		}
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportRawUdp,JingleTransportRawUdp)
#endif

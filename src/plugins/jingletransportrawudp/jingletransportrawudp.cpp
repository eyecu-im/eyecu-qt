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
#include "rawudpiodevice.h"

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
	for (QDomElement candidate = incomingTransport.firstChildElement("candidate");
		 !candidate.isNull(); candidate=candidate.nextSiblingElement("candidate"))
        if (candidate.hasAttribute("id") &&
            candidate.hasAttribute("ip") &&
            candidate.hasAttribute("port"))
        {
            candidates++;
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
			RawUdpIODevice *device = qobject_cast<RawUdpIODevice *>(AContent->ioDevice(comp));
			QUdpSocket *socket = device?device->socket():nullptr;

            if (!socket)
			{
				qDebug() << "NO input socket for comp" << comp << "found! Creating a new one...";
				socket = new QUdpSocket();
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
				AContent->setIoDevice(comp, nullptr); // Remove broken socket
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
			RawUdpIODevice *device = qobject_cast<RawUdpIODevice *>(AContent->ioDevice(comp));
			if (device)
			{
				device->setTargetAddress(address, port);
				device->open(QIODevice::ReadOnly|QIODevice::WriteOnly);
				continue;
			}

			qWarning() << "NO I/O device for the component!";
			AContent->setIoDevice(comp, nullptr); // Remove broken I/O device
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

	for (QSet<int>::ConstIterator it = comps.constBegin();
		 it != comps.constEnd(); ++it)
	{
		QIODevice *device = AContent->ioDevice(*it);
		Q_ASSERT(device);
		if (!device->bytesAvailable())
		{
			FPendingContents.insert(device, AContent);
			connect(device, SIGNAL(readyRead()), SLOT(onReadyRead()));
			QTimer::singleShot(1000, this, SLOT(onTimeout()));
		}
	}

	if (!FPendingContents.keys(AContent).isEmpty()) // Have pending content
	{
		QTimer *timer = new QTimer();
		timer->setSingleShot(true);
		connect(timer, SIGNAL(timeout()), SLOT(onTimeout()));
		FPendingTimers.insert(timer, AContent);
		timer->start(1000);	// 1 second timeout
	}

	emit connectionOpened(AContent);
    return true;
}

bool JingleTransportRawUdp::fillIncomingTransport(IJingleContent *AContent)
{
	qDebug() << "JingleTransportRawUdp::fillIncomingTransport()";

	QUdpSocket *socket = new QUdpSocket();
	socket->setProxy(QNetworkProxy::NoProxy);
    QHostAddress localAddress;

    QDomElement outgoingTransport = AContent->transportOutgoing();
    for (QDomElement candidate = outgoingTransport.firstChildElement("candidate");
		 !candidate.isNull(); candidate = candidate.nextSiblingElement("candidate"))
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
		quint16 port(0);
		for (int component=1; component<=AContent->componentCount(); ++component)
		{
			socket = getSocket(localAddress, port);
			if (socket)
			{
				int id=100;
				QString candidateId=QString("id%1").arg(id);
				QDomElement incomingTransport = AContent->transportIncoming();
				if (!incomingTransport.isNull())
				{
					QDomElement candidate=incomingTransport.ownerDocument().createElement("candidate");
					QString ip = socket->localAddress().toString();
					port = socket->localPort();
					incomingTransport.appendChild(candidate);
					candidate.setAttribute("component", QString().setNum(component));
					candidate.setAttribute("generation", 0);
					candidate.setAttribute("id", candidateId);
					candidate.setAttribute("ip", ip);
					candidate.setAttribute("port", QString::number(port));
					LOG_INFO(QString("About to set input device for local socket %1:%2")
							 .arg(socket->localAddress().toString()).arg(socket->localPort()));
					AContent->setIoDevice(component, new RawUdpIODevice(socket, nullptr));
					++port;
				}
				else
				{
					qWarning() << "Candidate 1 is in wrong state!";
					socket->deleteLater();
					return false;
				}
			}
			else
			{
				qWarning() << "Failed to get a local socket!";
				return false;
			}
		}

		qDebug() << "emitting incomingTransportFilled(" << AContent << ")";
		emit incomingTransportFilled(AContent);
		return true;
	}
	else
		qWarning() << "Failed to determine local address!";
	return false;
}

//TODO: Get rid of it
void JingleTransportRawUdp::freeIncomingTransport(IJingleContent *AContent)
{
	int compCnt = AContent->componentCount();
	for (int comp = 1; comp <= compCnt; ++comp)
		AContent->setIoDevice(comp, nullptr);
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

QUdpSocket *JingleTransportRawUdp::getSocket(const QHostAddress &ALocalAddress,
											 quint16 AFirst)
{
	QUdpSocket *socket = new QUdpSocket();
	socket->setProxy(QNetworkProxy::NoProxy); // UDP sockets cannot work with proxies in most cases
	if (!AFirst)
		AFirst = static_cast<quint16>(
				Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).value().toUInt());
	quint16 last = static_cast<quint16>(
				Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).value().toUInt());
	for (quint16 port = AFirst; port<=last; port++)
		if (socket->bind(ALocalAddress, port, QUdpSocket::DontShareAddress))
			return socket;
		else
			qWarning() << "Failed to bind socket!";
	delete socket;
	return nullptr;
}

void JingleTransportRawUdp::onReadyRead()
{
	QIODevice *device = qobject_cast<QIODevice *>(sender());
	if (device)
	{
		device->disconnect(SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		IJingleContent *content = FPendingContents.take(device);
		Q_ASSERT(content);
		if (FPendingContents.keys(content).isEmpty()) // The last content device was removed
		{
			QTimer *timer = FPendingTimers.key(content);
			Q_ASSERT(timer);
			FPendingTimers.remove(timer);
			timer->stop();
			timer->disconnect(SIGNAL(timeout()), this, SLOT(onTimeout()));
			delete timer;
		}
	}
}

void JingleTransportRawUdp::onTimeout()
{
	QTimer *timer = qobject_cast<QTimer *>(sender());
	if (timer)
	{
		IJingleContent *content = FPendingTimers.take(timer);
		delete timer;
		QList<QIODevice *> devices = FPendingContents.keys(content);
		for (QList<QIODevice *>::ConstIterator it = devices.constBegin();
			 it != devices.constEnd(); ++it)
			FPendingContents.remove(*it);

		emit connectionError(content);
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportRawUdp,JingleTransportRawUdp)
#endif

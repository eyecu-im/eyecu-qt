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
			qDebug() << "socket==" << socket;
            if (!socket)
			{
                AContent->setOutputDevice(id, socket = new QUdpSocket(this));
				socket->setProxy(QNetworkProxy::NoProxy);
			}

			qDebug() << "socket->state()=" << socket->state();
			qDebug() << "address=" << address;
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

    if (localAddress.isNull())
		localAddress = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).value().toString();

	if (!localAddress.isNull())
	{
//        QUdpSocket *socket1 = Options::node(OPV_JINGLERTP_USERTCP).value().toBool()?new QUdpSocket(this):NULL;
		while (true)
		{
			quint16 port = getPort();
			if (socket->bind(localAddress, port, QUdpSocket::DontShareAddress))
				break;
//			{
//                if (socket1)
//                {
//                    if (socket1->bind(localAddress, FCurrentPort+1, QUdpSocket::DontShareAddress))
//                        break;
//                    else
//                        socket->disconnectFromHost();
//                }
//                else
//                    break;
//            }
//            FCurrentPort+=2;
//            if (FCurrentPort>PORT_FINISH)
//                FCurrentPort=PORT_START;
		}

//		qDebug() << "socket=" << socket;
//		qDebug() << "socket1=" << socket1;

        if (socket->state()==QAbstractSocket::BoundState)
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
        }

//        if (socket1)
//            if (socket1->state()==QAbstractSocket::BoundState)
//            {
//                qDebug() << "socket1 is bound!";
//                int component=2;
//                int id=101;
//                QString candidateId=QString("id%1").arg(id);
//                QDomElement incomingTransport = AContent->transportIncoming();
//                if (!incomingTransport.isNull())
//                {
//                    QDomElement candidate=incomingTransport.ownerDocument().createElement("candidate");
//                    QString ip = socket1->localAddress().toString();
//                    QString port = QString::number(socket1->localPort());
//                    incomingTransport.appendChild(candidate);
//                    candidate.setAttribute("component", QString().setNum(component));
//                    candidate.setAttribute("generation", 0);
//                    candidate.setAttribute("id", candidateId);
//                    candidate.setAttribute("ip", ip);
//                    candidate.setAttribute("port", port);
//                    AContent->setInputDevice(candidateId, socket1);
//                }
//                else
//                {
//                    qWarning() << "Candidate 2 is is wrong state!";
//                    socket1->deleteLater();
//                    return false;
//                }
//            }

        emit incomingTransportFilled(AContent);
		qDebug() << "JingleTransportRawUdp::fillIncomingTransport(): return true(B)";
        return true;
    }
    else
        qWarning() << "Failed to determine local address!";
	qDebug() << "JingleTransportRawUdp::fillIncomingTransport(): return false(C)";
	return false;
}

void JingleTransportRawUdp::freeIncomingTransport(IJingleContent *AContent)
{
	QStringList candidateIds = AContent->candidateIds();
	for (QStringList::ConstIterator it=candidateIds.constBegin(); it!=candidateIds.constEnd(); ++it)
	{
		QUdpSocket *socket = qobject_cast<QUdpSocket*>(AContent->inputDevice(*it));
		if (socket)
		{
			freePort(socket->localPort());
			AContent->setInputDevice(*it, NULL);
		}
		else
			LOG_ERROR("Input device is NOT a UDP socket!");
	}
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

quint16 JingleTransportRawUdp::getPort()
{
	for (quint16 port = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).value().toInt(); port<=Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).value().toInt(); port+=2)
	{
		if (!FPorts.contains(port) && !FPorts.contains(port+1))
		{
			FPorts.insert(port);
			FPorts.insert(port+1);
			return port;
		}
	}
	return 0;
}

void JingleTransportRawUdp::freePort(quint16 APort)
{
	FPorts.remove(APort);
	FPorts.remove(APort+1);
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportRawUdp,JingleTransportRawUdp)
#endif

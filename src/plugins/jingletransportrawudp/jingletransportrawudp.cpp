#include <QNetworkInterface>
#include <utils/options.h>
#include <definitions/optionvalues.h>

#include "jingletransportrawudp.h"

#define PORT_START  1024
#define PORT_FINISH 49151

JingleTransportRawUdp::JingleTransportRawUdp(QObject *AParent) :
    QObject(AParent),FJingle(NULL),FServiceDiscovery(NULL),FCurrentPort(PORT_START)
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

    //AInitOrder = 200;   // This one should be initialized AFTER ...
    return true;
}

bool JingleTransportRawUdp::initObjects()
{
    if (FServiceDiscovery)
        registerDiscoFeatures();
    return true;
}

bool JingleTransportRawUdp::initSettings()
{
    return true;
}

bool JingleTransportRawUdp::openConnection(IJingleContent *AContent)
{
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
                AContent->setInputDevice(id, socket = new QUdpSocket(this));
            if (socket->state() != QUdpSocket::UnconnectedState &&
               (socket->state() != QUdpSocket::BoundState ||
                socket->localAddress() != address ||
                socket->localPort() != port))
                socket->disconnectFromHost();

            if (socket->state() == QUdpSocket::UnconnectedState)
                socket->bind(address, port, QUdpSocket::DontShareAddress);

            if (socket->state() == QUdpSocket::BoundState)
            {
                if (socket->openMode() == (QIODevice::ReadOnly|QIODevice::Unbuffered))
                    continue;
                else
                    if (socket->open(QIODevice::ReadOnly|QIODevice::Unbuffered))
                        continue;
                qWarning() << "Failed to open socket for writing!";
            }
            else
                qWarning() << "Failed to bind input socket!";
            AContent->setInputDevice(id, NULL); // Remove broken socket
            successful--;
        }
        else
            qWarning() << "Incoming candidate is broken!";

    if (!candidates)
    {
        qWarning() << "No input candidates found!";
        return false;
    }

    if (candidates > successful)
    {
        qWarning() << "Some input candidates are failed!";
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
            if (!socket)
                AContent->setOutputDevice(id, socket = new QUdpSocket(this));
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

	QTimer::singleShot(1000, this, SLOT(onTimeout()));
	emit startSend(AContent);
    return true;
}

bool JingleTransportRawUdp::fillIncomingTransport(IJingleContent *AContent)
{
    QUdpSocket *socket = new QUdpSocket(this);
    QHostAddress localAddress;

    QDomElement outgoingTransport = AContent->transportOutgoing();
    for (QDomElement candidate = outgoingTransport.firstChildElement("candidate");
         !candidate.isNull();
         candidate = candidate.nextSiblingElement("candidate"))
    {
        if (candidate.hasAttribute("ip") && candidate.hasAttribute("port") && candidate.hasAttribute("component") && candidate.hasAttribute("generation") && candidate.hasAttribute("id"))
        {
            socket->connectToHost(QHostAddress(candidate.attribute("ip")),
                                  candidate.attribute("port").toUInt(),
                                  QIODevice::ReadOnly|QIODevice::Unbuffered);
            localAddress =  socket->localAddress();
            socket->disconnectFromHost();            
        }
        else
            qWarning() << "Candidate is broken!";
        break;
    }

    if (localAddress.isNull())
    {
        QList<QHostAddress> addresses=QNetworkInterface::allAddresses();
        for (QList<QHostAddress>::const_iterator it=addresses.constBegin(); it!=addresses.constEnd(); it++)
            if ((*it).toIPv4Address()!=0x7f000001)
                localAddress = *it;
    }

	if (!localAddress.isNull())
	{
//        QUdpSocket *socket1 = Options::node(OPV_JINGLERTP_USERTCP).value().toBool()?new QUdpSocket(this):NULL;
//        while (true)
//        {
//            if (socket->bind(localAddress, FCurrentPort, QUdpSocket::DontShareAddress))
//            {
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
//        }

        qDebug() << "socket=" << socket;
//		qDebug() << "socket1=" << socket1;

        if (socket->state()==QAbstractSocket::BoundState)
        {
            qDebug() << "socket is bound!";
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
                AContent->setInputDevice(candidateId, socket);
            }
            else
            {
                qWarning() << "Candidate 1 is is wrong state!";
                socket->deleteLater();
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
        return true;
    }
    else
        qWarning() << "Failed to determine local address!";
    return false;
}

void JingleTransportRawUdp::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.active = true;
    dfeature.var = NS_JINGLE_TRANSPORTS_RAW_UDP;
//    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_JINGLE_RTP);
    dfeature.name = tr("Jingle RAW-UDP Transport");
    dfeature.description = tr("Allows using RAW-UDP transport in Jingle sesions");
	FServiceDiscovery->insertDiscoFeature(dfeature);
}

void JingleTransportRawUdp::onTimeout()
{
	qDebug() << "JingleTransportRawUdp::onTimeout()";
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportRawUdp,JingleTransportRawUdp)
#endif

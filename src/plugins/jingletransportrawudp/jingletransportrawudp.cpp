#include <QNetworkInterface>
#include <QNetworkProxy>
#include <QThread>
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

bool JingleTransportRawUdp::openConnection(const QString &ASid, const QString &AContentName)
{
	qDebug() << "JingleTransportRawUdp::openConnection(" << ASid << "," << AContentName << ")";
	IJingleContent *content = FJingle->content(ASid, AContentName);
	if (content)
	{
		int candidates = 0; // Total candidates
		int successful = 0; // Successfuly connected candidates
		int compCnt = content->componentCount();

		QSet<int> comps;
		QDomElement incomingTransport = content->transportIncoming();
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

				if (content->ioDevice(comp)) // The device for the component MUST exist
				{
					comps.insert(comp);
					successful++;
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

		if (comps.size() != content->componentCount())
		{
			LOG_WARNING("Component count for the content do not match!");
			return false;
		}

		comps.clear();
		QDomElement outgoingTransport = content->transportOutgoing();
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
				RawUdpIODevice *device = qobject_cast<RawUdpIODevice *>(content->ioDevice(comp));
				if (device)
				{
					device->setTargetAddress(address, port);
					device->open(QIODevice::ReadOnly|QIODevice::WriteOnly);
					continue;
				}

				qWarning() << "NO I/O device for the component!";
				content->setIoDevice(comp, nullptr); // Remove broken I/O device
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


		QPair<QString, QString> cid(ASid, AContentName);

		for (QSet<int>::ConstIterator it = comps.constBegin();
			 it != comps.constEnd(); ++it)
		{
			QIODevice *device = content->ioDevice(*it);
			Q_ASSERT(device);
			if (!device->bytesAvailable())
			{
				FPendingContents.insert(device, cid);
				connect(device, SIGNAL(readyRead()), SLOT(onReadyRead()));
				QTimer::singleShot(1000, this, SLOT(onTimeout()));
			}
		}

		if (!FPendingContents.keys(cid).isEmpty()) // Have pending content
		{
			QTimer *timer = new QTimer();
			timer->setSingleShot(true);
			connect(timer, SIGNAL(timeout()), SLOT(onTimeout()));
			FPendingTimers.insert(timer, cid);
//TODO: Make this timeout configurable
			timer->start(1000);	// 1 second timeout
		}

		emit connectionOpened(ASid, AContentName);
		return true;
	}
	else
		LOG_ERROR(QString("Content not found: %1: %2").arg(ASid).arg(AContentName));

	return false;
}

bool JingleTransportRawUdp::fillIncomingTransport(const QString &ASid, const QString &AContentName)
{
	qDebug() << "JingleTransportRawUdp::fillIncomingTransport()";
	IJingleContent *content = FJingle->content(ASid, AContentName);

	if (content)
	{
		QUdpSocket *socket = new QUdpSocket();
		socket->setProxy(QNetworkProxy::NoProxy);
		QHostAddress localAddress;

		QDomElement outgoingTransport = content->transportOutgoing();
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
			QThread *ioThread = new QThread(this);
			connect(ioThread, SIGNAL(finished()), ioThread, SLOT(deleteLater()));

			QDomElement incomingTransport = content->transportIncoming();
			for (int component=1; component <= content->componentCount(); ++component)
			{
				socket = getSocket(localAddress, port);
				if (socket)
				{
					int id=qrand();
					QString candidateId=QString("id%1").arg(id);
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

						RawUdpIODevice *ioDevice = new RawUdpIODevice(socket, ioThread);
						content->setIoDevice(component, ioDevice);
						++port;
					}
					else
					{
						qWarning() << "Candidate 1 is in wrong state!";
						socket->deleteLater();
						delete ioThread;
						return false;
					}
				}
				else
				{
					qWarning() << "Failed to get a local socket!";
					delete ioThread;
					return false;
				}
			}

			FThreads[ASid].insert(AContentName, ioThread);
			LOG_DEBUG(QString("Starting I/O thread for the content: %1").arg(AContentName));
			ioThread->start();

			FTransportFillNotifications.insertMulti(ASid, AContentName);
			QTimer::singleShot(0, this, SLOT(emitIncomingTransportFilled()));

			return true;
		}
		else
			qWarning() << "Failed to determine local address!";
	}
	else
		qWarning() << "Content do not exist!";

	return false;
}

void JingleTransportRawUdp::freeIncomingTransport(const QString &ASid, const QString &AContentName)
{
	IJingleContent *content = FJingle->content(ASid, AContentName);
	if (content)
	{
		int compCnt = content->componentCount();
		for (int comp = 1; comp <= compCnt; ++comp)
			content->setIoDevice(comp, nullptr);
	}

	if (!FThreads.contains(ASid))
		return;

	Q_ASSERT(FThreads.value(ASid).contains(AContentName));

	if (!FThreads[ASid].contains(AContentName))
		return;

	FThreads[ASid][AContentName]->quit();
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
		Q_ASSERT(FPendingContents.contains(device));
		QPair<QString,QString> cid = FPendingContents.take(device);
		if (FPendingContents.keys(cid).isEmpty()) // The last content device was removed
		{
			QTimer *timer = FPendingTimers.key(cid);
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
		Q_ASSERT(FPendingTimers.contains(timer));
		QPair<QString, QString> content = FPendingTimers.take(timer);
		delete timer;
		QList<QIODevice *> devices = FPendingContents.keys(content);
		for (QList<QIODevice *>::ConstIterator it = devices.constBegin();
			 it != devices.constEnd(); ++it)
			FPendingContents.remove(*it);

		emit connectionError(content.first, content.second);
	}
}

void JingleTransportRawUdp::emitIncomingTransportFilled()
{
	for (QHash<QString, QString>::ConstIterator it = FTransportFillNotifications.constBegin();
		 it != FTransportFillNotifications.constEnd(); ++it)
		emit incomingTransportFilled(it.key(), it.value());
	FTransportFillNotifications.clear();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportRawUdp,JingleTransportRawUdp)
#endif

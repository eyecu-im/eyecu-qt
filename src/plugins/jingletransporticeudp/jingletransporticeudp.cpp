#include "jingletransporticeudp.h"
#include "iceudpthread.h"

#include <QDebug>

JingleTransportIceUdp::JingleTransportIceUdp(QObject *parent) :
    QObject(parent),FOptionsManager(NULL),FServiceDiscovery(NULL)
{
}

void JingleTransportIceUdp::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Jingle ICE-UDP Transport");
    APluginInfo->description = tr("Implements XEP-0176: Jingle ICE-UDP transport Method");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(JINGLE_UUID);
}

bool JingleTransportIceUdp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{    
    Q_UNUSED(AInitOrder);

    qDebug() << "JingleTransportIceUdp::initConnections()";

    IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FServiceDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    //AInitOrder = 200;   // This one should be initialized AFTER ...
    return true;
}

bool JingleTransportIceUdp::initObjects()
{
    qDebug() << "JingleTransportIceUdp::initObjects()";

    if (FServiceDiscovery)
        registerDiscoFeatures();
    return true;
}

bool JingleTransportIceUdp::initSettings()
{
    qDebug() << "JingleTransportIceUdp::initSettings()";
    return true;
}

bool JingleTransportIceUdp::openConnection(IJingleContent *AContent)
{
    emit connectionsOpenFailed(AContent);
    return false;
}

bool JingleTransportIceUdp::fillIncomingTransport(IJingleContent *AContent)
{
    emit incomingTransportFillFailed(AContent);
    return false;
}

/*
QIODevice *JingleTransportIceUdp::tryCandidate(const QDomElement &ACandidate)
{
    qDebug() << "JingleTransportIceUdp::tryCandidate()";
//    QHostAddress address(ACandidate.attribute("ip"));
//    connectSocket(address, QString(ACandidate.attribute("port")).toInt(), ACandidate.attribute("id"));
//    RawUdpThread *thread=new RawUdpThread(AStreamJid, ASid, AContentName, ACandidate.attribute("id"), this);
//    connect(thread, SIGNAL(connectionEstablished(QString,QString,QString,QString,QIODevice*)), SIGNAL(connectionEstablished(QString,QString,QString,QString,QIODevice*)));
//    connect(thread, SIGNAL(connectionFailed(QString,QString,QString,QString)), SIGNAL(connectionFailed(QString,QString,QString,QString)));
//    thread->start();
    return NULL; //! Not implemented yet
}


bool JingleTransportIceUdp::fillTransport(QDomElement ATransportElement)
{
    QDomElement element=ATransportElement.ownerDocument().createElement("candidate");
    ATransportElement.appendChild(element);
    element.setAttribute("component", 1);
    element.setAttribute("foundation", 1);
    element.setAttribute("generation", 0);
    element.setAttribute("id", "el0747fg11");
    element.setAttribute("ip", "192.168.1.1");
    element.setAttribute("network", 1);
    element.setAttribute("port", 8998);
    element.setAttribute("priority", 2130706431);
    element.setAttribute("protocol", "udp");
    element.setAttribute("type", "host");
    return true;
}

void JingleTransportIceUdp::closeDevice(const QString &ASid, const QString &ACandidateId)
{
}

QIODevice *JingleTransportIceUdp::openDevice(const QString &ASid, const QString &ACandidateId)
{
    return NULL;
}
*/

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
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_JingleTransportIceUdp,JingleTransportIceUdp)
#endif

#ifndef JINGLETRANSPORTRAWUDP_H
#define JINGLETRANSPORTRAWUDP_H

#include <QUdpSocket>

#include "interfaces/ipluginmanager.h"
#include "interfaces/ijingle.h"
#include "interfaces/iservicediscovery.h"
#include "definitions/namespaces.h"
#include "utils/iconstorage.h"

#define JINGLETRANSPORTRAWUDP_UUID "{f5bcad05-cd36-b2fc-2e47-59ad8fb49c21}"

class JingleTransportRawUdp:
        public QObject,
        public IPlugin,
        public IJingleTransport
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IJingleTransport)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingleTransportRawUdp")
#endif
public:
    explicit JingleTransportRawUdp(QObject *AParent = 0);

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return JINGLETRANSPORTRAWUDP_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}

    //IJingleTransport
    bool    isStreaming() const {return false;}
    QString ns() const {return NS_JINGLE_TRANSPORTS_RAW_UDP;}
    int     priority() const {return 90;}
    bool    openConnection(IJingleContent *AContent);
    bool    fillIncomingTransport(IJingleContent *AContent);

protected:
    void registerDiscoFeatures();

protected slots:
	void onTimeout();

signals:
	void startSend(IJingleContent *AContent);
	void startReceive(IJingleContent *AContent);
	void connectionError(IJingleContent *AContent);
    void incomingTransportFilled(IJingleContent *AContent);
    void incomingTransportFillFailed(IJingleContent *AContent);

private:
    IJingle             *FJingle;
    IServiceDiscovery   *FServiceDiscovery;
    quint16             FCurrentPort;
};

#endif // JINGLETRANSPORTRAWUDP_H

#ifndef JINGLETRANSPORTICEUDP_H
#define JINGLETRANSPORTICEUDP_H

#include <QObject>
#include "interfaces/ipluginmanager.h"
#include "interfaces/ijingle.h"
#include "interfaces/iservicediscovery.h"
#include "interfaces/ioptionsmanager.h"
#include "definitions/namespaces.h"
#include "utils/iconstorage.h"

#define JINGLETRANSPORTICEUDP_UUID "{7e2841af-24ba-9c82-d3cb-ad24b36cd4ab}"

class JingleTransportIceUdp : public QObject, public IPlugin, public IJingleTransport
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IJingleTransport)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingleTransportIceUdp")
#endif
public:
	explicit JingleTransportIceUdp(QObject *parent = nullptr);

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return JINGLETRANSPORTICEUDP_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}

    //IJingleTransport
	Types types() const {return Datagram;}
    virtual QString ns() const {return NS_JINGLE_TRANSPORTS_ICE_UDP;}
	virtual int priority() const {return 50;}
    bool openConnection(IJingleContent *AContent);
	bool fillIncomingTransport(IJingleContent *AContent);
	void freeIncomingTransport(IJingleContent *AContent);

protected:
    void    registerDiscoFeatures();

signals:
    void connectionOpened(IJingleContent *AContent);
    void connectionError(IJingleContent *AContent);
    void incomingTransportFilled(IJingleContent *AContent);
    void incomingTransportFillFailed(IJingleContent *AContent);

signals:
    void deviceOpened(QIODevice *ADevice);
    void deviceClosed(QIODevice *ADevice);
    void connectionEstablished(QIODevice *ADevice);
    void deviceOpenFailed(QIODevice *ADevice);
    void connectionFailed(QIODevice *ADevice);

private:
    IOptionsManager   *FOptionsManager;
    IServiceDiscovery *FServiceDiscovery;
};

#endif // JINGLETRANSPORTICEUDP_H

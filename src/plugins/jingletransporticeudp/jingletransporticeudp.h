#ifndef JINGLETRANSPORTICEUDP_H
#define JINGLETRANSPORTICEUDP_H

#include <QPIceTransport>
#include "interfaces/ipluginmanager.h"
#include "interfaces/ijingle.h"
#include "interfaces/iservicediscovery.h"
#include "interfaces/ioptionsmanager.h"
#include "definitions/namespaces.h"
#include "utils/iconstorage.h"
#include "icethread.h"

#define JINGLETRANSPORTICEUDP_UUID "{7e2841af-24ba-9c82-d3cb-ad24b36cd4ab}"

class JingleTransportIceUdp:
		public QObject,
		public IPlugin,
		public IJingleTransport,
		public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IJingleTransport IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingleTransportIceUdp")
#endif
public:
	explicit JingleTransportIceUdp(QObject *parent = nullptr);

    //IPlugin
	QObject *instance()  override { return this; }
	QUuid pluginUuid() const  override{ return JINGLETRANSPORTICEUDP_UUID; }
	void pluginInfo(IPluginInfo *APluginInfo) override;
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	bool initObjects() override;
	bool initSettings() override;
	bool startPlugin() override {return true;}

    //IJingleTransport
	Types types() const override {return Datagram;}
	virtual QString ns() const override {return NS_JINGLE_TRANSPORTS_ICE_UDP;}
	virtual int priority() const override {return 50;}
	bool openConnection(IJingleContent *AContent) override;
	bool fillIncomingTransport(IJingleContent *AContent) override;
	void freeIncomingTransport(IJingleContent *AContent) override;

	// IOptionsDialogHolder interface
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) override;

protected:
	void registerDiscoFeatures();
	int readCandidates(IceThread *AIceThread);
	static QHash<QHostAddress, int> networksByIp();
	void addStunServers(const QStringList &AServers);

protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);

	void onIceSuccess(int AOperation);
	void onIceThreadFinished();

signals:
	//IJingleTransport
	void connectionOpened(IJingleContent *AContent) override;
	void connectionError(IJingleContent *AContent) override;
	void incomingTransportFilled(IJingleContent *AContent) override;
	void incomingTransportFillFailed(IJingleContent *AContent) override;

    void deviceOpened(QIODevice *ADevice);
    void deviceClosed(QIODevice *ADevice);
    void connectionEstablished(QIODevice *ADevice);
    void deviceOpenFailed(QIODevice *ADevice);
    void connectionFailed(QIODevice *ADevice);

private:
    IOptionsManager   *FOptionsManager;
    IServiceDiscovery *FServiceDiscovery;	
	IJingle			  *FJingle;
	QPIceTransport::Config FIceCfg;
	QList<IceThread *> FIceThreads;
};

#endif // JINGLETRANSPORTICEUDP_H

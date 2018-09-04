#ifndef JINGLETRANSPORTRAWUDP_H
#define JINGLETRANSPORTRAWUDP_H

#include <QUdpSocket>

#include <interfaces/ipluginmanager.h>
#include <interfaces/ijingle.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ioptionsmanager.h>
#include <definitions/namespaces.h>
#include <utils/iconstorage.h>

#define JINGLETRANSPORTRAWUDP_UUID "{f5bcad05-cd36-b2fc-2e47-59ad8fb49c21}"

class JingleTransportRawUdp:
		public QObject,
		public IPlugin,
		public IJingleTransport,
		public IOptionsDialogHolder
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IJingleTransport IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingleTransportRawUdp")
#endif
public:
	explicit JingleTransportRawUdp(QObject *AParent = nullptr);

	//IPlugin
	QObject *instance() { return this; }
	QUuid pluginUuid() const { return JINGLETRANSPORTRAWUDP_UUID; }
	void pluginInfo(IPluginInfo *APluginInfo);
	bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	bool initObjects();
	bool initSettings();
	bool startPlugin(){return true;}

	//IJingleTransport
	Types	types() const {return Datagram;}
	QString ns() const {return NS_JINGLE_TRANSPORTS_RAW_UDP;}
	int     priority() const {return 90;}
	bool    openConnection(IJingleContent *AContent);
	bool    fillIncomingTransport(IJingleContent *AContent);
	void	freeIncomingTransport(IJingleContent *AContent);

	// IOptionsDialogHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

protected:
	void registerDiscoFeatures();
	QUdpSocket *getSocket(const QHostAddress &ALocalAddress, quint16 AFirst=0);

protected slots:
	void onReadyRead();
	void onTimeout();

signals:
	//IJingleTransport
	void connectionOpened(IJingleContent *AContent);
	void connectionError(IJingleContent *AContent);
	void incomingTransportFilled(IJingleContent *AContent);
	void incomingTransportFillFailed(IJingleContent *AContent);

private:
	IJingle             *FJingle;
	IServiceDiscovery   *FServiceDiscovery;
	IOptionsManager		*FOptionsManager;
	QHash<QIODevice *, IJingleContent *> FPendingContents;
	QHash<QTimer *, IJingleContent *> FPendingTimers;
	QHash<IJingleContent *, QThread *>	FThreads;
};

#endif // JINGLETRANSPORTRAWUDP_H

#ifndef POSITIONINGMETHODIPPROVIDERFREEGEOIP_H
#define POSITIONINGMETHODIPPROVIDERFREEGEOIP_H

#include <interfaces/ipositioningmethodipprovider.h>
#include <interfaces/ipluginmanager.h>

#define POSITIONINGMETHODIPPROVIDERFREEGEOIP_UUID "{4d62bcd8-af35-67d0-b9a8-247bdaf7035a}"

class PositioningMethodIpProviderFreegeoip:
		public QObject,
		public IPlugin,
		public IPositioningMethodIpProvider
{
	Q_OBJECT
	Q_INTERFACES (IPlugin IPositioningMethodIpProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.PositioningMethodIpProviderFreegeoip")
#endif
public:
	PositioningMethodIpProviderFreegeoip(QObject *parent = nullptr);
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid   pluginUuid() const { return POSITIONINGMETHODIPPROVIDERFREEGEOIP_UUID; }
	virtual void    pluginInfo(IPluginInfo *APluginInfo);
	virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool    initObjects() { return true; }
	virtual bool    initSettings() { return true; }
	virtual bool    startPlugin() { return true; }

	//IPositioningMethodIpProvider
	virtual void	setHttpRequester(HttpRequester *AHttpRequester) {FHttpRequester=AHttpRequester;}
	virtual QString	name() const;
	virtual QIcon	icon() const;
	virtual bool	request();

protected:
	void parseResult(QByteArray ASearchResult);

protected slots:
	void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
	void requestError();
	void newPositionAvailable(const GeolocElement &APoi);

private:
	HttpRequester *FHttpRequester;
};

#endif // POSITIONINGMETHODIPPROVIDERFREEGEOIP_H

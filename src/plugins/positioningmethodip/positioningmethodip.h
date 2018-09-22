#ifndef POSITIONINGMETHODIP_H
#define POSITIONINGMETHODIP_H

#include <QTimer>

#include <definitions/menuicons.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>
#include <interfaces/ipositioningmethodipprovider.h>
#include <interfaces/iconnectionmanager.h>

#include <GeolocElement>

#include "positioningmethodipoptions.h"

class PositioningMethodIp:
        public QObject,
        public IPlugin,
        public IPositioningMethod,
		public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IPositioningMethod IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.PositioningMethodIp")
#endif
public:
    PositioningMethodIp();
    ~PositioningMethodIp();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return POSITIONINGMETHODIP_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

	//IPositioningMethod
	virtual QString name() const {return tr("IP-based");}
    virtual bool select(bool ASelect);
    virtual State state() const {return FCurrentState;}
	virtual QString iconId() const {return MNI_POSITIONING_GEOIP;}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

protected:
    void changeCurrentState(State AState);
	IPositioningMethodIpProvider *currentProvider() const;

protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
	void onTimeout();
	void onNewPositionAvailable(const GeolocElement &APosition);
	void onRequestError();

signals:
	void stateChanged(int AState);
	void newPositionAvailable(const GeolocElement &APosition);

private:
	QHash <QUuid, IPositioningMethodIpProvider *> FProviders;
	State						FCurrentState;
	IOptionsManager				*FOptionsManager;
	IPositioning				*FPositioning;
	IConnectionManager			*FConnectionManager;
	PositioningMethodIpOptions	*FOptions;
	HttpRequester				FHttpRequester;
	GeolocElement				FCurrentPosition;
	QTimer						FTimer;
};

#endif // POSITIONINGMETHODIP_H

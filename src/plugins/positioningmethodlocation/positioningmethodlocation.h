#ifndef POSITIONINGMETHODLOCATION_H
#define POSITIONINGMETHODLOCATION_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>

#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>

#include "positioningmethodlocationoptions.h"

#define POSITIONINGMETHODLOCATION_UUID "{8cfb32a5-0cb7-d2c6-b42a-77acf2b360e5}"

class PositioningMethodLocation:
        public QObject,
        public IPlugin,
        public IPositioningMethod,
		public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IPositioningMethod IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.PositioningMethodLocation")
#endif
public:
    PositioningMethodLocation();
    ~PositioningMethodLocation();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return POSITIONINGMETHODLOCATION_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
    //IPositioningProvider
    virtual QString name() const {return tr("Location", "This is the name of positioning method. Please, leave it untranslated, if not sure, what does that mean!");}
    virtual bool select(bool ASelect);
    virtual State state() const {return FCurrentState;}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
	{
		QMultiMap<int, IOptionsDialogWidget *> widgets;
		if (ANodeId == OPN_POSITIONING"."+pluginUuid().toString())
			widgets.insertMulti(OWO_MANUAL, new PositioningMethodLocationOptions(AParent));
		return widgets;
	}

protected:
    void changeCurrentState(State AState);

signals:
    void stateChanged(int AState);
	void newPositionAvailable(const GeolocElement &APosition);

private:
    State                               FCurrentState;
    IOptionsManager                     *FOptionsManager;
    IPositioning                        *FPositioning;
    PositioningMethodLocationOptions    *FOptions;
};

#endif // POSITIONINGMETHODLOCATION_H

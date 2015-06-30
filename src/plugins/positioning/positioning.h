#ifndef POSITIONING_H
#define POSITIONING_H

#include <interfaces/ipositioning.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/toolbargroups.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/shortcuts.h>

#include "positioningoptions.h"

class Positioning: public QObject,
                   public IPlugin,
                   public IPositioning,
				   public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IPositioning IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IPositioning")
#endif
public:
    Positioning();
    ~Positioning();

    // IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return POSITIONING_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    // IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    // IPositioning
	virtual const GeolocElement &currentPosition() const {return FPosition;}

protected slots:
    void selectPositioningMethod(const QUuid &AMethodUuid);
    void onMethodStateChanged(int AState);
	void onNewPositionAvailable(const GeolocElement &APosition);

    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);

signals:
	void newPositionAvailable(const GeolocElement &APosition);

private:
	IOptionsManager		*FOptionsManager;
    QHash<QUuid, IPositioningMethod *> FMethods;
	IPositioningMethod	*FSelectedMethod;
	GeolocElement		FPosition;
};

#endif // POSITIONING_H

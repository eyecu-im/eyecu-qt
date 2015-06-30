#ifndef GCOMPASS_H
#define GCOMPASS_H

#include <interfaces/igcompass.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>
#include <definitions/optionnodes.h>

class Gcompass: public QObject,
                public IPlugin,
                public IGcompass
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IGcompass)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IGcompass")
#endif
public:
    Gcompass();
    ~Gcompass();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return COMPASS_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

protected slots:
    void onNewPositionAvailable(const GeolocElement &APosition);
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);

};

#endif // GCOMPASS_H

#ifndef MAPSOURCEROSREESTR_H
#define MAPSOURCEROSREESTR_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceRosreestr>

#define MAPSOURCEROSREESTR_UUID "{bc2d9f4a-8fd0-4ba5-7d6e-f8ea54dbc75a}"

class MapSourceRosreestr :  public SourceRosreestr, public IPlugin, public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceRosreestr")
#endif
public:
    MapSourceRosreestr();
    ~MapSourceRosreestr();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
	virtual	QString			getIconId() const {return MNI_MAP_ROSREESTR;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCEROSREESTR_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
    virtual bool initObjects(){return true;}
    virtual bool initSettings(){return true;}
    virtual bool startPlugin(){return true;}	
};

#endif // MAPSOURCEROSREESTR_H

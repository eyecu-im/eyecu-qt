#ifndef MAPSOURCERUMAP_H
#define MAPSOURCERUMAP_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceRuMap>

#define MAPSOURCERUMAP_UUID "{f3cda5de-09c8-d3b0-26ab-7d5acb3d65b2}"

#define MODE_MAP 0
#define MODE_HYBRID 1
#define MODE_SATELLITE 2

class MapSourceRuMap:
		public SourceRuMap,
        public IPlugin,
        public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceRuMap")
#endif
public:
    MapSourceRuMap();
    ~MapSourceRuMap();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
    virtual QString			getIconId() const {return MNI_MAP_GEOCENTRECONSULTING;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid   pluginUuid() const { return MAPSOURCERUMAP_UUID; }
    virtual void    pluginInfo(IPluginInfo *APluginInfo);
	virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
    virtual bool    initObjects(){return true;}
    virtual bool    initSettings(){return true;}
    virtual bool    startPlugin(){return true;}	
};

#endif // MAPSOURCERUMAP_H

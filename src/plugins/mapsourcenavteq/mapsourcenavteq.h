#ifndef MAPSOURCENAVTEQ_H
#define MAPSOURCENAVTEQ_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceNavteq>

#define MAPSOURCENAVTEQ_UUID "{2b36ce6f-b22c-7de8-56bd-ca38bdf6c78a}"

class MapSourceNavteq:
		public SourceNavteq,
        public IPlugin,
        public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceNavteq")
#endif
public:
    MapSourceNavteq();
    ~MapSourceNavteq();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId()  const {return MNI_MAP_NAVTEQ;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCENAVTEQ_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
    virtual bool initObjects(){return true;}
    virtual bool initSettings(){return true;}
    virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCENAVTEQ_H

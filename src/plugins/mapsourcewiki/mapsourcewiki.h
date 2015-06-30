#ifndef MAPSOURCEWIKI_H
#define MAPSOURCEWIKI_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceWiki>

#define MAPSOURCEWIKI_UUID "{f45cb322-c48a-83b5-c62e-ce55d2376b92}"

class MapSourceWiki:
		public SourceWiki,
		public IPlugin,
		public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceWiki")
#endif
public:
    MapSourceWiki();
    ~MapSourceWiki();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_WIKI;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCEWIKI_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
    virtual bool initObjects(){return true;}
    virtual bool initSettings(){return true;}
    virtual bool startPlugin(){return true;}	
};

#endif // MAPSOURCEWIKI_H

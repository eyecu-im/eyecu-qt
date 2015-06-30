#ifndef MAPSOURCEYAHOO_H
#define MAPSOURCEYAHOO_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceYahoo>

#define MAPSOURCEYAHOO_UUID "{23b149a8-62c6-34b5-c28a-2644b1c7ae28}"

class MapSourceYahoo:
		public SourceYahoo,
        public IPlugin,
        public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceYahoo")
#endif
public:
    MapSourceYahoo();
    ~MapSourceYahoo();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_YAHOO;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCEYAHOO_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCEYAHOO_H

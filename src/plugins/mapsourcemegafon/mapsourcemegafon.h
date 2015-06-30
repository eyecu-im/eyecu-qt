#ifndef MAPSOURCEMEGAFON_H
#define MAPSOURCEMEGAFON_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceMegafon>

#define MAPSOURCEMEGAFON_UUID "{2f4d6eba-3c81-5d4e-db6c-fb83dc90b0a7}"

class MapSourceMegafon:
		public SourceMegafon,
        public IPlugin,
        public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceMegafon")
#endif
public:
    MapSourceMegafon();
    ~MapSourceMegafon();

	//MapSource
	virtual MapSource		*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_MEGAFON;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCEMEGAFON_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
    virtual bool initObjects(){return true;}
    virtual bool initSettings(){return true;}
    virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCEMEGAFON_H

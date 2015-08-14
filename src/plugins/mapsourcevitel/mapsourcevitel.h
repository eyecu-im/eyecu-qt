#ifndef MAPSOURCEVITEL_H
#define MAPSOURCEVITEL_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceViTel>

#define MAPSOURCEVITEL_UUID "{2d385bed-f67d-a601-b47d-9c828db1f63b}"

class MapSourceViTel:
		public SourceViTel,
        public IPlugin,
        public IMapSource
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceViTel")
#endif
public:
    MapSourceViTel();
    ~MapSourceViTel();

	//IMapSource
	MapSource				*mapSource() {return this;}
    virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_VITEL;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCEVITEL_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
    virtual bool initObjects(){return true;}
    virtual bool initSettings(){return true;}
    virtual bool startPlugin(){return true;}	
};

#endif // MAPSOURCEVITEL_H

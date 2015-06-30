#ifndef MAPSOURCEKOSMOSNIMKI_H
#define MAPSOURCEKOSMOSNIMKI_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceKosmosnimki>

#define MAPSOURCEKOSMOSNIMKI_UUID "{2f463c21-fb2a-846f-cd33-d2e66789a30b}"

#define MODE_MAP 0
#define MODE_HYBRID 1
#define MODE_SATELLITE 2
#define MODE_LANDSCAPE 3

class MapSourceKosmosnimki:
		public SourceKosmosnimki,
		public IPlugin,
		public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource )
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceKosmosnimki")
#endif
public:
	MapSourceKosmosnimki();
	~MapSourceKosmosnimki();

	//IMapSource
	virtual MapSource		*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_KOSMOSNIMKI;}

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid   pluginUuid() const { return MAPSOURCEKOSMOSNIMKI_UUID; }
	virtual void    pluginInfo(IPluginInfo *APluginInfo);
	virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool    initObjects(){return true;}
	virtual bool    initSettings(){return true;}
	virtual bool    startPlugin(){return true;}
};

#endif // MAPSOURCEKOSMOSNIMKI_H

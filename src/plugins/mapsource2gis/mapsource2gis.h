#ifndef MAPSOURCE2GIS_H
#define MAPSOURCE2GIS_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <Source2Gis>

#define MAPSOURCE2GIS_UUID "{b42ae810-2bc3-8f4a-2bf4-b2dec84b11a2}"

class MapSource2Gis : public Source2Gis, public IPlugin, public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin MapSource IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSource2Gis")
#endif
public:
	//IPlugin
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return MAPSOURCE2GIS_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}

	//IMapSource
	virtual MapSource		*mapSource() {return this;}	
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_2GIS;}
};

#endif // MAPSOURCE2GIS_H

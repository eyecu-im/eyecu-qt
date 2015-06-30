#ifndef MAPSOURCEBING_H
#define MAPSOURCEBING_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceBing>

#define MAPSOURCEBING_UUID "{f3db45a6-8ed1-c672-d418-57ac923d5f48}"

class MapSourceBing: public SourceBing, public IPlugin, public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin MapSource IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceBing")
#endif

public:
	//IPlugin
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return MAPSOURCEBING_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}

	//IMapSource
	MapSource				*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId()  const {return MNI_MAP_BING;}
};

#endif // MAPSOURCEBING_H

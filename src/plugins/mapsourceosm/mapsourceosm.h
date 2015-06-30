#ifndef MAPSOURCEOSM_H
#define MAPSOURCEOSM_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceOsm>

#define MAPSOURCEOSM_UUID "{26a9b0d4-2ef8-31c3-482a-b391e0a7723d}"

class MapSourceOsm:
		public SourceOsm,
		public IPlugin,
		public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceOsm")
#endif
public:
	MapSourceOsm();
	~MapSourceOsm();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_OSM;}

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return MAPSOURCEOSM_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCEOSM_H

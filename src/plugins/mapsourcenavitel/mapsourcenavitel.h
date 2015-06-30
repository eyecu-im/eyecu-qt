#ifndef MAPSOURCENAVITEL_H
#define MAPSOURCENAVITEL_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceNavitel>

#define MAPSOURCENAVITEL_UUID "{23f6bc21-34ae-2fc3-254a-a5789b3d2e31}"

class MapSourceNavitel:
		public SourceNavitel,
		public IPlugin,
		public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceNavitel")
#endif
public:
	MapSourceNavitel();
	~MapSourceNavitel();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_NAVITEL;}

	//IPlugin
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return MAPSOURCENAVITEL_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCENAVITEL_H

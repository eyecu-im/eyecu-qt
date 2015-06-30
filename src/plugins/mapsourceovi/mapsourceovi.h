#ifndef MAPSOURCEOVI_H
#define MAPSOURCEOVI_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceOvi>

#define MAPSOURCEOVI_UUID "{72c36bde-a25c-b283-c325-8f649a23de81}"

class MapSourceOvi:
		public SourceOvi,
		public IPlugin,
		public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceOvi")
#endif
public:
	MapSourceOvi();
	~MapSourceOvi();

	virtual MapSource		*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_HERE;}

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return MAPSOURCEOVI_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCEOVI_H

#ifndef MAPSOURCEPROGOROD_H
#define MAPSOURCEPROGOROD_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceProGorod>

#define MAPSOURCPROGOROD_UUID "{4dbf3cf8-2ab4-0ed6-b7af-dc76e30bd14a}"

class MapSourceProGorod:
		public SourceProGorod,
		public IPlugin,
		public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceProGorod")
#endif
public:
	MapSourceProGorod();
	~MapSourceProGorod();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_PROGOROD;}

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return MAPSOURCPROGOROD_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}
};

#endif // MAPSOURCEPROGOROD_H

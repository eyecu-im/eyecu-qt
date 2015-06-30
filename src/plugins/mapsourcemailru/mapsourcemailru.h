#ifndef MAPSOURCEMAILRU_H
#define MAPSOURCEMAILRU_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceMailRu>

#define MAPSOURCEMAILRU_UUID "{32b764d2-68b5-23e8-456c-268bfd439d05}"

class MapSourceMailRu:
		public SourceMailRu,
		public IPlugin,
		public IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceMailRu")
#endif
public:
	MapSourceMailRu();
	~MapSourceMailRu();

	//IMapSource
	virtual MapSource		*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_MAILRU;}

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return MAPSOURCEMAILRU_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}

private:
	mutable int FIndex;
};

#endif // MAPSOURCEMAILRU_H

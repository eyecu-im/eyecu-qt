#ifndef MAPSOURCEESRI_H
#define MAPSOURCEESRI_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceEsri>

#define MAPSOURCEESRI_UUID "{2f365bd8-ce8d-2098-b321-e8a259c4a68f}"

class MapSourceEsri: public SourceEsri, public IPlugin, IMapSource
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceEsri")
#endif
public:
	//IPlugin
	virtual QObject *instance() {return this;}
	virtual QUuid pluginUuid() const {return MAPSOURCEESRI_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) {Q_UNUSED(APluginManager) Q_UNUSED(AInitOrder) return true;}
	virtual bool initObjects(){return true;}
	virtual bool initSettings(){return true;}
	virtual bool startPlugin(){return true;}

	//IMapSource
	virtual MapSource		*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_ESRI;}
};

#endif // MAPSOURCEESRI_H

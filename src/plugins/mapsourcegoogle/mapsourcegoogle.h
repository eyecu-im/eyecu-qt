#ifndef MAPSOURCEGOOGLE_H
#define MAPSOURCEGOOGLE_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <SourceGoogle>

#define MAPSOURCEGOOGLE_UUID "{5a8d2702-6b53-c2a1-324d-e4a624723c1e}"

class MapSourceGoogle: public SourceGoogle, public IPlugin, public IMapSource, public IOptionsDialogHolder
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IOptionsDialogHolder IMapSource)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceGoogle")
#endif
public:
	MapSourceGoogle();
	~MapSourceGoogle();

	//IMapSource
	virtual MapSource		*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_GOOGLE;}

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return MAPSOURCEGOOGLE_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin(){return true;}

	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);

private:
	IOptionsManager *FOptionsManager;
	IMap			*FMap;
};

#endif // MAPSOURCEGOOGLE_H

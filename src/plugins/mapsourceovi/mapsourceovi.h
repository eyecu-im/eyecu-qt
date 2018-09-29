#ifndef MAPSOURCEOVI_H
#define MAPSOURCEOVI_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <SourceOvi>

#define MAPSOURCEOVI_UUID "{72c36bde-a25c-b283-c325-8f649a23de81}"

class MapSourceOvi:
		public SourceOvi,
		public IPlugin,
		public IMapSource,
		public IOptionsDialogHolder
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceOvi")
#endif
public:
	MapSourceOvi();
	~MapSourceOvi();

	// MapSource interface
	virtual MapSource		*mapSource() override {return this;}
	virtual QList<int>      getModeIcons() const override;
	virtual QString			getIconId() const override {return MNI_MAP_HERE;}

	//IPlugin
	virtual QObject *instance() override { return this; }
	virtual QUuid pluginUuid() const override { return MAPSOURCEOVI_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo) override;
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	virtual bool initObjects() override;
	virtual bool initSettings() override;
	virtual bool startPlugin() override {return true;}

	// IOptionsDialogHolder interface
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) override;

protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);

private:
	IOptionsManager *FOptionsManager;
	IMap			*FMap;
};

#endif // MAPSOURCEOVI_H

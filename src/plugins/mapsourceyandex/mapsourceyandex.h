#ifndef MAPSOURCEYANDEX_H
#define MAPSOURCEYANDEX_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imap.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <SourceYandex>

#define MAPSOURCEYANDEX_UUID "{6a72cb3f-2c1a-7d22-b34d-7bca358d09e1}"

class MapSourceYandex:
		public SourceYandex,
        public IPlugin,
        public IMapSource,
		public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSource IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSourceYandex")
#endif
public:
    MapSourceYandex();
    ~MapSourceYandex();

	// IMapSource interface
	MapSource				*mapSource() {return this;}
	virtual QList<int>      getModeIcons() const;
	virtual QString			getIconId() const {return MNI_MAP_YANDEX;}

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSOURCEYANDEX_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects(){return true;}
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

#endif // MAPSOURCEYANDEX_H

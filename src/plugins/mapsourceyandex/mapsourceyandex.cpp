#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>

#include "mapsourceyandex.h"
#include "optionsyandex.h"

MapSourceYandex::MapSourceYandex()
{}

MapSourceYandex::~MapSourceYandex()
{}

//-----------------------------
void MapSourceYandex::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Yandex map source");
    APluginInfo->description = tr("Allows Map plugin to use Yandex as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(MAPSCENE_UUID);
}

bool MapSourceYandex::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
	if (plugin)
		FMap = qobject_cast<IMap *>(plugin->instance());

	connect(Options::instance(), SIGNAL(optionsOpened()), SLOT(onOptionsOpened()));
	connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)), SLOT(onOptionsChanged(OptionsNode)));

    return true;
}


bool MapSourceYandex::initSettings()
{
	Options::setDefaultValue(OPV_MAP_SOURCE_YANDEX_VERSION_SATELLITE, "5.203.0");
	Options::setDefaultValue(OPV_MAP_SOURCE_YANDEX_VERSION_SCHEME, "5.33.0");
    if (FOptionsManager)
    {
		IOptionsDialogNode node = {ONO_MAP_YANDEX, OPN_MAP_YANDEX, MNI_MAP_YANDEX, tr("Yandex")};
		FMap->insertOptionsDialogNode(node);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapSourceYandex::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_MAP_YANDEX )
        widgets.insertMulti(OWO_MAPSOURCE, new OptionsYandex(AParent));
    return widgets;
}

void MapSourceYandex::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_YANDEX_VERSION_SATELLITE));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_YANDEX_VERSION_SCHEME));
}

void MapSourceYandex::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MAP_SOURCE_YANDEX_VERSION_SATELLITE)
		setVersionSattellite(ANode.value().toString());
	else if (ANode.path() == OPV_MAP_SOURCE_YANDEX_VERSION_SCHEME)
		setVersionScheme(ANode.value().toString());
}

QList<int> MapSourceYandex::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP << ICON_MAP1 << ICON_HYBRID << ICON_SATELLITE;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceyandex, MapSourceYandex)
#endif

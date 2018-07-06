#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>

#include "mapsourcegoogle.h"
#include "settingsgoogle.h"

MapSourceGoogle::MapSourceGoogle():
	FOptionsManager(NULL), FMap(NULL)
{}

MapSourceGoogle::~MapSourceGoogle()
{}
//-----------------------------

void MapSourceGoogle::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Google map source");
	APluginInfo->description = tr("Allows Map plugin to use Google as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

bool MapSourceGoogle::initConnections(IPluginManager *APluginManager, int &AInitOrder)
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

bool MapSourceGoogle::initObjects()
{
	return true;
}

bool MapSourceGoogle::initSettings()
{
	Options::setDefaultValue(OPV_MAP_SOURCE_GOOGLE_VERSION_SATELLITE, 719);
	Options::setDefaultValue(OPV_MAP_SOURCE_GOOGLE_VERSION_MAP, 169);
	Options::setDefaultValue(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_T, 169);
	Options::setDefaultValue(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_R, 169);
	if (FOptionsManager)
	{
		IOptionsDialogNode node = {ONO_MAP_GOOGLE, OPN_MAP_GOOGLE, MNI_MAP_GOOGLE, tr("Google")};
		FMap->insertOptionsDialogNode(node);
		FOptionsManager->insertOptionsDialogHolder(this);
	}
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapSourceGoogle::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MAP_GOOGLE )
		widgets.insertMulti(OWO_MAPSOURCE, new SettingsGoogle(AParent));
	return widgets;
}

void MapSourceGoogle::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_MAP));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_SATELLITE));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_R));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_T));
}

void MapSourceGoogle::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MAP_SOURCE_GOOGLE_VERSION_MAP)
		setVersionMap(ANode.value().toInt());
	else if (ANode.path() == OPV_MAP_SOURCE_GOOGLE_VERSION_SATELLITE)
		setVersionSatellite(ANode.value().toInt());
	else if (ANode.path() == OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_R)
		setVersionTerrainR(ANode.value().toInt());
	else if (ANode.path() == OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_T)
		setVersionTerrainT(ANode.value().toInt());
}

QList<int> MapSourceGoogle::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP << ICON_HYBRID << ICON_SATELLITE << ICON_TERRAIN;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcegoogle, MapSourceGoogle)
#endif

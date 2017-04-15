#include "mapsourceosm.h"

MapSourceOsm::MapSourceOsm()
{}

MapSourceOsm::~MapSourceOsm()
{}

//-----------------------------
void MapSourceOsm::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("OSM map source");
	APluginInfo->description = tr("Allows Map plugin to use Open Street Maps as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

QList<int> MapSourceOsm::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP << ICON_MAP1 << ICON_MAP2 << ICON_MAP3 << ICON_TERRAIN;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceosm, MapSourceOsm)
#endif

#include "mapsourceovi.h"

MapSourceOvi::MapSourceOvi()
{}

MapSourceOvi::~MapSourceOvi()
{}

//-----------------------------
void MapSourceOvi::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("HERE map source");
	APluginInfo->description = tr("Allows Map plugin to use Nokia's HERE service as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

QList<int> MapSourceOvi::getModeIcons() const
{
	return QList<int>() << ICON_MAP << ICON_HYBRID << ICON_TERRAIN << ICON_SATELLITE;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceovi, MapSourceOvi)
#endif

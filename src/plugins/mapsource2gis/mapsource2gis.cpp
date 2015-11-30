#include "mapsource2gis.h"

//-----------------------------
void MapSource2Gis::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("2GIS map source");
	APluginInfo->description = tr("Allows Map plugin to use 2GIS service as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

QList<int> MapSource2Gis::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsource2gis, MapSource2Gis)
#endif

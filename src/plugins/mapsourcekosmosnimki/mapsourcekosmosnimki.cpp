#include "mapsourcekosmosnimki.h"

MapSourceKosmosnimki::MapSourceKosmosnimki()
{}

MapSourceKosmosnimki::~MapSourceKosmosnimki()
{}

//-----------------------------
void MapSourceKosmosnimki::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Kosmosnimki map source");
	APluginInfo->description = tr("Allows Map plugin to use Kosmosnimki as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

//-----------------------------

QList<int> MapSourceKosmosnimki::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP << ICON_HYBRID << ICON_SATELLITE << ICON_TERRAIN;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcekosmosnimki, MapSourceKosmosnimki)
#endif

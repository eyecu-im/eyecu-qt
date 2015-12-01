#include "mapsourcenavitel.h"

MapSourceNavitel::MapSourceNavitel()
{}
//-----------------------------

MapSourceNavitel::~MapSourceNavitel()
{}

//-----------------------------
void MapSourceNavitel::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Navitel map source");
	APluginInfo->description = tr("Allows Map plugin to use Navitel as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

QList<int> MapSourceNavitel::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcenavitel, MapSourceNavitel)
#endif

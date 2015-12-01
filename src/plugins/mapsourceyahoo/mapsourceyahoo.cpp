#include "mapsourceyahoo.h"

MapSourceYahoo::MapSourceYahoo()
{}
//-----------------------------

MapSourceYahoo::~MapSourceYahoo()
{}

//-----------------------------
void MapSourceYahoo::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Yahoo! map source");
    APluginInfo->description = tr("Allows Map plugin to use Yahoo! as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

QList<int> MapSourceYahoo::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP << ICON_HYBRID << ICON_SATELLITE;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceyahoo, MapSourceYahoo)
#endif

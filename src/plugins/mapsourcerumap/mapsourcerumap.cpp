#include "mapsourcerumap.h"

MapSourceRuMap::MapSourceRuMap()
{}

MapSourceRuMap::~MapSourceRuMap()
{}

//-----------------------------
void MapSourceRuMap::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("RuMap map source");
    APluginInfo->description = tr("Allows Map plugin to use RuMap (Geocentre-Consulting) as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(MAPSCENE_UUID);
}

//-----------------------------

QList<int> MapSourceRuMap::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP << ICON_HYBRID << ICON_SATELLITE;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcerumap, MapSourceRuMap)
#endif

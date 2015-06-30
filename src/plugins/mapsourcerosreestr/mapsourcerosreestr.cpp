#include "mapsourcerosreestr.h"

MapSourceRosreestr::MapSourceRosreestr()
{}

MapSourceRosreestr::~MapSourceRosreestr()
{}

//-----------------------------
void MapSourceRosreestr::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Rosreestr map source");
    APluginInfo->description = tr("Allows Map plugin to use Rosreestr service as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(MAPSCENE_UUID);
}

QList<int> MapSourceRosreestr::getModeIcons() const
{
    return QList<int>() << ICON_MAP;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcerosreestr, MapSourceRosreestr)
#endif

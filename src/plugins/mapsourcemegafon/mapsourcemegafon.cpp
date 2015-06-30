#include "mapsourcemegafon.h"

MapSourceMegafon::MapSourceMegafon()
{}

MapSourceMegafon::~MapSourceMegafon()
{}

void MapSourceMegafon::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("MegaFon Navigator map source");
    APluginInfo->description = tr("Allows Map plugin to use MegaFon Navigator as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(MAPSCENE_UUID);
}

QList<int> MapSourceMegafon::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcemegafon, MapSourceMegafon)
#endif

#include "mapsourcewiki.h"

MapSourceWiki::MapSourceWiki()
{}
//-----------------------------

MapSourceWiki::~MapSourceWiki()
{}

//-----------------------------
void MapSourceWiki::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Wikimapia map source");
    APluginInfo->description = tr("Allows Map plugin to use Wikimapia as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
//    APluginInfo->dependences.append(MAPSCENE_UUID);
}

#define MODE_MAP 0

QList<int> MapSourceWiki::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcewiki, MapSourceWiki)
#endif

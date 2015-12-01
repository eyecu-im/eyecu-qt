#include "mapsourcenavteq.h"

MapSourceNavteq::MapSourceNavteq()
{}

MapSourceNavteq::~MapSourceNavteq()
{}

//-----------------------------
void MapSourceNavteq::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Navteq map source");
    APluginInfo->description = tr("Allows Map plugin to use Navteq service as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

#define MODE_MAP        0
#define MODE_NN4D       1
#define MODE_MAPQUEST   2
#define MODE_SATELLITE  3

QList<int> MapSourceNavteq::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP << ICON_MAP1 << ICON_MAP2 << ICON_SATELLITE;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcenavteq, MapSourceNavteq)
#endif

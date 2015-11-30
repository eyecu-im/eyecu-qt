#include "mapsourcevitel.h"

MapSourceViTel::MapSourceViTel()
{}

MapSourceViTel::~MapSourceViTel()
{}

//-----------------------------
void MapSourceViTel::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Vi-Tel map source");
    APluginInfo->description = tr("Allows Map plugin to use Vi-Tel service as map source");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

QList<int> MapSourceViTel::getModeIcons() const
{
    QList<int> list;
    list << ICON_MAP;
    return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcevitel, MapSourceViTel)
#endif

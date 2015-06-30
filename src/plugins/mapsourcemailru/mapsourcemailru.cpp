#include "mapsourcemailru.h"

MapSourceMailRu::MapSourceMailRu(): FIndex(1)
{}

MapSourceMailRu::~MapSourceMailRu()
{}

//-----------------------------
void MapSourceMailRu::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Mail.ru map source");
	APluginInfo->description = tr("Allows Map plugin to use maps@mail.ru as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
//	APluginInfo->dependences.append(MAPSCENE_UUID);
}

QList<int> MapSourceMailRu::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP << ICON_SATELLITE;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcemailru, MapSourceMailRu)
#endif

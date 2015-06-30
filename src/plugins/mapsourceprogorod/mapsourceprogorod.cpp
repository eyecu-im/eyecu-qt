#include "mapsourceprogorod.h"

MapSourceProGorod::MapSourceProGorod()
{}

MapSourceProGorod::~MapSourceProGorod()
{}
//-----------------------------

void MapSourceProGorod::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Pro-gorod map source");
	APluginInfo->description = tr("Allows Map plugin to use Pro-gorod as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
//	APluginInfo->dependences.append(MAPSCENE_UUID);
}

QList<int> MapSourceProGorod::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceprogorod, MapSourceProGorod)
#endif

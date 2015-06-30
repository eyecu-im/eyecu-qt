#include "mapsourcebing.h"

//-----------------------------
void MapSourceBing::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Bing map source");
	APluginInfo->description = tr("Allows Map plugin to use Microsoft's Bing service as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
//	APluginInfo->dependences.append(MAPSCENE_UUID);
}

QList<int> MapSourceBing::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP << ICON_HYBRID << ICON_SATELLITE;
	return list;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourcebing, MapSourceBing)
#endif

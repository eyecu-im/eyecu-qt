#include "mapsourceesri.h"

//-----------------------------
void MapSourceEsri::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Esri map source");
	APluginInfo->description = tr("Allows Map plugin to use Esri service as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
//	APluginInfo->dependences.append(MAPSCENE_UUID);
}

QList<int> MapSourceEsri::getModeIcons() const
{
	QList<int> list;
	list << ICON_MAP << ICON_HYBRID << ICON_SATELLITE << ICON_TERRAIN;
	return list;
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceesri, MapSourceEsri)
#endif

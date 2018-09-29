#include <QDebug>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include "mapsourceovi.h"
#include "settingshere.h"

#define APP_ID "VKGZLTjvEdi1xsdDMKJp"
#define APP_CODE "I7t8K4lZHm7pLLZ0k8XIwg"

MapSourceOvi::MapSourceOvi():
	FOptionsManager(nullptr),
	FMap(nullptr)
{}

MapSourceOvi::~MapSourceOvi()
{}

//-----------------------------
void MapSourceOvi::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("HERE map source");
	APluginInfo->description = tr("Allows Map plugin to use Nokia's HERE service as map source");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
}

bool MapSourceOvi::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

	IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMap").value(0, nullptr);
	if (plugin)
		FMap = qobject_cast<IMap *>(plugin->instance());

	connect(Options::instance(), SIGNAL(optionsOpened()), SLOT(onOptionsOpened()));
	connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)), SLOT(onOptionsChanged(OptionsNode)));

	return true;
}

bool MapSourceOvi::initObjects()
{
	setAppId(APP_ID);
	setAppCode(APP_CODE);

	if (FOptionsManager)
	{
		IOptionsDialogNode node = {ONO_MAP_HERE, OPN_MAP_HERE, MNI_MAP_HERE, tr("Here")};
		FMap->insertOptionsDialogNode(node);
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

bool MapSourceOvi::initSettings()
{
	Options::setDefaultValue(OPV_MAP_SOURCE_HERE_LANG_PRIMARY, QString());
	Options::setDefaultValue(OPV_MAP_SOURCE_HERE_LANG_SECONDARY, QString());
	Options::setDefaultValue(OPV_MAP_SOURCE_HERE_POLITICALVIEW, QString());
	Options::setDefaultValue(OPV_MAP_SOURCE_HERE_MODE_NIGHT, false);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapSourceOvi::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MAP_HERE )
		widgets.insertMulti(OWO_MAPSOURCE, new SettingsHere(AParent));
	return widgets;
}

void MapSourceOvi::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_HERE_LANG_PRIMARY));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_HERE_LANG_SECONDARY));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_HERE_POLITICALVIEW));
	onOptionsChanged(Options::node(OPV_MAP_SOURCE_HERE_MODE_NIGHT));
}

void MapSourceOvi::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MAP_SOURCE_HERE_LANG_PRIMARY)
	{
		QString lg = ANode.value().toString();
		setLanguage(lg.isEmpty()?lang(QLocale().name()):lg);
	}
	else if (ANode.path() == OPV_MAP_SOURCE_HERE_LANG_SECONDARY)
	{
		setLanguage2(ANode.value().toString());
	}
	else if (ANode.path() == OPV_MAP_SOURCE_HERE_POLITICALVIEW)
	{
		QString pview(ANode.value().toString());
		if (pview.isEmpty())
			setPView(QLocale());
		else
			setPView(pview);
	}
	else if (ANode.path() == OPV_MAP_SOURCE_HERE_MODE_NIGHT)
	{
		setStyle(ANode.value().toBool()?Night:Day);
	}
}

QList<int> MapSourceOvi::getModeIcons() const
{
	return QList<int>() << ICON_MAP << ICON_MAP1 << ICON_HYBRID
						<< ICON_SATELLITE << ICON_TERRAIN << ICON_TERRAIN;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsourceovi, MapSourceOvi)
#endif

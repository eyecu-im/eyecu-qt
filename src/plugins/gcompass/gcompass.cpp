#include "gcompass.h"

Gcompass::Gcompass()
{}

Gcompass::~Gcompass()
{}

//-----------------------------
void Gcompass::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Compass");
	APluginInfo->description = tr("Compass");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(POSITIONING_UUID);
}

bool Gcompass::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin = APluginManager->pluginInterface("IPositioning").value(0,NULL);
    if (plugin)
		connect(plugin->instance(),SIGNAL(newPositionAvailable(GeolocElement)),SLOT(onNewPositionAvailable(GeolocElement)));
    else
        return false;

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    //AInitOrder = 200;   // This one should be initialized AFTER ...
    return true; //FMessageWidgets!=NULL
}

bool Gcompass::initObjects()
{

    return true;
}

bool Gcompass::initSettings()
{
    return true;
}

void Gcompass::onNewPositionAvailable(const GeolocElement &APosition)
{
	Q_UNUSED(APosition)
}


void Gcompass::onOptionsOpened()
{
}

void Gcompass::onOptionsClosed()
{
}

void Gcompass::onOptionsChanged(const OptionsNode &ANode)
{
	Q_UNUSED(ANode)
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_gcompass, Gcompass)
#endif

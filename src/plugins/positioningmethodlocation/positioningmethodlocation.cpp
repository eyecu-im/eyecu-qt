#include "positioningmethodlocation.h"

PositioningMethodLocation::PositioningMethodLocation():
    FCurrentState(Stopped),
    FOptionsManager(NULL),
    FPositioning(NULL),
    FOptions(NULL)
{}

PositioningMethodLocation::~PositioningMethodLocation()
{}

//-----------------------------
void PositioningMethodLocation::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Positioning Method Location");
	APluginInfo->description = tr("Positioning method, which uses Location interface");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(POSITIONING_UUID);
}

bool PositioningMethodLocation::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    AInitOrder = 150;   // This one should be initialized AFTER ...
    return true; //FMessageWidgets!=NULL

}

bool PositioningMethodLocation::initObjects()
{
    return true;
}

bool PositioningMethodLocation::initSettings()
{
    Options::setDefaultValue(OPV_POSITIONING_METHOD_LOCATION_INTERVAL,60);
    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_LOCATION, OPN_GEOLOC"."+pluginUuid().toString(), MNI_POSITIONING_LOCATION, tr("Location")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

bool PositioningMethodLocation::select(bool ASelect)
{
	Q_UNUSED(ASelect)

    return false;
}



void PositioningMethodLocation::changeCurrentState(IPositioningMethod::State AState)
{
    if (FCurrentState != AState)
        emit stateChanged(FCurrentState = AState);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_positioningmethodlocation, PositioningMethodLocation)
#endif

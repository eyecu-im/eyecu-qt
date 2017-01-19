#include "positioning.h"

#define POSITIONINGMETHODMANUAL_UUID "{32dbe444-c291-4947-ab76-899ce7bcd023}"

Positioning::Positioning():
	FOptionsManager(NULL),
    FSelectedMethod(NULL)
{}

Positioning::~Positioning()
{}

//-----------------------------
void Positioning::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Positioning");
    APluginInfo->description = tr("Allows to determine device position");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
}

bool Positioning::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    QList<IPlugin *> providers = APluginManager->pluginInterface("IPositioningMethod");
    for (QList<IPlugin *>::ConstIterator it = providers.constBegin(); it!=providers.constEnd(); it++)
        FMethods.insert((*it)->pluginUuid(), qobject_cast<IPositioningMethod *>((*it)->instance()));

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    AInitOrder = 200;   // This one should be initialized AFTER .....!
    return true;
}

bool Positioning::initObjects()
{
    return true;
}

bool Positioning::initSettings()
{
    Options::setDefaultValue(OPV_POSITIONING_METHOD, POSITIONINGMETHODMANUAL_UUID);
    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_POSITIONING, OPN_POSITIONING, MNI_POSITIONING, tr("Positioning")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Positioning::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;

	if (ANodeId == OPN_POSITIONING)
	{
		widgets.insertMulti(OHO_POSITIONING, FOptionsManager->newOptionsDialogHeader(tr("General"), AParent));
		widgets.insertMulti(OWO_POSITIONING, new PositioningOptions(this, FMethods, FOptionsManager, AParent));
	}
	return widgets;
}

void Positioning::selectPositioningMethod(const QUuid &AMethodUuid)
{
    IPositioningMethod *method = FMethods.value(AMethodUuid);
    if (FSelectedMethod != method)
    {
        if (FSelectedMethod)
        {
			FSelectedMethod->instance()->disconnect(SIGNAL(newPositionAvailable(GeolocElement)),this,SLOT(onNewPositionAvailable(GeolocElement)));
			FSelectedMethod->instance()->disconnect(SIGNAL(newPositionAvailable(GeolocElement)),this,SIGNAL(newPositionAvailable(GeolocElement)));
            FSelectedMethod->instance()->disconnect(SIGNAL(stateChanged(int)),this,SLOT(onMethodStateChanged(int)));
            FSelectedMethod->select(false);
            if (FPosition.isValid())
            {
                FPosition.clear();
                emit newPositionAvailable(FPosition);
            }
        }
        FSelectedMethod = method;
        if (FSelectedMethod)
        {
			connect(FSelectedMethod->instance(),SIGNAL(newPositionAvailable(GeolocElement)),SIGNAL(newPositionAvailable(GeolocElement)));
			connect(FSelectedMethod->instance(),SIGNAL(newPositionAvailable(GeolocElement)),SLOT(onNewPositionAvailable(GeolocElement)));
            connect(FSelectedMethod->instance(),SIGNAL(stateChanged(int)),SLOT(onMethodStateChanged(int)));
            FSelectedMethod->select(true);
        }
    }
}

void Positioning::onMethodStateChanged(int AState)
{
    if (AState == IPositioningMethod::Stopped)
    {
        QUuid uuid(POSITIONINGMETHODMANUAL_UUID); // Manual
        if (!FMethods.contains(uuid))
            uuid = QUuid();
        Options::node(OPV_POSITIONING_METHOD).setValue(uuid.toString());
    }
}

void Positioning::onNewPositionAvailable(const GeolocElement &APosition)
{
    FPosition = APosition;
}

void Positioning::onOptionsOpened()
{
    QUuid tmp(Options::node(OPV_POSITIONING_METHOD).value().toString());
    if(!FMethods.contains(tmp))
    {
        tmp = POSITIONINGMETHODMANUAL_UUID; // Manual
        if(!FMethods.contains(tmp))
            tmp = QUuid();                              // No positioning provider
    }
    selectPositioningMethod(tmp);
}

void Positioning::onOptionsClosed()
{}

void Positioning::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_POSITIONING_METHOD)
        selectPositioningMethod(ANode.value().toString());
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_positioning, Positioning)
#endif

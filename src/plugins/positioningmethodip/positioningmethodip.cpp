#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>

#include "positioningmethodip.h"

PositioningMethodIp::PositioningMethodIp():
    FCurrentState(Stopped),
    FOptionsManager(NULL),
    FPositioning(NULL),
	FConnectionManager(NULL),
	FOptions(NULL),
	FHttpRequester(new QNetworkAccessManager(this))
{}

PositioningMethodIp::~PositioningMethodIp()
{}

//-----------------------------
void PositioningMethodIp::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Positioning Method IP");
	APluginInfo->description = tr("Positioning method, which determines position based on your IP address");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(POSITIONING_UUID);
}

bool PositioningMethodIp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

	QList<IPlugin *> providers = APluginManager->pluginInterface("IPositioningMethodIpProvider");
	if (providers.isEmpty())
		return false;

	for (QList<IPlugin *>::ConstIterator it = providers.constBegin(); it != providers.constEnd(); it++)
	{
		FProviders.insert((*it)->pluginUuid(), qobject_cast<IPositioningMethodIpProvider *>((*it)->instance()));
		connect((*it)->instance(), SIGNAL(newPositionAvailable(GeolocElement)), SLOT(onNewPositionAvailable(GeolocElement)));
		connect((*it)->instance(), SIGNAL(requestError()), SLOT(onRequestError()));
	}

	IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin= APluginManager->pluginInterface("IConnectionManager").value(0,NULL);
	if (plugin)
		FConnectionManager = qobject_cast<IConnectionManager *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
	connect(&FTimer, SIGNAL(timeout()), SLOT(onTimeout()));

	return true;
}

bool PositioningMethodIp::initObjects()
{
	for (QHash<QUuid, IPositioningMethodIpProvider *>::ConstIterator it = FProviders.constBegin(); it != FProviders.constEnd(); it++)
		(*it)->setHttpRequester(&FHttpRequester);
    return true;
}

bool PositioningMethodIp::initSettings()
{
	Options::setDefaultValue(OPV_POSITIONING_METHOD_IP_UPDATERATE, 60);
	Options::setDefaultValue(OPV_POSITIONING_METHOD_IP_PROVIDER, FProviders.keys().first().toString());
	Options::setDefaultValue(OPV_POSITIONING_METHOD_IP_PROXY, APPLICATION_PROXY_REF_UUID);
    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_GEOLOC, OPN_GEOLOC"."+pluginUuid().toString(), MNI_POSITIONING_GEOIP, tr("IP-based")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

bool PositioningMethodIp::select(bool ASelect)
{
	if (ASelect)
	{
		emit stateChanged(FCurrentState = Starting);
		currentProvider()->request();
		int updateRate = Options::node(OPV_POSITIONING_METHOD_IP_UPDATERATE).value().toInt();
		if (updateRate)
			FTimer.start(updateRate*1000);
	}
	else
	{
		emit stateChanged(FCurrentState = Stopping);
		if (FTimer.isActive())
			FTimer.stop();
		emit stateChanged(FCurrentState = Stopped);
	}

	return true;
}

QMultiMap<int, IOptionsDialogWidget *> PositioningMethodIp::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_GEOLOC"."+pluginUuid().toString())
	{
		widgets.insertMulti(OHO_GEOIP_GENERAL, FOptionsManager->newOptionsDialogHeader(tr("General"), AParent));
		widgets.insertMulti(OWO_GEOIP_GENERAL, new PositioningMethodIpOptions(FProviders, AParent));
		widgets.insertMulti(OHO_GEOIP_CONNECTION, FOptionsManager->newOptionsDialogHeader(tr("Connection"), AParent));
		if (FConnectionManager)
			widgets.insertMulti(OWO_GEOIP_CONNECTION, FConnectionManager->proxySettingsWidget(Options::node(OPV_POSITIONING_METHOD_IP_PROXY), AParent));
	}
    return widgets;
}

void PositioningMethodIp::changeCurrentState(IPositioningMethod::State AState)
{
    if (FCurrentState != AState)
		emit stateChanged(FCurrentState = AState);
}

IPositioningMethodIpProvider *PositioningMethodIp::currentProvider() const
{
	return FProviders.value(Options::node(OPV_POSITIONING_METHOD_IP_PROVIDER).value().toString());
}

void PositioningMethodIp::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_POSITIONING_METHOD_IP_PROVIDER));
	onOptionsChanged(Options::node(OPV_POSITIONING_METHOD_IP_UPDATERATE));
	if (FConnectionManager)
		onOptionsChanged(Options::node(OPV_POSITIONING_METHOD_IP_PROXY));
}

void PositioningMethodIp::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_POSITIONING_METHOD_IP_PROVIDER)
	{
		if (FTimer.isActive())
		 currentProvider()->request();
	}
	else if (ANode.path() == OPV_POSITIONING_METHOD_IP_UPDATERATE)
	{
		int updateRate = ANode.value().toInt();
		if (updateRate)
		{
			if (FTimer.isActive())
				FTimer.setInterval(updateRate*1000);
			else if (FCurrentState == Started)
				FTimer.start(updateRate*1000);
		}
		else
			if (FTimer.isActive())
				FTimer.stop();
	}
	else if (ANode.path() == OPV_POSITIONING_METHOD_IP_PROXY)
		FHttpRequester.networkAccessManager()->setProxy(FConnectionManager->proxyById(ANode.value().toString()).proxy);
}

void PositioningMethodIp::onNewPositionAvailable(const GeolocElement &APosition)
{
	if (FCurrentState == Starting)
		emit stateChanged(FCurrentState = Started);
	FCurrentPosition = APosition;
	FCurrentPosition.setReliability(GeolocElement::Reliable);
	emit newPositionAvailable(FCurrentPosition);
}

void PositioningMethodIp::onRequestError()
{
	if (FCurrentState == Starting)
		emit stateChanged(FCurrentState = Stopped);
	else if (FCurrentState == Started)
	{
		FCurrentPosition.setReliability(GeolocElement::WasReliable);
		emit newPositionAvailable(FCurrentPosition);
	}

}

void PositioningMethodIp::onTimeout()
{
	currentProvider()->request();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_positioningmethodip, PositioningMethodIp)
#endif

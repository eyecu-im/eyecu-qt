#include <QDialogButtonBox>
#include <utils/iconstorage.h>
#include <definitions/shortcuts.h>
#include <definitions/toolbargroups.h>
#include "positioningmethodmanual.h"
#include "setlocation.h"

PositioningMethodManual::PositioningMethodManual():
    FCurrentState(Stopped),
    FOptionsManager(NULL),
	FMainWindowPlugin(NULL),
    FMap(NULL),
    FPoi(NULL),
    FOptions(NULL),
    FActionHereIAm(NULL),
	FActionSetPosition(NULL),
    FActionStopPublish(NULL),
    FPoiActionHereIAm(NULL)
{}

PositioningMethodManual::~PositioningMethodManual()
{}

void PositioningMethodManual::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Positioning Method Manual");
    APluginInfo->description = tr("Positioning method, which allows user to specify its location manually");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(POSITIONING_UUID);
}

bool PositioningMethodManual::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
        FMap = qobject_cast<IMap *>(plugin->instance());

	if (!FMap)
	{
		plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
		if (plugin)
			FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
	}

    plugin = APluginManager->pluginInterface("IPoi").value(0,NULL);
    if (plugin)
        FPoi = qobject_cast<IPoi *>(plugin->instance());

    connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));    

    AInitOrder = 150;   // This one should be initialized AFTER ...
    return true;
}

bool PositioningMethodManual::initObjects()
{
    Shortcuts::declareShortcut(SCT_MAP_STOPLOCATIONPUBLICATION, tr("Stop location publication"), tr("Ctrl+R", "Stop publication"), Shortcuts::ApplicationShortcut);
    Shortcuts::declareShortcut(SCT_POI_HEREIAM, tr("Here I am!"), tr("F6", "Here I am! (POI)"), Shortcuts::WidgetShortcut);
    if (FPoi)
        FPoi->insertPoiShortcut(SCT_POI_HEREIAM);
    FIntervalTimer=new QTimer(this);
    return true;
}

bool PositioningMethodManual::initSettings()
{
    Options::setDefaultValue(OPV_POSITIONING_METHOD_MANUAL_COORDINATES, QPointF(0.0, 51.47772));
    Options::setDefaultValue(OPV_POSITIONING_METHOD_MANUAL_TIMESTAMP, QString());
	Options::setDefaultValue(OPV_POSITIONING_METHOD_MANUAL_INTERVAL, 0);

    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_MANUAL, OPN_POSITIONING"."+pluginUuid().toString(), MNI_MANUAL, tr("Manual")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }

    return true;
}

bool PositioningMethodManual::select(bool ASelect)
{
    if (ASelect)
    {
        changeCurrentState(Starting);
		addMenu();
        QString tmp=Options::node(OPV_POSITIONING_METHOD_MANUAL_INTERVAL).value().toString();
        if(tmp!="Disable"){
            if(tmp.toInt()!=0)
                setTimeInterval(tmp.toInt());
        }
        else
            setTimeInterval(0);
        changeCurrentState(Started);
        timerDataSend();
    }
    else
    {
        changeCurrentState(Stopping);
		deleteMenu();
        setTimeInterval(0);
        changeCurrentState(Stopped);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> PositioningMethodManual::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_POSITIONING"."+pluginUuid().toString())
        widgets.insertMulti(OWO_MANUAL, new PositioningMethodManualOptions(AParent));
    return widgets;
}

void PositioningMethodManual::addMenu()
{
    if (FMap)
    {
        if(!FActionHereIAm)
        {
			FActionHereIAm = FMap->addMenuAction(tr("Here I am!"), RSR_STORAGE_MENUICONS, MNI_GEOLOC, 0);
            connect(FActionHereIAm,SIGNAL(triggered(bool)),SLOT(hereIam()));
			FActionStopPublish = FMap->addMenuAction(tr("Stop publication"),QString(RSR_STORAGE_MENUICONS),QString(MNI_GEOLOC_OFF),1);
            FActionStopPublish->setShortcutId(SCT_MAP_STOPLOCATIONPUBLICATION);
            connect(FActionStopPublish,SIGNAL(triggered(bool)),SLOT(retractGeoloc()));
        }
    }
	else if (FMainWindowPlugin)
	{
		if (!FActionSetPosition)
		{
			FActionSetPosition = new Action();
			FActionSetPosition->setText(tr("Set my position"));
			FActionSetPosition->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GEOLOC));
			FActionSetPosition->setEnabled(true);
			FMainWindowPlugin->mainWindow()->topToolBarChanger()->insertAction(FActionSetPosition, TBG_MWTTB_MAPS);
			connect(FActionSetPosition,SIGNAL(triggered(bool)),SLOT(setPosition()));
		}
	}
    if (FPoi)
    {
        if(!FPoiActionHereIAm)
        {
			FPoiActionHereIAm = FPoi->addMenuAction(tr("Here I am!"),QString(RSR_STORAGE_MENUICONS),QString(MNI_GEOLOC));
            connect(FPoiActionHereIAm,SIGNAL(triggered(bool)), SLOT(onPoiActionTriggered()));
        }
    }
}

void PositioningMethodManual::deleteMenu()
{
    if (FMap)
    {
        if (FActionHereIAm)
        {
            FMap->removeMenuAction(FActionHereIAm);
            FActionHereIAm=NULL;
        }
        if (FActionStopPublish)
        {
            FMap->removeMenuAction(FActionStopPublish);
            FActionStopPublish=NULL;
        }
    }
}

void PositioningMethodManual::hereIAmCoords(double ALongitude, double ALatitude)
{
    QPointF data(ALongitude, ALatitude);
    Options::node(OPV_POSITIONING_METHOD_MANUAL_COORDINATES).setValue(data);

    QDateTime timestamp =
#if QT_VERSION >= 0x040700
                            QDateTime::currentDateTimeUtc();
#else
                            QDateTime::currentDateTime().toUTC();
#endif

    QString dtString = timestamp.toString(Qt::ISODate);
    if (dtString.right(1)!="Z") // Workarounf for
        dtString.append('Z');   // missing Z bug
    Options::node(OPV_POSITIONING_METHOD_MANUAL_TIMESTAMP).setValue(dtString);
	if (FActionSetPosition)
		FActionSetPosition->setChecked(true);
}

void PositioningMethodManual::hereIamPoi(QString APoiId)
{
    GeolocElement poi=FPoi->getPoi(APoiId);
	hereIAmCoords(poi.lon(), poi.lat());
}

void PositioningMethodManual::changeCurrentState(IPositioningMethod::State AState)
{
    if (FCurrentState != AState)
        emit stateChanged(FCurrentState = AState);
}

void PositioningMethodManual::hereIam()
{
    if(FMap)
    {
        MercatorCoordinates coords = FMap->menuPopupPosition();
        hereIAmCoords(coords.longitude(), coords.latitude());
	}
}

void PositioningMethodManual::setPosition()
{
	QPointF coords = Options::node(OPV_POSITIONING_METHOD_MANUAL_COORDINATES).value().toPointF();
	MercatorCoordinates coordinates(coords.y(), coords.x());
	SetLocation *setLocation = new SetLocation(coordinates, IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GEOLOC));
	switch (setLocation->exec())
	{
		case SetLocation::Accepted:
			hereIAmCoords(setLocation->coordinates().longitude(), setLocation->coordinates().latitude());
			break;

		case SetLocation::Discarded:
			retractGeoloc();
			break;
	}
}

void PositioningMethodManual::retractGeoloc()
{
	emit newPositionAvailable(GeolocElement());
	if (FActionSetPosition)
		FActionSetPosition->setChecked(false);
}

void PositioningMethodManual::timerDataSend()
{
    newDataSend(Options::node(OPV_POSITIONING_METHOD_MANUAL_COORDINATES).value().toPointF(), Options::node(OPV_POSITIONING_METHOD_MANUAL_TIMESTAMP).value().toString());
}

void PositioningMethodManual::newDataSend(const QPointF &ACoordinates, const QString &ATimeStamp)
{
	GeolocElement position;
	position.setLat(ACoordinates.y());
	position.setLon(ACoordinates.x());
	position.setReliability(GeolocElement::NotReliable);
	position.setTimeStamp(QDateTime::fromString(ATimeStamp, Qt::ISODate));
    emit newPositionAvailable(position);
}

void PositioningMethodManual::setTimeInterval(long timeout)
{
    if (timeout==0)
    {
        disconnect(FIntervalTimer,SIGNAL(timeout()),this,SLOT(timerDataSend()));
        FIntervalTimer->stop();
    }
    else
    {
        FIntervalTimer->setInterval(timeout*1000);// to mls
        if (!FIntervalTimer->isActive())
        {
            connect(FIntervalTimer,SIGNAL(timeout()),SLOT(timerDataSend()));
            FIntervalTimer->start();
        }
    }
}

void PositioningMethodManual::onOptionsOpened()
{}

void PositioningMethodManual::onOptionsClosed(){}

void PositioningMethodManual::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path()==OPV_POSITIONING_METHOD_MANUAL_TIMESTAMP)
        newDataSend(Options::node(OPV_POSITIONING_METHOD_MANUAL_COORDINATES).value().toPointF(), ANode.value().toString());
}

void PositioningMethodManual::onPoiActionTriggered()
{
    Action *action=qobject_cast<Action *>(sender());
    hereIamPoi(action->data(IPoi::ADR_ID).toString());
}

void PositioningMethodManual::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
    if (AId==SCT_POI_HEREIAM)
    {
        QTreeWidget *treeWidget=qobject_cast<QTreeWidget *>(AWidget);
        if (treeWidget)
        {
            QList<QTreeWidgetItem *> selected=treeWidget->selectedItems();
            if (selected.size()==1)
                hereIamPoi(selected[0]->data(0, IPoi::PDR_ID).toString());
        }
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_positioningmethodmanual, PositioningMethodManual)
#endif

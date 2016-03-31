#include <QMessageBox>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <MapObject>

#include <definitions/rosterlabels.h>
#include <definitions/menuicons.h>
#include <definitions/mapicons.h>
#include <definitions/namespaces.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewindowwidgets.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/messagedataroles.h>
#include <definitions/resources.h>
#include <definitions/toolbargroups.h>
#include <definitions/actiongroups.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/messageviewurlhandlerorders.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>

#include <utils/options.h>
#include <utils/menu.h>
#include <utils/logger.h>

#include "poi.h"
#include "newpoi.h"
#include "poioptions.h"
#include "messagepoilist.h"

#define MNO_SHOW_POI    "SHOWPOI"

#define PST_POI         "poi"
#define PSN_POI         "storage:poi"

Poi::Poi():
    FPrivateStorage(NULL),
    FMessageProcessor(NULL),
    FOptionsManager(NULL),
    FAccountManager(NULL),
    FDiscovery(NULL),
    FMainWindowPlugin(NULL),
    FRostersViewPlugin(NULL),
    FIconStorage(NULL),
    FMessageWidgets(NULL),
	FMessageStyleManager(NULL),
	FMap(NULL),
	FGeoMap(NULL),
    FMapLocationSelector(NULL),
    FMapMessage(NULL),
    FTypeStorage(NULL)
{
    if(false)
    {
        tr("10pin");
        tr("Access");
        tr("Adit");
        tr("Administrative");
        tr("Advocacy");
        tr("Aerial way");
        tr("Air force base");
        tr("Airfield");
        tr("Airport");
        tr("Allotments");
        tr("Alpine hut");
        tr("Anime");
        tr("Apron");
        tr("Aqueduct");
        tr("Archaeological");
        tr("Archery");
        tr("Arts centre");
        tr("Athletics");
        tr("Atm");
        tr("Attraction");
        tr("Baby hatch");
        tr("Bahai");
        tr("Bakery");
        tr("Bank");
        tr("Bar");
        tr("Barrier");
        tr("Baseball");
        tr("Basketball");
        tr("Battlefield");
        tr("Bay");
        tr("BBQ");
        tr("Beach");
        tr("Beach resort");
        tr("Beach volleyball");
        tr("Beacon");
        tr("Beauty", "Bauty shop");
        tr("Bench");
        tr("Beverages");
        tr("Bicycle");
        tr("Bicycle designated");
        tr("Biergarten");
        tr("Blizzard");
        tr("Block");
        tr("Board");
        tr("Boatyard");
        tr("Bollard");
        tr("Border control");
        tr("Border point");
        tr("Boule");
        tr("Boundary stone");
        tr("Boutique");
        tr("Break");
        tr("Buddhism");
        tr("Buffer stop");
        tr("Building");
        tr("Bunker");
        tr("Bus station");
        tr("Bus stop");
        tr("Butcher");
        tr("Cable car");
        tr("Cafe");
        tr("Camping");
        tr("Canoe");
        tr("Canal");
        tr("Cape");
        tr("Car");
        tr("Car parts");
        tr("Car repair");
        tr("Car sharing");
        tr("Car wash");
        tr("Caravan");
        tr("Castle");
        tr("Cattle grid");
        tr("Cave entrance");
        tr("Cemetery");
        tr("Centre");
        tr("Chair lift");
        tr("Chalet");
        tr("Chemist");
        tr("Chess");
        tr("Chicane");
        tr("Chimney");
        tr("Church");
        tr("Cinema");
        tr("City");
        tr("City hall");
        tr("City limit");
        tr("Cliff");
        tr("Climbing");
        tr("Clock");
        tr("Clothes");
        tr("Coastline");
        tr("Collapse");
        tr("College");
        tr("Commercial");
        tr("Common");
        tr("Communication center");
        tr("Community centre");
        tr("Computer");
        tr("Confectionery");
        tr("Construction");
        tr("Convenience");
        tr("Copyshop");
        tr("Cosmodrome");
        tr("County");
        tr("Court of law");
        tr("Crane");
        tr("Creek");
        tr("Cricket");
        tr("Croquet");
        tr("Crossing");
        tr("Curtain");
        tr("Cycle barrier");
        tr("Cycling");        
        tr("Dam");
        tr("Danger");
        tr("Dead end");
        tr("Deli");
        tr("Dentist");
        tr("Deprecated");
        tr("Dirt");
        tr("District");
        tr("Ditch");
        tr("Diving");
        tr("Diy store");
        tr("Doctor");
        tr("Dog");
        tr("Drag lift");
        tr("Drain");
        tr("Drinking water");
        tr("Driving school");
        tr("Earthquake");
        tr("Electronics");
        tr("Embassy");
        tr("Emergency access point");
        tr("Emergency phone");
        tr("Entrance");
        tr("Erotic");
        tr("Eruption");
        tr("Exchange");
        tr("Excrement bags");
        tr("Exit");
        tr("Fabric");
        tr("Farm");
        tr("Fastfood");
        tr("Ferry");
        tr("Fire");
        tr("Fire hydrant");
        tr("Fire brigade");
        tr("Fishing");
        tr("Flag");
        tr("Flood");
        tr("Florist");
        tr("Fog");
        tr("Food court");
        tr("Foot");
        tr("Football");
        tr("Foot designated");
        tr("Ford");
        tr("Fossil");
        tr("Frame");
        tr("Fruits");
        tr("Fuel");
        tr("Furniture");
        tr("Garages");
        tr("Garden");
        tr("Garden centre");
        tr("Gasometer");
        tr("Gate");
        tr("Geyser");
        tr("Gift");
        tr("Glacier");
        tr("Golf");
        tr("Gondola");
        tr("Goods");
        tr("Government");
        tr("Grad");
        tr("Greengrocer");
        tr("Greenhouse horticulture");
        tr("Grit bin");
        tr("Groyne");
        tr("Guest house");
        tr("Guidepost");
        tr("Gym");
        tr("Hairdresser");
        tr("Hamlet");
        tr("Handball");
        tr("Hangar");
        tr("Hardware");
        tr("Hearing aids");
        tr("Helipad");
        tr("Hi-Fi");
        tr("Highway");
        tr("Hill");
        tr("Hinduism");
        tr("Hockey");
        tr("Horse");
        tr("Horse designated");
        tr("Hospital");
        tr("Hostel");
        tr("House");
        tr("House number");
        tr("Hotel");
        tr("Hunting stand");
        tr("Hydro");
        tr("Icecream");
        tr("Ice rink");
        tr("Incline");
        tr("Information");
        tr("Information office");
        tr("Island");
        tr("Islet");
        tr("Isolated dwelling");
        tr("Jainism");
        tr("Jewelry");
        tr("Jewish");
        tr("Karting");
        tr("KFC");
        tr("Kindergarten");
        tr("Kiosk");
        tr("Kitchen");
        tr("Laundry");
        tr("Library");
        tr("Lift gate");
        tr("Lighthouse");
        tr("Locality");
        tr("Lock gate");
        tr("Locust");
        tr("Mall");
        tr("Map");
        tr("Marina");
        tr("Marketplace");
        tr("Maximum height");
        tr("Maximum length");
        tr("Maximum weight");
        tr("Maximum width");
        tr("McDonald's");
        tr("Measurement station");
        tr("Memorial");
        tr("Military base");
        tr("Military stock");
        tr("Mine");
        tr("Minimum speed");
        tr("Misty howler");
        tr("Mobile phone");
        tr("Monument");
        tr("Motel");
        tr("Motocross");
        tr("Motor");
        tr("Motorway");
        tr("Motorway link");
        tr("Motorbike");
        tr("Motor car");
        tr("Mountain pass");
        tr("Mud");
        tr("Mudflow");
        tr("Multi");
        tr("Multi storey");
        tr("Municipality");
        tr("Museum");
        tr("Musical instrument");
        tr("Muslim");
        tr("Nature reserve");
        tr("Naval base");
        tr("Nightclub");
        tr("No left turn");
        tr("No right turn");
        tr("No straight on");
        tr("No U-turn");
        tr("None");
        tr("Note");
        tr("Nuclear");
        tr("Oil");
        tr("Only left turn");
        tr("Only right turn");
        tr("Only straight on");
        tr("Optician");
        tr("Outdoor");
        tr("Paint");
        tr("Palaeontological site");
        tr("Park");
        tr("Park and ride");
        tr("Parking");
        tr("Parking entrance");
        tr("Parking exit");
        tr("Path");
        tr("Passing place");
        tr("Peak");
        tr("Pedestrian");
        tr("Pelota");
        tr("Pet");
        tr("Pharmacy");
        tr("Picnic");
        tr("Pier");
        tr("Pitch");
        tr("Plant");
        tr("Platform");
        tr("Playground");
        tr("Pole");
        tr("Police");
        tr("Polygon");
        tr("Pool");
        tr("Post box");
        tr("Post office");
        tr("Power");
        tr("Prison");
        tr("Primary");
        tr("Primary link");
        tr("PSV");
        tr("Pub");
        tr("Racquetball");
        tr("Radar");
        tr("Railway");
        tr("Railway small");
        tr("Railway station");
        tr("Rain");
        tr("Range");
        tr("Recycling");
        tr("Reef");
        tr("Rental");
        tr("Reservoir covered");
        tr("Residential");
        tr("Restaurant");
        tr("Restricted area");
        tr("Restriction");
        tr("Riding");
        tr("River");
        tr("River bank");
        tr("Road");
        tr("Rock");
        tr("Roundabout left");
        tr("Roundabout right");
        tr("Rowing");
        tr("Ruins");
        tr("Runway");
        tr("Safety training");
        tr("Sally port");
        tr("Sauna");
        tr("School");
        tr("Seafood");
        tr("Secondary");
        tr("Secondary link");
        tr("Senate");
        tr("Services");
        tr("Shelter");
        tr("Shinto");
        tr("Shoes");
        tr("Sikhism");
        tr("Skateboard");
        tr("Skating");
        tr("Skiing");
        tr("Slipway");
        tr("Snow");
        tr("Soccer");
        tr("Speed");
        tr("Speed trap");
        tr("Spring");
        tr("Stadium");
        tr("State");
        tr("Station");
        tr("Stationery");
        tr("Steps");
        tr("Stile");
        tr("Stop");
        tr("Stop position");
        tr("Storm");
        tr("Stream");
        tr("Street lamp");
        tr("Stripclub");
        tr("Studio");
        tr("Suburbs");
        tr("Subway");
        tr("Supermarket");
        tr("Surveillance");
        tr("Survey point");
        tr("Table tennis");
        tr("Tailor");
        tr("Tank");
        tr("Taoism");
        tr("Taxi");
        tr("Taxiway");
        tr("Telephone");
        tr("Tennis");
        tr("Terminal");
        tr("Tertiary");
        tr("Tertiary link");
        tr("Theater");
        tr("Theme park");
        tr("Ticket machine");
        tr("Toilets");
        tr("Toll booth");
        tr("Town");
        tr("Town Hall");
        tr("Tower");
        tr("Tower small");
        tr("Toys");
        tr("Track");
        tr("Trade");
        tr("Traffic light");
        tr("Tram");
        tr("Trash bin");
        tr("Travel agency");
        tr("Trees");
        tr("Trunk");
        tr("Trunk link");
        tr("Tsunami");
        tr("Turning");
        tr("Turning circle");
        tr("Turnstile");
        tr("Turntable");
        tr("Tyres");
        tr("Unclassified");
        tr("Underground");
        tr("University");
        tr("Vacuum cleaner");
        tr("Variety store");
        tr("Veterinary");
        tr("Viaduct");
        tr("Video");
        tr("Viewpoint");
        tr("Village");
        tr("Volcano");
        tr("Volleyball");
        tr("Waste disposal");
        tr("Wastewater");
        tr("Wastewater plant");
        tr("Water");
        tr("Water park");
        tr("Water tower");
        tr("Water well");
        tr("Water works");
        tr("Waterfall");
        tr("Watermill");
        tr("Waterway");
        tr("Wayside cross");
        tr("Wayside shrine");
        tr("Weapons");
        tr("Weir");
        tr("Well");
        tr("Wifi");
        tr("Wind");
        tr("Windmill");
        tr("Windsock");
        tr("Works");
        tr("Zebra crossing");
        tr("Zoo");
    }

    if(false)
    {
         tr("Accommodation");
         tr("Administration");
         tr("Communication points");
         tr("Disaster");
         tr("Education");
         tr("Food");
         tr("Geographic");
         tr("Health");
         tr("Historic");
         tr("Industrial");
         tr("Leisure");
         tr("Miscellaneous");
         tr("Money");
         tr("Nature");
         tr("Nautical");
         tr("None");
         tr("Place");
         tr("Religion");
         tr("Service");
         tr("Shop");
         tr("Sightseeing");
         tr("Signs of traffic");
         tr("Special");
         tr("Sport");
         tr("Public transport");
         tr("Vehicle");
    }
}

Poi::~Poi()
{}

//-----------------------------
void Poi::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Points of Interest");
    APluginInfo->description = tr("Allows to create, store and handle user's Points of Interest");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(PRIVATESTORAGE_UUID);
}

bool Poi::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IPrivateStorage").value(0);
    if (plugin)
    {
        FPrivateStorage = qobject_cast<IPrivateStorage *>(plugin->instance());
        if (FPrivateStorage)
        {
                connect(FPrivateStorage->instance(),SIGNAL(storageOpened(const Jid &)),SLOT(onPrivateStorageOpened(const Jid &)));
                connect(FPrivateStorage->instance(),SIGNAL(dataSaved(const QString &, const Jid &, const QDomElement &)),
                        SLOT(onPrivateDataSaved(const QString &, const Jid &, const QDomElement &)));
                connect(FPrivateStorage->instance(),SIGNAL(dataLoaded(const QString &, const Jid &, const QDomElement &)),
                        SLOT(onPrivateDataLoaded(const QString &, const Jid &, const QDomElement &)));
                connect(FPrivateStorage->instance(),SIGNAL(dataChanged(const Jid &, const QString &, const QString &)),
                        SLOT(onPrivateDataChanged(const Jid &, const QString &, const QString &)));
                connect(FPrivateStorage->instance(),SIGNAL(storageClosed(const Jid &)),SLOT(onPrivateStorageClosed(const Jid &)));
        }
    }

    plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
    if (plugin)
        FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
    if (plugin)
        FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
    if (plugin)
    {
        FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
        if (FRostersViewPlugin)
        {
            connect(FRostersViewPlugin->rostersView()->instance(),
                    SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
                    SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));
            connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
        }
    }

    plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
    if (plugin)
    {
        FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
        if (FMessageWidgets)
        {
            connect(FMessageWidgets->instance(), SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
            connect(FMessageWidgets->instance(),SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onNormalWindowCreated(IMessageNormalWindow *)));
        }
    }

	plugin = APluginManager->pluginInterface("IMessageStyleManager").value(0,NULL);
    if (plugin)
		FMessageStyleManager = qobject_cast<IMessageStyleManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
    if (plugin)
	{
		FMap = qobject_cast<IMap *>(plugin->instance());
		FGeoMap = FMap->geoMap();
	}

    plugin = APluginManager->pluginInterface("IMapLocationSelector").value(0,NULL);
    if (plugin)
        FMapLocationSelector = qobject_cast<IMapLocationSelector *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMapMessage").value(0,NULL);
    if (plugin)
        FMapMessage = qobject_cast<IMapMessage *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
    if (plugin)
        FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
    if (plugin)
        FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());        

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    connect(this, SIGNAL(poisLoaded(QString,PoiHash)), SLOT(showStreamPoi(QString)));
    connect(this, SIGNAL(poisRemoved(QString)), SLOT(hideStreamPoi(QString)));

    AInitOrder = 100;   // This one should be initialized AFTER Location!
    return true;

}

bool Poi::initObjects()
{
    // Shortcuts
    Shortcuts::declareGroup(SCTG_POI, tr("Points of Interest"), SGO_POI);
    Shortcuts::declareShortcut(SCT_POI_VIEW, tr("Toggle view POIs on the map"), tr("Alt+V", "Toggle view POIs on the map"), Shortcuts::ApplicationShortcut);
    Shortcuts::declareShortcut(SCT_POI_LIST, tr("Display POI list for all accounts"), tr("Ctrl+P", "POI list (all)"), Shortcuts::ApplicationShortcut);
    Shortcuts::declareShortcut(SCT_POI_LISTACCOUNT, tr("Display POI list for the account"), tr("Alt+P", "POI list (account)"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_POI_SHOW, tr("Show"), tr("Space", "Show POI"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_POI_SAVE, tr("Save"), tr("F2", "Save POI"), Shortcuts::WidgetShortcut);    
    Shortcuts::declareShortcut(SCT_POI_EDIT, tr("Edit"), tr("F2", "Edit POI"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_POI_DELETE, tr("Delete"), tr("Del", "Delete POI"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_POI_REMOVE, tr("Remove"), tr("Del", "Remove POI"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_POI_OPENURL, tr("Open URL"), tr("F3", "Open URL in the POI"), Shortcuts::WidgetShortcut);
    Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_POI, tr("Points of Interest"), SGO_POI);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_POI_INSERT, tr("Insert POI"), tr("Alt+P", "Insert POI (into message)"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_POI_INSERTLOCATION, tr("Insert location"), tr("Alt+G", "Insert location"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_POI_EDIT, tr("Edit POI"), tr("Return", "Edit POI (in message)"), Shortcuts::WidgetShortcut);
    Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_POI_DELETE, tr("Delete POI"), tr("Del", "Delete POI (from message)"), Shortcuts::WidgetShortcut);

    qRegisterMetaType<GeolocElement>("GeolocElement");

    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
    FTypeStorage = IconStorage::staticStorage(RSR_STORAGE_POI_TYPE);

    loadPoiTypes();

    if (FDiscovery)
        registerDiscoFeatures();

    if (FMessageProcessor)
    {
        FMessageProcessor->insertMessageWriter(MWO_POI, this);
        FMessageProcessor->insertMessageEditor(MEO_POI, this);
    }

	if (FMap)
    {
		FGeoMap->setObjectHandler(MOT_POI, this);
		FGeoMap->registerDataType(MDR_POI_ICON, MOT_POI, 100, MOP_NONE, MOP_CENTER);
		FGeoMap->registerDataType(MDR_POI_LABEL, MOT_POI, 110, MOP_NONE, MOP_RIGHT);
		FGeoMap->addDataHolder(MOT_POI, this);
        if (FMessageWidgets)
            FMessageWidgets->insertViewUrlHandler(MVUHO_POI, this);
    }

    if (FMapMessage)
        FMapMessage->insertUrlHandler(MVUHO_POI, this);

    if (FMainWindowPlugin)
    {
        FMenuToolbar = new Menu();
        FMenuToolbar->setTitle(tr("POI"));
		FMenuToolbar->setIcon(RSR_STORAGE_MENUICONS, MNI_POI_TLB);
        FMenuToolbar->menuAction()->setEnabled(true);

        IMainWindow *mainWindow = FMainWindowPlugin->mainWindow();
        mainWindow ->topToolBarChanger()    // Get toolbar changer
                   ->insertAction(FMenuToolbar->menuAction(), TBG_MWTTB_POI) // Add action as a button
                   ->setPopupMode(QToolButton::InstantPopup);

        Action *action = new Action();
        action->setText(tr("POI list"));
		action->setIcon(RSR_STORAGE_MENUICONS, MNI_POI_VIEW);
        action->setShortcutId(SCT_POI_LIST);
        connect(action, SIGNAL(triggered(bool)), SLOT(onPoiList(bool)));

        FMenuToolbar->addAction(action, AG_POI_MENU_COMMON, false);
        FMenuToolbar->addSeparator();

        action = new Action();
        action->setCheckable(true);
        action->setText(tr("Show POI"));
		action->setIcon(RSR_STORAGE_MENUICONS, MNI_POI);
        action->setData(Action::DR_UserDefined, MNO_SHOW_POI);
        action->setShortcutId(SCT_POI_VIEW);
		connect(action, SIGNAL(triggered()), SLOT(onPoiShow()));

        FMenuToolbar->addAction(action, AG_POI_MENU_COMMON, false);
    }

    if (FRostersViewPlugin)
        Shortcuts::insertWidgetShortcut(SCT_POI_LISTACCOUNT, FRostersViewPlugin->rostersView()->instance());

    return true;
}

bool Poi::initSettings()
{
    Options::setDefaultValue(OPV_POI_SHOW, true);
    Options::setDefaultValue(OPV_POI_CUR_COUNTRY,0);
    Options::setDefaultValue(OPV_POI_TYPE,0);

    Options::setDefaultValue(OPV_POI_PNT_ICONSIZE,16);                          // Not implemented yet
    Options::setDefaultValue(OPV_POI_PNT_TEXTCOLOR, QColor(Qt::cyan));          // Text color
    Options::setDefaultValue(OPV_POI_PNT_TEMPTEXTCOLOR, QColor(Qt::yellow));    // Text color
    Options::setDefaultValue(OPV_POI_PNT_SHADOWCOLOR, QColor(Qt::gray));        // Shadow color
    Options::setDefaultValue(OPV_POI_PNT_FONT, QFont("Times", 12, QFont::Normal, false));   // Font

    Options::setDefaultValue(OPV_POI_FILTER, getAllTypes());

    // Account-specific options
    Options::setDefaultValue(OPV_ACCOUNT_ENABLEPOI, true);    // Enable POI for sp

    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_POI, OPN_POI, MNI_POI, tr("Points of Interest")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Poi::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_POI)
        widgets.insertMulti(OWO_POI, new PoiOptions(this, AParent));
    else    // Add "Enable Points of Interest" option to account settings page
    {
        QStringList nodeTree = ANodeId.split(".", QString::SkipEmptyParts);
		if (FOptionsManager && nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="Additional")
        {
            OptionsNode aoptions = Options::node(OPV_ACCOUNT_ITEM, nodeTree.at(1));
			widgets.insertMulti(OWO_ACCOUNTS_ADDITIONAL_POI, FOptionsManager->newOptionsDialogWidget(aoptions.node(OPV_ENABLEPOI), tr("Enable Points of Interest"), AParent));
        }
    }
    return widgets;
}

void Poi::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_POI_FILTER));
    onOptionsChanged(Options::node(OPV_POI_SHOW));
}

void Poi::onOptionsClosed()
{}

void Poi::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path()==OPV_POI_SHOW)
    {
        QMultiHash<int, QVariant> data;
        data.insert(Action::DR_UserDefined, MNO_SHOW_POI);
        Action *action=FMenuToolbar->findActions(data).first();

        if (ANode.value().toBool())
        {
            showAllPoi();
            action->setChecked(true);
        }
        else
        {
            hideAllPoi();
            action->setChecked(false);
        }
    }
    else if (ANode.path()==OPV_POI_PNT_ICONSIZE)
		FGeoMap->updateObjects(MOT_POI, MDR_POI_ICON);
    else if (ANode.path()==OPV_POI_PNT_TEXTCOLOR ||
             ANode.path()==OPV_POI_PNT_SHADOWCOLOR ||
             ANode.path()==OPV_POI_PNT_FONT)
		FGeoMap->updateObjects(MOT_POI, MDR_POI_LABEL);
    else if (ANode.path()==OPV_POI_PNT_TEMPTEXTCOLOR)
    {
        if (FTempPois.contains("temporary"))
			FGeoMap->updateObject(MOT_POI, "temporary", MDR_POI_LABEL);
    }
    else if (ANode.path()==OPV_POI_FILTER)
    {
        FPoiFilter = ANode.value().toStringList();
        updateAllPoi();
    }
    // It's an account-specific option!
    else if (ANode.name()==OPV_ENABLEPOI &&
        ANode.parent().name()=="account" &&
        ANode.parent().parent().name()=="accounts")
    {
        QString AccountID=ANode.parent().nspace();
		IAccount *account=FAccountManager->findAccountById(AccountID);
        if (account)
        {
            Jid streamJid=account->streamJid();
            QString bareJid=streamJid.bare();

            if (ANode.value().toBool()) // Enabled
            {
                FPoiAccounts.append(account);
                if (!FStreamPoi.contains(bareJid))
                {
                    FStreamPoi.append(bareJid);
                    loadPoiList(streamJid);
                    if (FStreamPoi.count()==1)
                        addMenuMap();
                }
            }
            else                        // Disabled
            {
                FPoiAccounts.removeAll(account);
                for (QList<IAccount *>::const_iterator it=FPoiAccounts.constBegin(); it!=FPoiAccounts.constEnd(); it++)
                    if ((*it)->streamJid().bare() == bareJid)
                        return; // More accounts with the same bare JID exist
                FStreamPoi.removeAll(bareJid);
                emit poisRemoved(bareJid);
                if (FStreamPoi.count()==0)
                    deleteMenuMap();
            }
        }
    }
}

// Global "Show POI" button clicked
void Poi::onPoiShow()
{
    OptionsNode node=Options::node(OPV_POI_SHOW);
    node.setValue(!node.value().toBool()); // Tgoggle "Show POI" setting
}

void Poi::registerDiscoFeatures()
{
    IDiscoFeature dfeature;
    dfeature.var = NS_X_POI;
    dfeature.active = true;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_POI);
    dfeature.name = tr("Points of Interest");
    dfeature.description = tr("Allows to store and exchange with Points of Interest");
    FDiscovery->insertDiscoFeature(dfeature);
    dfeature.var = NS_PEP_GEOLOC;
    dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GEOLOC);
    dfeature.name = tr("User Location");
    dfeature.description = tr("Supports user Geolocation");
    FDiscovery->insertDiscoFeature(dfeature);
}

MessagePoiList *Poi::findPoiList(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType) const
{
    if (AMessageType==Message::Chat)
        return getPoiList(FMessageWidgets->findChatWindow(AStreamJid, AContactJid));
    else
        return getPoiList(FMessageWidgets->findNormalWindow(AStreamJid, AContactJid));
}

MessagePoiList *Poi::getPoiList(const IMessageNormalWindow *AWindow) const
{
    return AWindow?qobject_cast<MessagePoiList *>(AWindow->messageWidgetsBox()->widgetByOrder(MNWW_POILISTWIDGET)):NULL;
}

MessagePoiList *Poi::getPoiList(const IMessageChatWindow *AWindow) const
{
    return AWindow?qobject_cast<MessagePoiList *>(AWindow->messageWidgetsBox()->widgetByOrder(MCWW_POILISTWIDGET)):NULL;
}

QString Poi::locationString(const QDomElement &ALocation) const
{
    QString location;
    if (!ALocation.firstChildElement("lat").isNull())
    {
        QString lat(ALocation.firstChildElement("lat").text());
        location.append(tr("Lat")).append(": ").append((lat[0]=='-')?tr("%1S").arg(lat.mid(1, 8)):tr("%1N").arg(lat.mid(0, 8)));
    }

    if (!ALocation.firstChildElement("lon").isNull())
    {
        if (!location.isEmpty())
            location.append("; ");
        QString lon(ALocation.firstChildElement("lon").text());
        location.append(tr("Lon")).append(": ").append((lon[0]=='-')?tr("%1W").arg(lon.mid(1, 8)):tr("%1E").arg(lon.mid(0, 8)));
    }
    return location;
}

bool Poi::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
    return FDiscovery==NULL || !FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
            || (FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_PEP_GEOLOC)
                && FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_X_POI)); // ??
}

GeolocElement Poi::getPoi(const QString &AId) const
{
    QStringList splitted = AId.split(':');
    if (splitted.size()==1)
        return FTempPois.value(AId);
    else
        if (FGeolocHash.contains(splitted[0]))
            return FGeolocHash[splitted[0]].value(splitted[1]);
        else
            return GeolocElement();
}

bool Poi::putPoi(const QString &AId, const GeolocElement &APoiData, const QString &ABareJid)
{
    int type=FGeolocHash[ABareJid].contains(AId)?PMT_MODIFIED:PMT_ADDED;
    FGeolocHash[ABareJid].insert(AId, APoiData);
    QString id=QString("%1:%2").arg(ABareJid).arg(AId);
    showSinglePoi(id);
    emit poiModified(id, type);
    Jid streamJid=findStreamJid(ABareJid);
    if (streamJid.isValid())
        return savePoiList(streamJid);
    return false;
}

bool Poi::putPoi(const QString &AId, const GeolocElement &APoiData, bool AShow)
{
    int type=FTempPois.contains(AId)?PMT_MODIFIED:PMT_ADDED;
    FTempPois.insert(AId, APoiData);
    if (AShow)
        showSinglePoi(AId);
    else
		if (FGeoMap->isObjectExists(MOT_POI, AId))
			FGeoMap->updateObject(MOT_POI, AId);

    emit poiModified(AId, type);
    return true;
}

bool Poi::removePoi(const QString &AId)
{
    QStringList splitted = AId.split(':');
    if (splitted.size()==1)
    {
        if(FTempPois.contains(AId))
        {
            FTempPois.remove(AId);
			FGeoMap->removeObject(MOT_POI, AId);
            emit poiModified(AId, PMT_REMOVED);
            return true;
        }
    }
    else
    {
        QString bareJid=splitted[0];
        if (FGeolocHash.contains(bareJid))
        {
            if(FGeolocHash[bareJid].contains(splitted[1]))
            {
                FGeolocHash[bareJid].remove(splitted[1]);
                Jid streamJid=findStreamJid(bareJid);
                if (streamJid.isValid())
                    if (savePoiList(streamJid))
                    {
						FGeoMap->removeObject(MOT_POI, AId);
                        emit poiModified(AId, PMT_REMOVED);
                        return true;
                    }
            }
        }
    }
    return false;
}

Action *Poi::addMenuAction(QString text, QString icon, QString keyIcon)
{
    Action *action = new Action();
    action->setText(text);
    action->setIcon(icon, keyIcon);
    FActions.append(action);
    return action;
}

void Poi::removeMenuAction(Action *action)
{
    FActions.removeOne(action);
}

QString Poi::parsePOI(const Message &AMessage)
{
    // Find "<x />" element with POI namespace in stanza
    QDomElement x = AMessage.stanza().firstElement("x", NS_X_POI);
    if (x.isNull()) // Not found
        return QString();

    // Find "<geoloc />" element with geoloc namespace in the "<x />" element
    QDomElement poi = x.firstChildElement("geoloc");//    AStanza.firstElement("geoloc"," NS_PEP_GEOLOC");
    if (poi.isNull()) // Not found
        return QString();

    // "<geoloc />" element found! Now, let's build POI HTML code
    QString htmlPoi;
    QString hash16;

    int direction=AMessage.data(MDR_MESSAGE_DIRECTION).toInt();
    if (direction==IMessageProcessor::DirectionIn)
    {
        QDomDocument doc;
        doc.appendChild(doc.importNode(poi, true));
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(doc.toByteArray());
        hash16=hash.result().toHex();
        FExtGeolocHash.insert(hash16, GeolocElement(poi));
    }

    // Build location element...    
    QString coord;
    QString location = locationString(poi);
    if (!location.isEmpty())
        coord = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\" > <i>%3</i></a>")
                    .arg(getIconFileName("geoloc")).arg(tr("Location")).arg(location);

    // Build POI title element...
    if (direction==IMessageProcessor::DirectionIn)
        htmlPoi.append(QString("<a href='poi:%1'>").arg(hash16, 0, 16));

    // Build URL element...
    QDomElement uri=poi.firstChildElement("uri");
    QString urlString;
    if (!uri.isNull())
    {
		QUrl url;
#if QT_VERSION >= 0x050000
		url.fromEncoded(uri.text().toLatin1(), QUrl::StrictMode);
#else
        url.setEncodedUrl(uri.text().toLatin1(), QUrl::StrictMode);
#endif
        urlString = QString("<br><img src=\"%1\" title=\"%2\" alt=\"%2\" /> <a href=\"%3\">%3</a>")
                       .arg(FIconStorage->fileFullName(MNI_LINK))
                       .arg(tr("Link"))
                       .arg(url.toString());
    }

    // Build description element...
    QString descr;
    QString tmp = poi.firstChildElement("description").text();
    if(!tmp.isEmpty())
        descr = QString("<br><img src=\"%1\" title=\"%2\" alt=\"%2\" /> %3")
                        .arg(FIconStorage->fileFullName(MNI_DESCRIPTION))
                        .arg(tr("Description"))
                        .arg(tmp);

    // Build type icon subelement...

    QString type = poi.firstChildElement("type").text();
    QString icon = QString("<img src=\"%1\" title=\"%2\" alt=\"%2\">")
        .arg(getTypeIconFileName(type)).arg(FTranslatedTypes.value(type));

    // Append type icon subelement
    htmlPoi.append(icon);

    // Append POI name, if any
    if (!poi.firstChildElement("text").isNull())
        htmlPoi.append(" ")
               .append(poi.firstChildElement("text").text());

    if (direction==IMessageProcessor::DirectionIn)
        htmlPoi.append("</a>");
    htmlPoi.append("<br>");
    if (direction==IMessageProcessor::DirectionIn)
        htmlPoi.append(QString("<a href=\"geoloc:%1\">").arg(hash16));

    // Append the rest of POI data
    htmlPoi.append(coord);  // Geolocation coordinates
    if (direction==IMessageProcessor::DirectionIn)
        htmlPoi.append("</a>");
    htmlPoi.append(descr)   // Description
           .append(urlString);    // URL

    return htmlPoi;
}

// Protected SLOTS
void Poi::onNormalWindowCreated(IMessageNormalWindow *AWindow)
{
    Jid contactJid = AWindow->contactJid();
    Jid streamJid  = AWindow->streamJid();

    if(AWindow->mode()==IMessageNormalWindow::WriteMode && isSupported(streamJid, contactJid))
    {
        Menu *menu = new Menu(new MessagePoiList(this, FMapLocationSelector, &FPoiAccounts, FIconStorage, AWindow->instance()));
        menu->setTitle(tr("Add POI"));
		menu->setIcon(RSR_STORAGE_MENUICONS, MNI_POI_TLB);
        menu->menuAction()->setEnabled(true);

        Action *action = new Action(menu);
        action->setText(tr("Insert POI"));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_POI_TLB);
        action->setData(ADR_MESSAGE_TYPE, Message::Normal);
        action->setData(ADR_CONTACT_JID, contactJid.full());
        action->setData(ADR_STREAM_JID, streamJid.full());
        action->setShortcutId(SCT_MESSAGEWINDOWS_POI_INSERT);
        connect(action, SIGNAL(triggered(bool)), SLOT(onInsertPoi(bool)));
        menu->addAction(action, AG_PERSISTENT, false);

        action = new Action(menu);
        action->setText(tr("Insert location"));
		action->setIcon(RSR_STORAGE_MAPICONS,MPI_NEWCENTER);
        action->setData(ADR_MESSAGE_TYPE, Message::Normal);
        action->setData(ADR_CONTACT_JID, contactJid.full());
        action->setData(ADR_STREAM_JID, streamJid.full());
        action->setShortcutId(SCT_MESSAGEWINDOWS_POI_INSERTLOCATION);
        connect(action, SIGNAL(triggered(bool)), SLOT(onInsertLocation(bool)));
        menu->addAction(action, AG_PERSISTENT, false);

        AWindow->toolBarWidget()->toolBarChanger()
                                ->insertAction(menu->menuAction(),TBG_MWTBW_POI_VIEW)
                                ->setPopupMode(QToolButton::InstantPopup);
    }
}

void Poi::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

    IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
    IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
    if (window)
        updateChatWindowActions(window);
}

void Poi::onChatWindowCreated(IMessageChatWindow *AWindow)
{
    new MessagePoiList(this, FMapLocationSelector, &FPoiAccounts, FIconStorage, AWindow->instance());
    updateChatWindowActions(AWindow);
    connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
}

void  Poi::onInsertLocation(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    MessagePoiList *messagePoiList=qobject_cast<MessagePoiList *>(action->parentWidget()->parentWidget());
    if (action)
    {        
        NewPoi *newPoi  = new NewPoi(this, FMapLocationSelector, FPoiAccounts, tr("Insert location"));
        newPoi->initStreamList(QString(), false);
        newPoi->allowEmptyName(true);
		newPoi->setLocation(FMap->mapCenter());
        if(newPoi->exec())
            messagePoiList->addPoi(newPoi->getPoi());
        newPoi->deleteLater();
    }
}

void  Poi::onInsertPoi(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    MessagePoiList *messagePoiList=qobject_cast<MessagePoiList *>(action->parentWidget()->parentWidget());
    if (messagePoiList)
    {       
        QString id(action->data(ADR_STREAM_JID).toString());
        id.append("+").append(action->data(ADR_CONTACT_JID).toString());
        if (FPoiInsertLists.contains(id))
            FPoiInsertLists[id]->activateWindow();
        else
        {
            PoiList *poiList=*FPoiInsertLists.insert(id, new PoiList(this));
            poiList->setWindowTitle(tr("Insert point of interest"));
            poiList->setWindowIcon(getIcon(MNI_POI_ADD));
            poiList->fillTable(FGeolocHash);

            if (poiList->exec())
            {
                QStringList splitted=poiList->selectedId().split(':');
                messagePoiList->addPoi(FGeolocHash[splitted[0]][splitted[1]]);
            }
            FPoiInsertLists.take(id)->deleteLater();
        }
    }
}

void Poi::insertMessagePoi(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType, const GeolocElement &APoi)
{
    MessagePoiList *messagePoiList=findPoiList(AStreamJid, AContactJid, AMessageType);
    if (messagePoiList)
        messagePoiList->addPoi(APoi);
}

void Poi::addPoiToMessage(Message &AMessage, GeolocElement &element)
{
    QDomElement x=AMessage.stanza().document().createElementNS(NS_X_POI, "x");
    element.exportElement(x);
    AMessage.stanza().element().appendChild(x);
}

void Poi::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
    if (ALabelId == AdvancedDelegateItem::DisplayId)
	{
		QStringList bareJids;
        for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
			if ((*it)->kind() == RIK_STREAM_ROOT)
            {
                Jid streamJid = (*it)->data(RDR_STREAM_JID).toString();
				IAccount *account=FAccountManager->findAccountByStream(streamJid);
                if (FPoiAccounts.contains(account))
					bareJids.append(streamJid.bare());
           }

		if (!bareJids.isEmpty())
		{
			Action *action = new Action(AMenu);
			action->setData(ADR_STREAM_JID, bareJids);
			action->setText(tr("POI list"));
			action->setIcon(RSR_STORAGE_MENUICONS,MNI_POI_VIEW);
			action->setDisabled(FPoiList);
			connect(action, SIGNAL(triggered(bool)), SLOT(onPoiList(bool)));
			AMenu->addAction(action, AG_RVCM_VIEW_POI, true);
		}
	}
}

void Poi::addMenuMap()
{
	if (FMap)
    {
		FActionNewPOI = FMap->addMenuAction(tr("Add POI"), QString(RSR_STORAGE_MENUICONS),QString(MNI_POI_ADD),0);
        connect(FActionNewPOI, SIGNAL(triggered()), SLOT(addNewPoi()));
    }
}

void Poi::deleteMenuMap()
{
	if (FMap)
        if (FActionNewPOI)
        {
			FMap->removeMenuAction(FActionNewPOI);
            FActionNewPOI=NULL;
        }
}

//-------------------------------------------------------

void Poi::showStreamPoi(const QString &ABareJid)
{
    PoiHash Dg = FGeolocHash.value(ABareJid);
    if(!Dg.isEmpty())
		if(FMap)
        {
            QString id(ABareJid);
            id.append(":%1");
            for(PoiHash::const_iterator it=Dg.constBegin(); it!=Dg.constEnd(); it++)
                showSinglePoi(id.arg(it.key()));
        }
}

void Poi::showSinglePoi(QString AId)
{
    QStringList splitted=AId.split(':');
    if (Options::node(OPV_POI_SHOW).value().toBool() || splitted.size()<2)
    {        
        GeolocElement geolocElement=splitted.size()>1?FGeolocHash[splitted[0]][splitted[1]]:FTempPois[AId];
        if(isTypeEnabled(geolocElement.hasProperty("type")?geolocElement.type():"none"))
        {
			FGeoMap->addObject(MOT_POI, AId, geolocElement.coordinates()); // MercatorCoordinates(geolocElement.value("lat").toDouble(), geolocElement.value("lon").toDouble())
            emit mapDataChanged(MOT_POI, AId, MDR_ALL);
        }
    }
}

void Poi::hideOnePoi(QString AId)
{
	FGeoMap->removeObject(MOT_POI, AId);
}

void Poi::updateOnePoi(QString AId)
{
    QStringList splitted=AId.split(':');
    if (Options::node(OPV_POI_SHOW).value().toBool() || splitted.size()<2)
    {
        GeolocElement geolocElement=splitted.size()>1?FGeolocHash[splitted[0]][splitted[1]]:FTempPois[AId];
        QString type=geolocElement.hasProperty("type")?geolocElement.type():"none";
        if(isTypeEnabled(type))
        {
			FGeoMap->addObject(MOT_POI, AId, geolocElement.coordinates()); // MercatorCoordinates(geolocElement.value("lat").toDouble(), geolocElement.value("lon").toDouble())
            emit mapDataChanged(MOT_POI, AId, MDR_ALL);
        }
        else
			FGeoMap->removeObject(MOT_POI, AId);
    }
}

void Poi::hideStreamPoi(const QString &ABareJid)
{
    PoiHash Dg = FGeolocHash.value(ABareJid);
    if(!Dg.isEmpty())
		if(FMap)
        {
            QString id(ABareJid);
            id.append(":%1");
            for(PoiHash::const_iterator it=Dg.constBegin(); it!=Dg.constEnd(); it++)
                hideOnePoi(id.arg(it.key()));
        }
}

void Poi::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
    if (AId==SCT_POI_LISTACCOUNT)
    {
        IRostersView *rostersView=FRostersViewPlugin->rostersView();
		if (AWidget==rostersView->instance())
		{
			QList<IRosterIndex*> indexes = rostersView->selectedRosterIndexes();
			QSet<QString> bareJids;
			for (QList<IRosterIndex*>::ConstIterator it=indexes.constBegin(); it!=indexes.constEnd(); it++)
				if ((*it)->data()==RIK_STREAM_ROOT)
					bareJids.insert(Jid(rostersView->instance()->currentIndex().data(RDR_FULL_JID).toString()).bare());
			showPoiList(bareJids);
		}
    }
    else
    {
        QTreeWidget *treeWidget=qobject_cast<QTreeWidget *>(AWidget);
        if (treeWidget)
        {
            QList<QTreeWidgetItem *> selected=treeWidget->selectedItems();
            if (!selected.isEmpty())
            {
                QTreeWidgetItem *item=selected[0];
                int action=AId==SCT_POI_SHOW?IPoi::PA_SHOW:
                           AId==SCT_POI_EDIT?IPoi::PA_EDIT:
                           AId==SCT_POI_SAVE?IPoi::PA_SAVE:
                           AId==SCT_POI_DELETE?IPoi::PA_DELETE:
                           AId==SCT_POI_REMOVE?IPoi::PA_REMOVE:
                           AId==SCT_POI_OPENURL?IPoi::PA_OPENURL:
                                                IPoi::PA_NULL;
                if (action)
                    processPoiAction(action, item->data(0, IPoi::PDR_ID).toString(), AWidget);
            }
        }
    }
}

void Poi::updateStreamPoi(const QString &ABareJid)
{
    PoiHash Dg = FGeolocHash.value(ABareJid);
    if(!Dg.isEmpty())
    {
        QString id(ABareJid);
        id.append(":%1");
        for(PoiHash::const_iterator it=Dg.constBegin(); it!=Dg.constEnd(); it++)
            updateOnePoi(id.arg(it.key()));
    }
}

void Poi::showAllPoi()
{
    for (QStringList::const_iterator it=FStreamPoi.constBegin(); it!=FStreamPoi.constEnd(); it++)
        showStreamPoi(*it);
}

void Poi::hideAllPoi()
{
    for (QStringList::const_iterator it=FStreamPoi.constBegin(); it!=FStreamPoi.constEnd(); it++)
        hideStreamPoi(*it);
}

/**
 * @brief Poi::updateAllPoi updates visibility state of all the POIs on the map (both stream and temporary POIs)
 */
void Poi::updateAllPoi()
{
	if(FMap)
    {
        for (QStringList::const_iterator it=FStreamPoi.constBegin(); it!=FStreamPoi.constEnd(); it++)
            updateStreamPoi(*it);

        for(PoiHash::const_iterator it=FTempPois.constBegin(); it!=FTempPois.constEnd(); it++)
            updateOnePoi(it.key());
    }
}

void Poi::addNewPoi()
{
    Action *action = qobject_cast<Action *>(sender());
    if(action)
    {
        QString title = tr("Add new point of interest");
		GeolocElement poi;
		poi.setCordinates(FMap->menuPopupPosition());
		NewPoi *newPoi  = new NewPoi(this, FMapLocationSelector, FPoiAccounts, title, poi, action->parentWidget()->parentWidget());
        newPoi->initStreamList();
        newPoi->allowEmptyName(false);
        if(newPoi->exec())
        {
			poi = newPoi->getPoi();
            Jid streamJid = newPoi->getStreamJid();
            QString bareJid=streamJid.bare();
            qulonglong cnt = FGlCount.value(bareJid)+1;
            QString id(QString::number(cnt, 16));
            FGeolocHash[bareJid].insert(id, poi);
            FGlCount[bareJid]=cnt;
            savePoiList(streamJid);
            showSinglePoi(bareJid.append(":").append(id));
            emit poiModified(bareJid, PMT_ADDED);
        }
        newPoi->deleteLater();
    }
}

// ------------------------------------------------------

void Poi::onPoiList(bool)
{
    Action *action = qobject_cast<Action *>(sender());
    if(action)
	{		
		QStringList   bareJids = action->data(ADR_STREAM_JID).toStringList();
		QSet<QString> streamBareJids;
		for (QStringList::ConstIterator it = bareJids.constBegin(); it!=bareJids.constEnd(); it++)
			streamBareJids.insert(*it);
		showPoiList(streamBareJids);
	}
}

void Poi::showPoiList(const QSet<QString> &AStreamBareJids)
{
	if (FPoiList)
		FPoiList->activateWindow();
    else
    {
		FPoiList = new PoiList(this);
		FPoiList->setWindowTitle(AStreamBareJids.size() == 1?tr("Point of interest list for %1").arg(*(AStreamBareJids.constBegin()))
																			 :tr("Point of interest list"));
		FPoiList->setWindowIcon(getIcon(MNI_POI_ADD));

		if (AStreamBareJids.isEmpty())
			FPoiList->fillTable(FGeolocHash);
		else
		{
			QHash<QString, PoiHash> geolocHash;
			for (QSet<QString>::ConstIterator it = AStreamBareJids.constBegin(); it!=AStreamBareJids.constEnd(); ++it)
				geolocHash.insert(*it, FGeolocHash[*it]);
			FPoiList->fillTable(geolocHash);
		}

		if (FPoiList->exec())
			poiShow(FPoiList->selectedId());
		FPoiList->deleteLater();
    }
}

// POI actions
void Poi::poiShow(const QString &APoiId) const
{
	if(FMap && !FMap->showObject(MOT_POI, APoiId))
    {   // Failed to show POI as a map object (filtered?)
        GeolocElement poi=getPoi(APoiId);
        if (poi.hasProperty(GeolocElement::Lat) && poi.hasProperty(GeolocElement::Lon))
        {
			FMap->showMap(true);
			FGeoMap->getScene()->setMapCenter(poi.coordinates());
        }
	}
}

bool Poi::poiOpenUri(QString AId)
{
    GeolocElement poi=getPoi(AId);
	if (poi.hasProperty(GeolocElement::Uri))
        return QDesktopServices::openUrl(poi.uri());
    return false;
}

bool Poi::poiDelete(QString AId)
{
    GeolocElement e=getPoi(AId);
    if (e.isEmpty())
        return false;

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Question,
                                              tr("Are you sure?"),
											  tr("Delete POI %1").arg(e.text()),
                                              QMessageBox::Ok|QMessageBox::Cancel);

    if(messageBox->exec() == QMessageBox::Ok)
        return removePoi(AId);
    else
        return false;
}

bool Poi::poiEdit(QString id, QWidget *AParent)
{    
    QStringList splitId=id.split(':');
    GeolocElement e = FGeolocHash.value(splitId[0]).value(splitId[1]);
    QString title = tr("Edit point of interest");

    NewPoi *newPoi  = new NewPoi(this, FMapLocationSelector, FPoiAccounts, title, e, AParent);
    newPoi->initStreamList(splitId[0], false);
    newPoi->allowEmptyName(false);
    if(newPoi->exec())
		if (putPoi(splitId[1], newPoi->getPoi(), newPoi->getStreamJid().bare()))
        {
            updateOnePoi(id);
            return true;
        }
    newPoi->deleteLater();
    return false;
}

bool Poi::poiSave(QString AId, const IAccount *AAccount, QWidget *AParent)
{
    NewPoi *newPoi  = new NewPoi(this, FMapLocationSelector, FPoiAccounts, tr("Save point of interest"), AAccount?FExtGeolocHash.value(AId):FTempPois.value(AId), AParent);
    newPoi->initStreamList(AAccount?AAccount->streamJid().bare():FTempPoiStreamJid);
    newPoi->allowEmptyName(false);
    if(newPoi->exec())
    {
        QString bareJid = newPoi->getStreamJid().bare();
        qulonglong cnt  = FGlCount.value(bareJid)+1;
        GeolocElement e=newPoi->getPoi();
		e.removeProperty("color");
        if (putPoi(QString::number(cnt, 16), e, bareJid))
        {
            FGlCount[bareJid]=cnt;
            return true;
        }
    }
    newPoi->deleteLater();
    return false;
}

Jid Poi::findStreamJid(const QString &bareJid) const
{
    for (QList<IAccount *>::const_iterator it=FPoiAccounts.constBegin(); it!=FPoiAccounts.constEnd(); it++)
        if ((*it)->streamJid().bare()==bareJid)
            return (*it)->streamJid();
    return Jid(QString());
}

QIcon Poi::getIcon(const QString &AType) const
{
    return FIconStorage->getIcon(AType);
}

QString Poi::getIconFileName(const QString &AType) const
{
    return FIconStorage->fileFullName(AType);
}

QIcon Poi::getTypeIcon(const QString &AType) const
{
    return AType.isEmpty()?getIcon(MNI_POI_NONE):FTypeStorage->getIcon(AType);
}

QString Poi::getTypeIconFileName(const QString &AType) const
{
    return AType.isEmpty()?getIconFileName(MNI_POI_NONE):FTypeStorage->fileFullName(AType);
}

QString Poi::getFullType(const QString &AType, const QString &AClass) const
{
    if (AClass.isEmpty())
    {
        if (FSubTypes.contains(AType))
            return AType;
        else
        {
            QString root=FSubTypes.key(AType);
            if (!root.isEmpty())
                return root+":"+AType;
        }
    }
    else
    {
        if (FSubTypes.contains(AClass))
            return FSubTypes.values(AClass).contains(AType)?(AClass+":"+AType):AClass;
        else
            return getFullType(AType);
    }
    return QString();
}

bool Poi::isTypeEnabled(QString AType) const
{
    if (FPoiFilter.contains(AType))
    {
        QStringList splitted=AType.split(':');
        return splitted.size()>1?FPoiFilter.contains(splitted[0]):true;
    }
    return false;
}

void Poi::updateChatWindowActions(IMessageChatWindow *AChatWindow)
{
    QList<QAction *> actions = AChatWindow->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_POI_VIEW);
    QAction *handle=actions.value(0, NULL);
    if(isSupported(AChatWindow->streamJid(), AChatWindow->contactJid()))
    {
        if (!handle)
        {
            MessagePoiList *poiList=getPoiList(AChatWindow);
            if (poiList->topLevelItemCount())
                poiList->show();
            Menu *menu = new Menu(poiList);
            menu->setTitle(tr("Points of Interest"));
			menu->setIcon(RSR_STORAGE_MENUICONS, MNI_POI_TLB);
            menu->menuAction()->setEnabled(true);

            Action *action = new Action(menu);
            action->setText(tr("Insert POI"));
			action->setIcon(RSR_STORAGE_MENUICONS,MNI_POI_TLB);
            action->setData(ADR_MESSAGE_TYPE, Message::Chat);
            action->setData(ADR_CONTACT_JID, AChatWindow->contactJid().full());
            action->setData(ADR_STREAM_JID, AChatWindow->streamJid().full());
            action->setShortcutId(SCT_MESSAGEWINDOWS_POI_INSERT);
            connect(action, SIGNAL(triggered(bool)), SLOT(onInsertPoi(bool)));
            menu->addAction(action, AG_PERSISTENT, false);

            action = new Action(menu);
            action->setText(tr("Insert location"));
			action->setIcon(RSR_STORAGE_MAPICONS,MPI_NEWCENTER);
            action->setData(ADR_MESSAGE_TYPE, Message::Chat);
            action->setData(ADR_CONTACT_JID, AChatWindow->contactJid().full());
            action->setData(ADR_STREAM_JID, AChatWindow->streamJid().full());
            action->setShortcutId(SCT_MESSAGEWINDOWS_POI_INSERTLOCATION);
            connect(action, SIGNAL(triggered(bool)), SLOT(onInsertLocation(bool)));
            menu->addAction(action, AG_PERSISTENT, false);

            AChatWindow->toolBarWidget()->toolBarChanger()
                       ->insertAction(menu->menuAction(), TBG_MWTBW_POI_VIEW)
                       ->setPopupMode(QToolButton::InstantPopup);
        }
    }
    else
    {
        if (handle)
        {
            AChatWindow->toolBarWidget()->toolBarChanger()->removeItem(handle);
            handle->deleteLater();
            getPoiList(AChatWindow)->hide();
        }
    }
}

QStringList Poi::getAllTypes() const
{
    QStringList allTypes;
    allTypes.append("none");
    for (QMap<QString, QString>::const_iterator it=FSubTypes.constBegin(); it!=FSubTypes.constEnd(); it++)
    {
        if (!allTypes.contains(it.key()))
            allTypes.append(it.key());
        allTypes.append(it.key()+':'+it.value());
    }
    return allTypes;
}

const QMap<QString, QString> &Poi::getTypeMap() const
{
    return FSubTypes;
}

const QHash<QString, QString> &Poi::getTranslatedTypes() const
{
    return FTranslatedTypes;
}

bool Poi::loadPoiTypes()
{
    FSubTypes.clear();
	FTranslatedTypes.clear();
    FTranslatedTypes.insert("none", tr("None"));

    QDir dir(FTypeStorage->resourcesDirs()[0]);
    if (dir.isReadable())
        if(dir.cd(FTypeStorage->storage()))
            if(dir.cd(FTypeStorage->subStorage()))
            {
                QStringList files = dir.entryList(QStringList() << FILE_STORAGE_DEFINITIONS_MASK);
                for(QStringList::const_iterator it=files.constBegin(); it!=files.constEnd(); ++it)
                {
                    QDomDocument doc;
                    QFile file(dir.filePath(*it));
                    if (file.fileName()!="types.def.xml")
                        if(file.open(QFile::ReadOnly) && doc.setContent(file.readAll(),false))
                        {
                            file.close();
                            QDomElement objElem = doc.documentElement().firstChildElement();
                            if(objElem.tagName() == "icon")
                            {
                                for(QDomElement e = objElem; !e.isNull(); e = e.nextSiblingElement())
                                {
                                    QString key = e.firstChildElement("key").text();
                                    QString name = e.firstChildElement("name").text();

                                    QStringList splitted=key.split(':');

                                    if (splitted.size()>1)
                                        FSubTypes.insertMulti(splitted[0], splitted[1]);
                                    //TODO: Move this translation into a separate CPP (as it done for "countries")
									FTranslatedTypes.insert(key, tr(name.toLatin1().data()));
                                }
                            }
                        }
                }
                return true;
            }
    return false;
}

//---------------------------------------------
void Poi::onMapObjectInserted(int AType, const QString &AId)
{
    if (AType==MOT_POI)
    {
        emit mapDataChanged(AType, AId, MDR_POI_ICON);
        emit mapDataChanged(AType, AId, MDR_POI_LABEL);
    }
}

void Poi::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

QGraphicsItem * Poi::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{    
    QStringList splitId = ASceneObject->mapObject()->id().split(":");
    GeolocElement e = splitId.size()>1?FGeolocHash[splitId[0]].value(splitId[1])
                                      :FTempPois[splitId[0]];

    switch(ARole)
    {
        case MDR_POI_ICON:
        {
			QIcon icon = getTypeIcon(e.type());
            if (icon.isNull())
                icon=getIcon(MNI_POI_NOTYPE);
            if (ACurrentElement)
                qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement)->setPixmap(icon.pixmap(icon.availableSizes().first()));
            else
                ACurrentElement = new QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()));
            break;
        }

        case MDR_POI_LABEL:
        {
            QGraphicsSimpleTextItem *textItem;
            if (ACurrentElement)
            {
                textItem=qgraphicsitem_cast<QGraphicsSimpleTextItem *>(ACurrentElement);
				textItem->setText(e.text());
            }
            else
            {
				textItem = new QGraphicsSimpleTextItem(e.text());
                QGraphicsDropShadowEffect * dropShadowEffect=new QGraphicsDropShadowEffect();
                // Set black shadow for bright colors and white shadow for dark
                dropShadowEffect->setColor(Qt::black);
                dropShadowEffect->setBlurRadius(3);
                dropShadowEffect->setOffset(0.7);
                textItem->setGraphicsEffect(dropShadowEffect);
            }
            QFont font=Options::node(OPV_POI_PNT_FONT).value().value<QFont>();
            font.setUnderline(ASceneObject->isActive());
            textItem->setFont(font);
            QColor color= e.hasProperty("color")?e.property("color").value<QColor>():Options::node(OPV_POI_PNT_TEXTCOLOR).value().value<QColor>();
            textItem->setBrush(QBrush(color));
            return textItem;
        }
    }
    return ACurrentElement;
}

void Poi::mouseHoverEnter(SceneObject *ASceneObject)
{
    objectUpdated(ASceneObject);
}

void Poi::mouseHoverLeave(SceneObject *ASceneObject)
{
    objectUpdated(ASceneObject);
}

bool Poi::mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton)
{
	Q_UNUSED(AButton)

    poiShow(ASceneObject->mapObject()->id());
    return true;
}

bool Poi::contextMenu(const QString &AId, Menu *AMenu)
{
    QStringList splitId = AId.split(":");
    bool temporary=splitId.size()==1;
    GeolocElement e = temporary?FTempPois.value(splitId[0])
                               :FGeolocHash.value(splitId[0]).value(splitId[1]);
    Action *action=new Action(AMenu);
    action->setText(tr("Show"));
    action->setIcon(RSR_STORAGE_MENUICONS, MNI_POI);
    action->setData(ADR_ID, AId);
    action->setData(ADR_ACTION, PA_SHOW);
    connect(action, SIGNAL(triggered()), SLOT(onPoiActionTriggered()));
    AMenu->addAction(action);

    action=new Action(AMenu);
    if (temporary)
    {
        action->setText(tr("Save"));
        action->setIcon(RSR_STORAGE_MENUICONS, MNI_EDIT_ADD);
        action->setData(ADR_ACTION, PA_SAVE);
    }
    else
    {
        action->setText(tr("Edit"));
		action->setIcon(RSR_STORAGE_MENUICONS, MNI_EDIT);
        action->setData(ADR_ACTION, PA_EDIT);
    }
    action->setData(ADR_ID, AId);
    connect(action, SIGNAL(triggered()), SLOT(onPoiActionTriggered()));
    AMenu->addAction(action);

	if (e.hasProperty(GeolocElement::Uri))
    {
        action=new Action(AMenu);
        action->setText(tr("Open URL"));
        action->setIcon(RSR_STORAGE_MENUICONS, MNI_LINK);
        action->setData(ADR_ID, AId);
        action->setData(ADR_ACTION, PA_OPENURL);
        connect(action, SIGNAL(triggered()), SLOT(onPoiActionTriggered()));
        AMenu->addAction(action);
    }

    action=new Action(AMenu);
    if (temporary)
    {
        action->setText(tr("Remove"));
        action->setData(ADR_ACTION, PA_REMOVE);
    }
    else
    {
        action->setText(tr("Delete"));
        action->setData(ADR_ACTION, PA_DELETE);
    }
    action->setIcon(RSR_STORAGE_MENUICONS, MNI_EDIT_DELETE);
    action->setData(ADR_ID, AId);
    connect(action, SIGNAL(triggered()), SLOT(onPoiActionTriggered()));
    AMenu->addAction(action);

    for (QList<Action *>::const_iterator it=FActions.constBegin(); it!=FActions.constEnd(); it++)
    {
        (*it)->setData(ADR_ID, AId);
        AMenu->addAction(*it);
    }

    return true;
}

bool Poi::processPoiAction(int AAction, QString AId, QWidget *AParentWidget)
{
    switch (AAction)
    {
        case PA_SHOW:
            poiShow(AId);
            break;

        case PA_EDIT:
            poiEdit(AId, AParentWidget);
            break;

        case PA_SAVE:
            if (poiSave(AId, NULL, AParentWidget))
                removePoi(AId);
            break;

        case PA_OPENURL:
            poiOpenUri(AId);
            break;

        case PA_DELETE:
            poiDelete(AId);
            break;

        case PA_REMOVE:
            removePoi(AId);
            break;

        default:
            return false;
    }
    return true;
}

bool Poi::insertPoiShortcut(const QString &AShortcutId)
{
    if (FPoiShortcuts.contains(AShortcutId))
        return false;
    else
    {
        FPoiShortcuts.append(AShortcutId);
        return true;
    }
}

void Poi::setTreeWidgetShortcuts(QTreeWidget *ATreeWidget, bool ATemporary)
{
    Shortcuts::insertWidgetShortcut(SCT_POI_SHOW, ATreeWidget);
    Shortcuts::insertWidgetShortcut(SCT_POI_OPENURL, ATreeWidget);
    if (ATemporary)
    {
        Shortcuts::insertWidgetShortcut(SCT_POI_SAVE, ATreeWidget);
        Shortcuts::insertWidgetShortcut(SCT_POI_REMOVE, ATreeWidget);
    }
    else
    {
        Shortcuts::insertWidgetShortcut(SCT_POI_EDIT, ATreeWidget);
        Shortcuts::insertWidgetShortcut(SCT_POI_DELETE, ATreeWidget);
    }

    for (QStringList::const_iterator it=FPoiShortcuts.constBegin(); it!=FPoiShortcuts.constEnd(); it++)
        Shortcuts::insertWidgetShortcut(*it, ATreeWidget);
}

QString Poi::getCoordString(const GeolocElement &APoiData) const
{
	QString lat(QString::number(APoiData.lat(), 'f', 6));
	QString lon(QString::number(APoiData.lon(), 'f', 6));
    return tr("%1, %2").arg((lat[0]=='-')?tr("%1S").arg(lat.mid(1, 8)):tr("%1N").arg(lat.mid(0, 8)))
                       .arg((lon[0]=='-')?tr("%1W").arg(lon.mid(1, 8)):tr("%1E").arg(lon.mid(0, 8)));
}

void Poi::onPoiActionTriggered()
{
    Action *action=qobject_cast<Action *>(sender());
    if (action)
        processPoiAction(action->data(ADR_ACTION).toInt(), action->data(ADR_ID).toString(), action->parentWidget()?action->parentWidget()->parentWidget():NULL);
}

bool Poi::contextMenu(SceneObject *ASceneObject, QMenu *AMenu)
{
	Menu *menu = qobject_cast<Menu *>(AMenu);
	return menu?contextMenu(ASceneObject->mapObject()->id(), menu):false;
}

void Poi::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    switch (ARole)
    {
        case MDR_ALL:
        case MDR_POI_LABEL:
        case MDR_POI_ICON:
            emit mapDataChanged(MOT_POI, ASceneObject->mapObject()->id(), ARole);
            break;

//FIXME: There is something wrong here!!!
        case MDR_NONE:
            QGraphicsSimpleTextItem *item=qgraphicsitem_cast<QGraphicsSimpleTextItem *>(ASceneObject->getElementByRole(MDR_POI_LABEL));
            if (item)
            {
                QFont font=item->font();
                font.setUnderline(ASceneObject->isActive());
                item->setFont(font);
                item->update();
            }
            else
                LOG_ERROR("Poi::objectUpdated(MDR_NONE): item=NULL!");
            break;
    }
}

QString Poi::toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const
{
	Q_UNUSED(AEvent)

    QStringList splitId = AMapObject->id().split(":");
    GeolocElement e = splitId.size()>1?FGeolocHash.value(splitId[0]).value(splitId[1])
                                      :FTempPois.value(splitId[0]);
    QString htmlPoi;
    if (e.hasProperty(GeolocElement::Description))
		htmlPoi.append("<strong>").append(tr("Description")).append(":</strong> ").append(e.description());
	if (e.hasProperty(GeolocElement::Lat) || e.hasProperty(GeolocElement::Lon))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
        htmlPoi.append("<strong>").append(tr("Location")).append(":</strong> ").append(getCoordString(e));
    }
	if (e.hasProperty(GeolocElement::Type))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Type")).append(":</strong> ").append(FTranslatedTypes.value(e.type()));
    }
	if (e.hasProperty(GeolocElement::Country) || e.hasProperty(GeolocElement::CountryCode))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
        htmlPoi.append("<strong>").append(tr("Country")).append(":</strong> ");
		if (e.hasProperty(GeolocElement::CountryCode))
        {
			htmlPoi.append(e.countryCode());
			if (e.hasProperty(GeolocElement::Country))
				htmlPoi.append(" (").append(e.country()).append(")");
        }
        else
			htmlPoi.append(e.country());
    }
	if (e.hasProperty(GeolocElement::Region))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Region")).append(":</strong> ").append(e.region());
    }
	if (e.hasProperty(GeolocElement::Locality))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Locality")).append(":</strong> ").append(e.locality());
    }
	if (e.hasProperty(GeolocElement::Area))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Area")).append(":</strong> ").append(e.area());
    }
	if (e.hasProperty(GeolocElement::Street))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Street")).append(":</strong> ").append(e.street());
    }
	if (e.hasProperty(GeolocElement::Building))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Building")).append(":</strong> ").append(e.building());
    }
	if (e.hasProperty(GeolocElement::Floor))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Floor")).append(":</strong> ").append(e.floor());
    }
	if (e.hasProperty(GeolocElement::Room))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Room")).append(":</strong> ").append(e.room());
    }
	if (e.hasProperty(GeolocElement::PostalCode))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Postal code")).append(":</strong> ").append(e.postalCode());
    }
	if (e.hasProperty(GeolocElement::TimeStamp))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("Timestamp")).append(":</strong> ").append(e.timeStamp().toString(Qt::DefaultLocaleShortDate));
    }
	if (e.hasProperty(GeolocElement::Uri))
    {
        if (!htmlPoi.isEmpty())
            htmlPoi.append("<br>");
		htmlPoi.append("<strong>").append(tr("URI")).append(":</strong> <u><font color=blue>").append(e.uri().toString()).append("</font></u>");
    }
    return htmlPoi;
}

bool Poi::messageViewUrlOpen(int AOrder, IMessageViewWidget *AWidget, const QUrl &AUrl)
{
    Q_UNUSED(AWidget)
    Q_UNUSED(AOrder)
	if (FMap)
    {
        if (AUrl.scheme()=="geoloc")
        {
            FTempPoiStreamJid=AWidget->messageWindow()->streamJid().bare();
            GeolocElement poi=FExtGeolocHash.value(AUrl.path());
            poi.setProperty("color", Options::node(OPV_POI_PNT_TEMPTEXTCOLOR).value().value<QColor>());
			putPoi("temporary", poi, true);
            poiShow("temporary");
        }
        else if (AUrl.scheme()=="poi")
        {
			poiSave(AUrl.path(), FAccountManager->findAccountByStream(AWidget->messageWindow()->streamJid()), AWidget->instance());
            return true;
        }
    }
    return false;
}

bool Poi::bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid)
{
    Q_UNUSED(AOrder)
	Q_UNUSED(AStreamJid)
	Q_UNUSED(AContactJid)

	if (FMap)
    {
        if (AUrl.scheme()=="geoloc")
        {
            FTempPoiStreamJid=AStreamJid.full();
            GeolocElement poi=FExtGeolocHash.value(AUrl.path());
            poi.setProperty("color", Options::node(OPV_POI_PNT_TEMPTEXTCOLOR).value().value<QColor>());
            putPoi("temporary", poi, true);
            poiShow("temporary");
        }
        else if (AUrl.scheme()=="poi")
        {
			poiSave(AUrl.path(), FAccountManager->findAccountByStream(AStreamJid), NULL);
            return true;
        }
    }
    return false;
}

bool Poi::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

	return !parsePOI(AMessage).isEmpty();
}

bool Poi::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder)
	Q_UNUSED(ALang)

    QString poiElement=parsePOI(AMessage);
    if (!poiElement.isEmpty())
    {
        QTextCursor cursor(ADocument);
        cursor.movePosition(QTextCursor::End);
        QTextBlockFormat format;
        format.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth, QTextLength(QTextLength::PercentageLength, 100));
        cursor.insertBlock(format);
		cursor.insertBlock(QTextBlockFormat());
        cursor.insertHtml(poiElement);
		return true;
    }
	return false;
}

bool Poi::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder) Q_UNUSED(AMessage) Q_UNUSED(ADocument) Q_UNUSED(ALang)
	return false;
}  // Nothing to do right now

bool Poi::messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection)
{
	Q_UNUSED(AOrder)

    if (ADirection==IMessageProcessor::DirectionOut)
    {
        Jid contactJid = AMessage.to();
        if(AMessage.type()==Message::Chat)
        {
            MessagePoiList *poiList=findPoiList(AStreamJid, contactJid, Message::Chat);
            if (poiList && poiList->topLevelItemCount())
            {
                GeolocElement e = poiList->topLevelItem(0)->data(1, MessagePoiList::IDR_POI).value<GeolocElement>();
                if(!e.isEmpty())
                {
                    addPoiToMessage(AMessage, e);
                    poiList->clear();
                    poiList->hide();
                }
            }
        }
        else if(AMessage.type()==Message::Normal)
        {
            MessagePoiList *poiList=findPoiList(AStreamJid, contactJid, Message::Normal);
            if (poiList && poiList->topLevelItemCount())
            {
                GeolocElement e = poiList->topLevelItem(0)->data(1, MessagePoiList::IDR_POI).value<GeolocElement>();
                if(!e.isEmpty())
                {
                    addPoiToMessage(AMessage, e);
                    poiList->clear();
                    poiList->hide();
                }
            }
        }
    }
	return false;
}

bool Poi::loadPoiList(const Jid &AStreamJid)
{    
    if (FPrivateStorage)
    {
        QString id = FPrivateStorage->loadData(AStreamJid,PST_POI,PSN_POI);
        if (!id.isEmpty())
        {
            FLoadRequests.insert(id,AStreamJid);
            return true;
        }
    }
    return false;
}

bool Poi::savePoiList(const Jid &AStreamJid)
{
    if (isEnabled(AStreamJid.bare()))
    {
        QDomDocument doc;
        QDomElement poi = doc.appendChild(doc.createElementNS(PSN_POI,PST_POI)).toElement();

        PoiHash items = FGeolocHash.value(AStreamJid.bare());
        for (PoiHash::const_iterator it = items.constBegin(); it != items.constEnd(); ++it)
            (*it).exportElement(poi);

        QString id = FPrivateStorage->saveData(AStreamJid,doc.documentElement());
        if (!id.isEmpty())
        {
            FSaveRequests.insert(id,AStreamJid);
            return true;
        }
    }
    return false;
}

// Private storage!!!
void Poi::onPrivateStorageOpened(const Jid &AStreamJid)
{
	IAccount *account=FAccountManager->findAccountByStream(AStreamJid);
    if (account->optionsNode().value("enable-poi").toBool())
    {
        FPoiAccounts.append(account);
        QString bareJid=AStreamJid.bare();
        if (!FStreamPoi.contains(bareJid))
        {
            FStreamPoi.append(bareJid);
            loadPoiList(AStreamJid);
            if (FStreamPoi.count()==1)
                addMenuMap();
        }
    }
}

void Poi::onPrivateDataSaved(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement)
{
    Q_UNUSED(AElement);
    if (FSaveRequests.contains(AId))
    {
        FSaveRequests.remove(AId);
        emit poisSaved(AStreamJid.bare(), FGeolocHash[AStreamJid.bare()]);
    }
}

void Poi::onPrivateDataLoaded(const QString &AId, const Jid &AStreamJid, const QDomElement &AElement)
{
    if (FLoadRequests.contains(AId))
    {
        FLoadRequests.remove(AId);

        PoiHash poiHash;
        qulonglong cnt = 0;
        QString bareJid=AStreamJid.bare();

        for(QDomElement e=AElement.firstChildElement(); !e.isNull(); e=e.nextSiblingElement())
            if(e.tagName() == "geoloc")
                poiHash.insert(QString::number(cnt++, 16), GeolocElement(e));

        if(!poiHash.isEmpty())
        {
            if (FGeolocHash.contains(bareJid))
                emit poisRemoved(bareJid);      // To notify everyone, to remove stored POIs for the stream.
            FGlCount.insert(bareJid, cnt);
            FGeolocHash.insert(bareJid, poiHash);
            emit poisLoaded(bareJid, FGeolocHash[bareJid]);
        }
    }
}

void Poi::onPrivateDataChanged(const Jid &AStreamJid, const QString &ATagName, const QString &ANamespace)
{
    if (isEnabled(AStreamJid.bare()) && ATagName==PST_POI && ANamespace==PSN_POI)
        loadPoiList(AStreamJid);
}

void Poi::onPrivateStorageClosed(const Jid &AStreamJid)
{
	IAccount *account=FAccountManager->findAccountByStream(AStreamJid);
    if (account->optionsNode().value("enable-poi").toBool())
    {
        FPoiAccounts.removeAll(account);
        QString bareJid=AStreamJid.bare();
        for (QList<IAccount *>::const_iterator it=FPoiAccounts.constBegin(); it!=FPoiAccounts.constEnd(); it++)
            if ((*it)->streamJid().bare() == bareJid) return; // More accounts with the same bare JID exist
        FStreamPoi.removeAll(bareJid);
        if (FStreamPoi.count()==0)
            deleteMenuMap();
        emit poisRemoved(bareJid);
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_poi, Poi)
#endif

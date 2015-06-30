#include "tracker.h"

#include <QPicture>
#include <QDebug>


#define  SEND    "TRACKER"

Tracker::Tracker():
    FOptionsManager(NULL),
    //FPositioning(NULL),
    FGeoloc(NULL),
    FIconStorage(NULL),
	FRosterManager(NULL)
{}

Tracker::~Tracker()
{}

//-----------------------------
void Tracker::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Tracker");
	APluginInfo->description = tr("Tracker");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	//APluginInfo->dependences.append(YYY_UUID); //----???
}

bool Tracker::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
    if (plugin)
    {
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
		if (FRosterManager)
        {
			connect(FRosterManager->instance(),SIGNAL(rosterOpened(IRoster *)),this,SLOT(rosterOpened(IRoster *)));
			connect(FRosterManager->instance(),SIGNAL(rosterClosed(IRoster *)),this,SLOT(rosterClosed(IRoster*)));
        }
    }

    plugin = APluginManager->pluginInterface("IGeoloc").value(0,NULL);
    if (plugin)
    {
        FGeoloc = qobject_cast<IGeoloc *>(plugin->instance());
        connect(FGeoloc->instance(),SIGNAL(locationReceived(Jid,Jid,MercatorCoordinates, bool)),SLOT(onLocationReceived(Jid,Jid,MercatorCoordinates,bool)));
    }

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    //AInitOrder = 200;   // This one should be initialized AFTER ...
    return true; //FMessageWidgets!=NULL

}

bool Tracker::initObjects()
{
    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_TRACKER);//<---in menuicon--

    return true;
}

bool Tracker::initSettings()
{
    Options::setDefaultValue(OPV_TRACKER,false);

    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_TRACKER, OPN_TRACKER, MNI_TRACKER, tr("Tracker")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> Tracker::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_TRACKER)
		widgets.insertMulti(OWO_TRACKER, new TrackOptions(this,AParent));
    return widgets;
}

void Tracker::onLocationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged)
{
	Q_UNUSED(AStreamJid)
	Q_UNUSED(AContactJid)
	Q_UNUSED(ACoordinates)
	Q_UNUSED(AReliabilityChanged)
// qDebug() << "Tracker::onLocationReceived(" << jid.full() << "," << coords.toString() << ")" ;//
}

void Tracker::rosterOpened(IRoster *ARoster)
{
    Jid streamJid = ARoster->streamJid();
	QList<IRosterItem> rosterItems = ARoster->items();
    QHash<Jid, QList<IRosterItem> > jidItems;
    jidItems.insert(streamJid,rosterItems);
}

void Tracker::rosterClosed(IRoster *ARoster)
{
    Q_UNUSED(ARoster);
}


//--------
QIcon Tracker::getIcon(const QString locName) const
{
    return FIconStorage->getIcon(locName);
}

QString Tracker::getIconFileName(const QString locName) const
{
    return FIconStorage->fileFullName(locName);
}

void Tracker::onMapObjectInserted(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
// Not implemented yet
//    emit mapDataChanged(AType, AId, MDT_TRACK_ICON);
}

void Tracker::onMapObjectRemoved(int AType, const QString &AId)
{
    Q_UNUSED(AType)
    Q_UNUSED(AId)
}

QGraphicsItem * Tracker::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)
    Q_UNUSED(ACurrentElement)
/*
    QIcon icon;
    QStringList splitId = id.split(":");

    //QHash<int,QDomElement> Dg = FGeolocHash.value(splitId[0]);

    switch(dtype)
    {
        case MDT_TRAC_ICON:
qDebug() << "switch(dtype)/splitId[2]= " << splitId[2];
//            icon =FNewPoi->getTypeIcon(splitId[2]);
//            if (!icon.isNull())
//                return icon.pixmap(icon.availableSizes().first());
            break;

        case MDT_TRAC_TEXT:
//            int key = QString(splitId[1]).toInt();
//            QDomElement e = Dg.value(key);
//            return  e.firstChildElement("text").text();
            break;
    }

    return QVariant();
*/
    return NULL;
}

/*
void Tracker::paint(const IMapObject *mapObject, const int position, const IMapObjectDataSource *source, QPainter *painter) const
{
    switch (position)
    {
        //case MOP_RIGHT_TOP:
        case MOP_RIGHT:
        {
            float x=0;
            QPixmap pixmap=source->getData(MDT_TRAC_ICON, mapObject->getId()).value<QPixmap>();
//            qDebug("pixmap.isNull=%d", pixmap.isNull());
            if (!pixmap.isNull())
            {
                painter->drawPixmap(x,0,pixmap);
                x+=pixmap.width();
            }

            QFont font("Times",10,QFont::Normal);
            font.setUnderline(mapObject->isActive());
            painter->setFont(font);
            QString label=source->getData(MDT_TRAC_TEXT, mapObject->getId()).value<QString>();

            // Draw shadow
            painter->setPen(Qt::darkBlue);//darkGray
            painter->drawText(QRectF(QPointF(x+1.0,1.0), paintingSize(mapObject, position, source)), label);

            // Draw label
            painter->setPen(Qt::cyan);//blue
            painter->drawText(QRectF(QPointF(x,0.0), paintingSize(mapObject, position, source)), label);
            break;
        }
    }
}

QSizeF Tracker::paintingSize(const IMapObject *mapObject, const int position, const IMapObjectDataSource *source) const
{
    switch (position)
    {
        case MOP_RIGHT:
        {
            float w=0, h=0;
            QPixmap pixmap=source->getData(MDT_TRAC_ICON, mapObject->getId()).value<QPixmap>();
            if (!pixmap.isNull())
            {
                w=pixmap.width();
                h=pixmap.height();
            }
            QPainter painter(new QPicture());
            painter.setFont(QFont("Times", 10, QFont::Normal));
            QString label=source->getData(MDT_TRAC_TEXT, mapObject->getId()).value<QString>();
            if (!label.isNull())
            {
                QSizeF size=painter.boundingRect(QRectF(), Qt::AlignLeft|Qt::AlignTop, label).size();
                // Increase both width and height by 1 for shadow drawing
                w+=size.rwidth()+1;
                if (h<size.rheight())
                    h=size.rheight();
            }
            return QSizeF(w,h);
        }
    }
    return QSizeF();    // Error!!!
}

bool Tracker::display(const IMapObject *mapObject) const
{
    return mapObject->isActive();
}
*/

void Tracker::onOptionsOpened()
{
    OptionsNode node = Options::node(OPV_TRACKER);
    //xx = node.value().toBool();
}

void Tracker::onOptionsClosed()
{
}

void Tracker::onOptionsChanged(const OptionsNode &ANode)
{
	Q_UNUSED(ANode)
}

void Tracker::objectUpdated(SceneObject *ASceneObject, int ARole)
{
    Q_UNUSED(ASceneObject)
    Q_UNUSED(ARole)
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tracker, Tracker)
#endif

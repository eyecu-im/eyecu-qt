#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <utils/logger.h>
#include <definitions/mapobjecttyperole.h>
#include <definitions/mapobjectdatarole.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/actiongroups.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterlabels.h>
#include <definitions/rosterclickhookerorders.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <MapObject>

#include "mapcontacts.h"
#include "mapcontactsoptions.h"

#define ADR_CONTACT_JID         Action::DR_Parametr1

MapContacts::MapContacts(QObject *parent) :
	QObject(parent),
	FMap(NULL),
	FGeoloc(NULL),
	FRosterManager(NULL),
	FRostersView(NULL),
	FRostersModel(NULL),
	FStatusIcons(NULL),
	FAvatars(NULL),
	FOptionsManager(NULL),
	FMessageProcessor(NULL),
	FAvatarSize(32,32)
{}

void MapContacts::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Map Contacts");
	APluginInfo->description = tr("Displays roster contacts on the map");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MAP_UUID);
	APluginInfo->dependences.append(ROSTER_UUID);
	APluginInfo->dependences.append(GEOLOC_UUID);
}

bool MapContacts::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	IPlugin *plugin = APluginManager->pluginInterface("IMap").value(0,NULL);
	if (plugin)
		FMap = qobject_cast<IMap *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IGeoloc").value(0,NULL);
	if (plugin)
	{
		FGeoloc = qobject_cast<IGeoloc *>(plugin->instance());
		connect(FGeoloc->instance(),SIGNAL(locationReceived(Jid,Jid,MercatorCoordinates,bool)),SLOT(onLocationReceived(Jid,Jid,MercatorCoordinates,bool)));
		connect(FGeoloc->instance(),SIGNAL(locationRemoved(Jid,Jid)),SLOT(onLocationRemoved(Jid,Jid)));
	}
	else
		return false;

	plugin = APluginManager->pluginInterface("IStatusIcons").value(0,NULL);
	if (plugin)
		FStatusIcons = qobject_cast<IStatusIcons *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
		if (FRosterManager)
		{
			connect(FRosterManager->instance(),SIGNAL(rosterOpened(IRoster *)),this,SLOT(onRosterOpened(IRoster *)));
			connect(FRosterManager->instance(),SIGNAL(rosterClosed(IRoster *)),this,SLOT(onRosterClosed(IRoster*)));
		}
	}
	else
		return false;

	plugin = APluginManager->pluginInterface("IRostersModel").value(0,NULL);
	if (plugin)
	{
		FRostersModel = qobject_cast<IRostersModel *>(plugin->instance());
		if (FRostersModel)
		{
			connect(FRostersModel->instance(), SIGNAL(indexDataChanged(IRosterIndex *, int)),
											   SLOT(onIndexDataChanged(IRosterIndex*, int)));
			connect(FRostersModel->instance(), SIGNAL(indexInserted(IRosterIndex *)),
											   SLOT(onIndexInserted(IRosterIndex*)));
			connect(FRostersModel->instance(), SIGNAL(indexRemoving(IRosterIndex *)),
											   SLOT(onIndexRemoving(IRosterIndex*)));
		}
	}
	else
		return false;

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IAvatars").value(0,NULL);
	if (plugin)
		FAvatars = qobject_cast<IAvatars *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
	if (plugin)
	{
		IRostersViewPlugin *rostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
		if (rostersViewPlugin)
		{
			FRostersView=rostersViewPlugin->rostersView();
			connect(FRostersView->instance(),
					SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
					SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));
			connect(Shortcuts::instance(), SIGNAL(shortcutActivated(const QString &, QWidget *)), SLOT(onShortcutActivated(const QString &, QWidget *)));
		}
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	AInitOrder = 150;
	return true;
}

bool MapContacts::initObjects()
{
	Shortcuts::declareShortcut(SCT_ROSTERVIEW_SHOWCONTACTONTHEMAP, tr("Show contact on the map"), tr("F10", "Show contact on the map (roster)"), Shortcuts::WidgetShortcut);

	FMap->geoMap()->setObjectHandler(MOT_CONTACT, this);
	FMap->geoMap()->addDataHolder(MOT_CONTACT, this);
	if (FStatusIcons)
		FMap->geoMap()->registerDataType(MDR_CONTACT_STATUS_ICON, MOT_CONTACT, 100, MOP_RIGHT_TOP, MOP_CENTER);

	if (FRosterManager)
		FMap->geoMap()->registerDataType(MDR_CONTACT_LABEL, MOT_CONTACT, 100, MOP_RIGHT_BOTTOM, MOP_RIGHT);

	if (FAvatars)
	{
		FMap->geoMap()->registerDataType(MDR_CONTACT_AVATAR, MOT_CONTACT, 100, MOP_LEFT);
		FEmptyAvatar = QImage(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->fileFullName(MNI_AVATAR_EMPTY)).scaled(FAvatarSize,Qt::KeepAspectRatio,Qt::FastTransformation);
	}

	if (FRostersView)
	{
		FRostersView->insertClickHooker(RCHO_MAPCONTACTS, this);
		Shortcuts::insertWidgetShortcut(SCT_ROSTERVIEW_SHOWCONTACTONTHEMAP, FRostersView->instance());
	}

	return true;
}

bool MapContacts::initSettings()
{
	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

	Options::setDefaultValue(OPV_MAP_CONTACTS_VIEW, MCO_VIEW_FULLACTIVE);
	Options::setDefaultValue(OPV_MAP_CONTACTS_FOLLOW, true);

	return true;
}

void MapContacts::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_ROSTER_AVATARS_DISPLAYEMPTY));
}

void MapContacts::onOptionsClosed()
{}

void MapContacts::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MAP_CONTACTS_VIEW)
		FMap->geoMap()->updateObjects(MOT_CONTACT, MDR_NONE);
	else if (ANode.path() == OPV_ROSTER_AVATARS_DISPLAYEMPTY)
		FShowEmptyAvatars = ANode.value().toBool();
	else if (ANode.path() == OPV_ROSTER_SHOWOFFLINE)
	{
		bool show=ANode.value().toBool();
		for (QStringList::ConstIterator it=FContacts.constBegin(); it!=FContacts.constEnd(); it++)
			if (!isOnline(*it))
				FMap->geoMap()->setObjectVisible(MOT_CONTACT, *it, show);
	}
}

QMultiMap<int, IOptionsDialogWidget *> MapContacts::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MAP)
	{
		widgets.insertMulti(OHO_MAP_CONTACTS, FOptionsManager->newOptionsDialogHeader("Contacts", AParent));
		widgets.insertMulti(OWO_MAP_CONTACTS, new MapContactsOptions(AParent));
	}
	return widgets;
}

//------------------------------
QStringList MapContacts::getFullJidList(const QString &bareJid) const
{
	return FResourceHash.values(bareJid);
}

bool MapContacts::hasGeoloc(QString &AJid) const
{
	Jid jid(AJid);
	if (!jid.isValid())
		return false;

	if (!FMap->geoMap()->isObjectExists(MOT_CONTACT, jid.full()))
	{
		if (FMap->geoMap()->isObjectExists(MOT_CONTACT, jid.bare()))
			AJid=jid.bare();
		else
			return false;
	}
	return true;
}

void MapContacts::showContact(QString AJid, bool AShowMap) const
{
	if (FResourceHash.contains(AJid))
		AJid=FResourceHash.value(AJid);
	if (hasGeoloc(AJid))
		FMap->showObject(MOT_CONTACT, AJid, Options::node(OPV_MAP_CONTACTS_FOLLOW).value().toBool(), AShowMap);
}
//------------------------------

void MapContacts::mouseHoverEnter(SceneObject *ASceneObject)
{
	objectUpdated(ASceneObject);
}

void MapContacts::mouseHoverLeave(SceneObject *ASceneObject)
{
	objectUpdated(ASceneObject);
}

bool MapContacts::mouseClicked(SceneObject *ASceneObject, Qt::MouseButton AButton)
{
	Q_UNUSED(ASceneObject)
	Q_UNUSED(AButton)
	return false;
}

bool MapContacts::mouseDoubleClicked(SceneObject *ASceneObject, Qt::MouseButton AButton)
{
	Q_UNUSED(AButton)

	if (FMessageProcessor)
	{
		IRosterIndex *index=getRosterIndex(ASceneObject->mapObject()->id());
		if (index)
			return FMessageProcessor->createMessageWindow(index->data(RDR_STREAM_JID).toString(), ASceneObject->mapObject()->id(), Message::Chat, IMessageHandler::SM_SHOW);
	}
	return false;
}

bool MapContacts::contextMenu(SceneObject *ASceneObject, QMenu *AMenu)
{
	IRosterIndex *index=getRosterIndex(ASceneObject->mapObject()->id());
	if (index)
	{
		Menu *menu = qobject_cast<Menu *>(AMenu);
		if (menu)
		{
			QList<IRosterIndex *> single;
			single.append(index);
			FRostersView->contextMenuForIndex(single, NULL, menu);


			QList<Action *> actions=menu->actions(AG_RVCM_RCHANGER);
			for (QList<Action *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
			{
				IconStorage *iconStorage=IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
				QIcon icon=(*it)->icon();
				if (icon.cacheKey()==iconStorage->getIcon(MNI_RCHANGER_COPY_GROUP).cacheKey() ||
					icon.cacheKey()==iconStorage->getIcon(MNI_RCHANGER_MOVE_GROUP).cacheKey() ||
					icon.cacheKey()==iconStorage->getIcon(MNI_RCHANGER_REMOVE_FROM_GROUP).cacheKey())
					menu->removeAction(*it);
			}

			actions=menu->actions(AG_RVCM_GEOLOC);
			if (!actions.isEmpty())
			{
				Action *action=actions[0];
				if (action->menu())
				{
					QList<Action *> actions=action->menu()->actions(AG_RVCM_GEOLOC);
					for (QList<Action *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
						if ((*it)->data(ADR_CONTACT_JID).toString()==ASceneObject->mapObject()->id())
						{
							(*it)->setText(tr("Show on the map"));
							menu->addAction(*it, AG_RVCM_GEOLOC);
							break;
						}
					menu->removeAction(action);
				}
				else
				{
					QString jid=action->data(ADR_CONTACT_JID).toString();
					if (jid!=ASceneObject->mapObject()->id())
						menu->removeAction(action);
				}
			}
		}
	}
	return true;
}

void MapContacts::objectUpdated(SceneObject *ASceneObject, int ARole)
{
	switch (ARole)
	{
		case MDR_ALL:
		case MDR_CONTACT_LABEL:
		case MDR_CONTACT_AVATAR:
		case MDR_CONTACT_STATUS_ICON:
			emit mapDataChanged(MOT_CONTACT, ASceneObject->mapObject()->id(), ARole);
			break;

		case  MDR_NONE:
		{
			QGraphicsSimpleTextItem *item=qgraphicsitem_cast<QGraphicsSimpleTextItem *>(ASceneObject->getElementByRole(MDR_CONTACT_LABEL));
			if (item)
			{
				QFont font=item->font();
				bool active=ASceneObject->isActive();
				font.setUnderline(active);
				item->setFont(font);
				item->update();
				int  view=Options::node(OPV_MAP_CONTACTS_VIEW).value().toInt();
				bool full=(view==MCO_VIEW_FULL)||(view==MCO_VIEW_FULLACTIVE&&active);
				ASceneObject->setFull(full);
			}
			else
				LOG_ERROR("Item is NULL!!!");
			break;
		}
	}
}

QString MapContacts::toolTipText(const MapObject *AMapObject, const QHelpEvent *AEvent) const
{
	IRosterIndex *index=getRosterIndex(AMapObject->id());
	QMap<int, QString> toolTips;
	QString result;

	FRostersView->toolTipsForIndex(index, AEvent, toolTips);
	if (!toolTips.isEmpty())
		result = QString("<span>%1</span>").arg(QStringList(toolTips.values()).join("<p/><nbsp>"));
	return result;
}

bool MapContacts::rosterIndexSingleClicked(int AOrder, IRosterIndex *AIndex, const QMouseEvent *AEvent)
{
	Q_UNUSED(AOrder)

	QModelIndex index = FRostersView->mapFromModel(FRostersView->rostersModel()->modelIndexFromRosterIndex(AIndex));
	if (FRostersView->labelAt(AEvent->pos(),index) == FGeoloc->rosterLabelId())
	{
		showContact(AIndex->data(RDR_FULL_JID).toString());
		return true;
	}
	return false;
}

int MapContacts::getShow(const QString &AJid) const
{
	Jid jid(AJid);
	if (jid.resource().isEmpty())	// Bare jid
	{								// Find full jid without geoloc information for it.
		IRosterIndex *index = getRosterIndex(AJid);
		if (index)
		{
			QStringList resources = index->data(RDR_RESOURCES).toStringList();
			for (QStringList::ConstIterator it = resources.constBegin(); it!=resources.constEnd(); it++)
				if (!FGeoloc->hasGeoloc(*it))
				{
					jid = *it;
					break;
				}
		}
	}

	if (!jid.resource().isEmpty())
	for (QList<IRoster *>::ConstIterator it=FRosterList.constBegin(); it!=FRosterList.constEnd(); it++)
	{
		IPresence *presence = FPresenceManager->createPresence((*it)->xmppStream());
		if ((*it)->streamJid() == jid)
			return presence->show();
		else
		{
			IPresenceItem presenceItem = presence->findItem(jid);
			if (!presenceItem.isNull())
				return presenceItem.show;
		}
	}
	return IPresence::Offline;
}

Qt::GlobalColor MapContacts::getColor(const QString &AJid, int AShow) const
{
	GeolocElement geoloc = FGeoloc->getGeoloc(AJid);
	bool online=isOnline(AShow);

	switch (geoloc.reliability())
	{
		case GeolocElement::Reliable: return online?Qt::green:Qt::darkGreen;
		case GeolocElement::WasReliable: return online?Qt::red:Qt::darkRed;
		case GeolocElement::NotReliable: return online?Qt::yellow:Qt::darkYellow;
		default: return online?Qt::darkGray:Qt::black;
	}
}

void MapContacts::updateStatus(MapObject *AContact) const
{
	if (!Options::node(OPV_ROSTER_SHOWOFFLINE).value().toBool())
	{
		int show=getShow(AContact->id());
		AContact->setVisible((show!=IPresence::Offline)&&(show!=IPresence::Error));
	}
	AContact->update(MDR_ALL);
}

IRosterIndex *MapContacts::getRosterIndex(const QString &AId) const
{
	Jid jid(AId);
	for(QList<IRoster *>::const_iterator it=FRosterList.constBegin(); it!=FRosterList.constEnd(); it++)
	{
		QList<IRosterIndex *> indexes=FRostersModel->findContactIndexes((*it)->streamJid(), jid);
		if (!indexes.isEmpty())
			return indexes.first();
	}
	return NULL;
}

QGraphicsItem * MapContacts::mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement)
{
	QString id=ASceneObject->mapObject()->id();
	IRosterIndex *index=getRosterIndex(id);
	if (index)
	{
		int show = getShow(id);
		switch (ARole) // Will return QPixmap, if available
		{
			case MDR_CONTACT_STATUS_ICON:
				if (FStatusIcons)
				{
					QIcon icon=FStatusIcons->iconByStatus(show, "both", false);
					QPixmap pixmap=icon.pixmap(icon.availableSizes().first());

					if (ACurrentElement)
						qgraphicsitem_cast<QGraphicsPixmapItem *>(ACurrentElement)->setPixmap(pixmap);
					else
						ACurrentElement=new QGraphicsPixmapItem(icon.pixmap(icon.availableSizes().first()));
				}
				break;

			case MDR_CONTACT_LABEL:
			{
				QGraphicsDropShadowEffect * dropShadowEffect;
				QGraphicsSimpleTextItem * label;
				QString name=index->data(RDR_NAME).toString();
				if (name.isEmpty())
					name=Jid(id).bare();
				if (ACurrentElement)
				{
					label = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(ACurrentElement);
					dropShadowEffect = qobject_cast<QGraphicsDropShadowEffect *>(label->graphicsEffect());
					label->setText(name);
				}
				else
				{
					label  = new QGraphicsSimpleTextItem(name);
					dropShadowEffect = new QGraphicsDropShadowEffect();
					dropShadowEffect -> setBlurRadius(3);
					dropShadowEffect -> setOffset(0.7);
					label -> setGraphicsEffect(dropShadowEffect);
				}

				if (ASceneObject->isActive())
				{
					QFont font=((QGraphicsSimpleTextItem *)ACurrentElement)->font();
					font.setUnderline(true);
					((QGraphicsSimpleTextItem *)ACurrentElement)->setFont(font);
				}
				Qt::GlobalColor color=getColor(id, show);
				// Set black shadow for bright colors and white shadow for dark
				dropShadowEffect->setColor((color==Qt::green||color==Qt::yellow||color==Qt::red)?Qt::black:Qt::white);
				label->setBrush(QBrush(color));
				return label;
			}

			case MDR_CONTACT_AVATAR:
				if (FAvatars)
				{
					QGraphicsPixmapItem * pixmap=NULL;
					QImage avatar = FAvatars->loadAvatarImage(FAvatars->avatarHash(id), FAvatarSize, (show==IPresence::Offline||show==IPresence::Error));

					if (avatar.isNull() && FShowEmptyAvatars)
						avatar = FEmptyAvatar;

					QSize size=avatar.size();
					int width=size.width();
					int height=size.height();
					if (height<10)
						height=10;
					QPolygonF polygon;
					polygon.append(QPointF(0, 0));
					polygon.append(QPointF(2+width+1, 0));
					polygon.append(QPointF(2+width+1, (2+height-10+2)/2));
					polygon.append(QPointF(2+width+1+5, (2+height+2)/2));
					polygon.append(QPointF(2+width+1, (2+height+10+2)/2));
					polygon.append(QPointF(2+width+1, 2+height+1));
					polygon.append(QPointF(0, 2+height+1));

					QGraphicsPolygonItem *polygonItem;
					if (ACurrentElement)
					{
						polygonItem=qgraphicsitem_cast<QGraphicsPolygonItem *>(ACurrentElement);
						polygonItem->setPolygon(polygon);
						QList<QGraphicsItem *> items=polygonItem->childItems();
						for (QList<QGraphicsItem *>::const_iterator it=items.constBegin(); it!=items.constEnd(); it++ )
							if ((*it)->type()==QGraphicsPixmapItem::Type)
							{
								pixmap=qgraphicsitem_cast<QGraphicsPixmapItem *>(*it);
								if (pixmap)
									pixmap->setPixmap(QPixmap::fromImage(avatar));
								else
									LOG_ERROR("MapObject::mapData(MDR_AVATAR): pixmap=NULL!");
								break;
							}
					}
					else
					{
						polygonItem = new QGraphicsPolygonItem(polygon);
						pixmap = new QGraphicsPixmapItem(QPixmap::fromImage(avatar), polygonItem);
					}
					polygonItem->setBrush(QBrush(getColor(id, show)));
					if (pixmap)
						pixmap->setPos(2, 2);
					return polygonItem;
				}
				break;
		}
	}
	return ACurrentElement;
}

void MapContacts::onIndexDataChanged(IRosterIndex *AIndex, int ARole)
{
	LOG_DEBUG("MapContacts::onIndexDataChanged("+AIndex->data(RDR_FULL_JID).toString()+","+ARole+")");
	if (ARole==RDR_RESOURCES)
	{
		LOG_DEBUG("RDR_RESOURCES");
		QStringList resources=AIndex->data(RDR_RESOURCES).toStringList();
		QStringList resourcesOld=FIndexResourceHash.values(AIndex);

		for (QStringList::const_iterator it=resourcesOld.constBegin(); it!=resourcesOld.constEnd(); it++)
			if (!resources.contains(*it)) // Removed
			{
				MapObject *object=FMap->geoMap()->getObject(MOT_CONTACT, *it);
				if (object)
					updateStatus(object);
			}

		FIndexResourceHash.remove(AIndex);
		for (QStringList::const_iterator it=resources.constBegin(); it!=resources.constEnd(); it++)
			FIndexResourceHash.insertMulti(AIndex, *it);

		resourcesOld=FIndexResourceHash.values(AIndex);
	}
	else if ((ARole==RDR_NAME)||(ARole==RDR_AVATAR_IMAGE)||(ARole==RDR_SHOW))
	{
		LOG_DEBUG("RDR_NAME|RDR_AVATAR_IMAGE|RDR_SHOW");
		QStringList resources=AIndex->data(RDR_RESOURCES).toStringList();
		if (resources.isEmpty())
			resources.append(AIndex->data(RDR_FULL_JID).toString());
		for (QStringList::const_iterator it=resources.constBegin(); it!=resources.constEnd(); it++)
		{
			MapObject *object=FMap->geoMap()->getObject(MOT_CONTACT, *it);
			if (object)
				switch (ARole)
				{
					case RDR_NAME:
						object->update(MDR_CONTACT_LABEL);
						break;

					case RDR_AVATAR_IMAGE:
						object->update(MDR_CONTACT_AVATAR);
						break;

					case RDR_SHOW:
						updateStatus(object);
						break;
				}
		}
	}
}

void MapContacts::addMapContact(const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AUpdateColor)
{
	FResourceHash.insert(AContactJid.bare(), AContactJid.full());
	FContacts.append(AContactJid.full());
	MapObject *object=FMap->geoMap()->addObject(MOT_CONTACT, AContactJid.full(), ACoordinates);
	if (object)
	{
		OptionsNode node=Options::node(OPV_ROSTER_SHOWOFFLINE);
		if (!node.value().toBool())
			object->setVisible(isOnline(AContactJid.full()));
		if (AUpdateColor)
		{
			emit mapDataChanged(MOT_CONTACT, AContactJid.full(), MDR_CONTACT_LABEL);
			emit mapDataChanged(MOT_CONTACT, AContactJid.full(), MDR_CONTACT_AVATAR);
		}
		object->update(MDR_NONE);
	}
}

void MapContacts::removeMapContact(const Jid &AContactJid)
{
	FResourceHash.remove(AContactJid.bare(), AContactJid.full());
	if (FMap->geoMap()->getObject(MOT_CONTACT, AContactJid.full()))
	{
		FMap->geoMap()->removeObject(MOT_CONTACT, AContactJid.full());
		FContacts.removeOne(AContactJid.full());
	}
}


void MapContacts::onLocationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged)
{
	Q_UNUSED(AStreamJid)

	if (getRosterIndex(AContactJid.full()))
		addMapContact(AContactJid, ACoordinates, AReliabilityChanged);
}

void MapContacts::onLocationRemoved(const Jid &AStreamJid, Jid AContactJid)
{
	Q_UNUSED(AStreamJid)
	removeMapContact(AContactJid);
}

void MapContacts::onIndexInserted(IRosterIndex *AIndex)
{
	if (AIndex->kind()==RIK_CONTACT || AIndex->kind()==RIK_MY_RESOURCE)
	{
		Jid contactJid(AIndex->data(RDR_FULL_JID).toString());
		if (FGeoloc->hasGeoloc(contactJid))
			addMapContact(contactJid, FGeoloc->getGeoloc(contactJid).coordinates());
	}
}

void MapContacts::onIndexRemoving(IRosterIndex *AIndex)
{
	removeMapContact(AIndex->data(RDR_FULL_JID).toString());
}

void MapContacts::onMapObjectInserted(int AType, const QString &AId)
{
	Q_UNUSED(AType)
	Q_UNUSED(AId)
}

void MapContacts::onMapObjectRemoved(int AType, const QString &AId)
{
	Q_UNUSED(AType)
	Q_UNUSED(AId)
}

void MapContacts::onMapObjectShowed(int AType, const QString &AId)
{
	if (AType == MOT_CONTACT)
		emit contactShowedOnTheMap(AId);
}

void MapContacts::onRosterOpened(IRoster *ARoster)
{
	FRosterList.append(ARoster);
}

void MapContacts::onRosterClosed(IRoster *ARoster)
{
	FRosterList.removeOne(ARoster);
}

void MapContacts::onShowContact(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		showContact(action->data(ADR_CONTACT_JID).toString());
}

void MapContacts::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	if (AId==SCT_ROSTERVIEW_SHOWCONTACTONTHEMAP)
		if (FRostersView && AWidget==FRostersView->instance() && !FRostersView->hasMultiSelection())
			showContact(FRostersView->instance()->currentIndex().data(RDR_FULL_JID).toString());
}

void MapContacts::onGeolocActionTriggered() const
{
	showContact(qobject_cast<Action *>(sender())->data(ADR_CONTACT_JID).toString());
}

void MapContacts::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
	if (ALabelId == AdvancedDelegateItem::DisplayId || ALabelId == FGeoloc->rosterLabelId())
	{
		QStringList list;

		for(QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
			if((*it)->kind() == RIK_MY_RESOURCE || (*it)->kind() == RIK_CONTACT || (*it)->kind() == RIK_AGENT || (*it)->kind() == RIK_STREAM_ROOT)
			{
				QStringList resources=(*it)->data(RDR_RESOURCES).toStringList();
				if (resources.isEmpty())
					resources.append((*it)->data(RDR_FULL_JID).toString());
				for (QStringList::const_iterator rit=resources.constBegin(); rit!=resources.constEnd(); rit++)
					if(!FGeoloc->getGeoloc(*rit).isEmpty())
						list.append(*rit);
			}

		Menu *menu;
		if (list.size()>1)
		{
			menu = new Menu(AMenu);
			menu->menuAction()->setText(tr("Show on the map"));
			menu->setIcon(RSR_STORAGE_MENUICONS, MNI_GEOLOC);
			AMenu->addAction(menu->menuAction(), AG_RVCM_GEOLOC, true);
		}
		else
			menu=AMenu;

		for (QStringList::const_iterator rit=list.constBegin(); rit!=list.constEnd(); rit++)
		{
			Action *action = new Action(menu);
			if (menu==AMenu)
				action->setText(tr("Show on the map"));
			else
				action->setText(*rit);

			action->setData(ADR_CONTACT_JID, *rit);
			if (FMap->geoMap()->isFollowed(MOT_CONTACT, *rit))
			{
				action->setEnabled(false);
				action->setIcon(RSR_STORAGE_MENUICONS, MNI_GEOLOC_OFF);
			}
			else
			{
				action->setEnabled(true);
				action->setIcon(RSR_STORAGE_MENUICONS, MNI_GEOLOC);
			}
			menu->addAction(action, AG_RVCM_GEOLOC, true);
			connect(action,SIGNAL(triggered(bool)),SLOT(onShowContact(bool)));
		}
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapcontacts, MapContacts)
#endif

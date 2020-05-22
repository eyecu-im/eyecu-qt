#include <QGraphicsDropShadowEffect>
#include <QToolTip>
#include <QLayoutItem>
#include <MapObject>

#include <utils/menu.h>
#include <utils/options.h>
#include <utils/widgetmanager.h>

#include <definitions/menuicons.h>
#include <definitions/mapicons.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/optionvalues.h>

#include "mapform.h"
#include "map.h"
#include "setlocation.h"

#include "ui_mapform.h"

//-----------------

MapForm::MapForm(Map *AMap, MapScene *AMapScene, QWidget *parent) :
	QScrollArea(parent),
	ui(new Ui::MapForm),
	FMap(AMap),
	FMapScene(AMapScene),
	FMapSource(NULL),
	FOldType(TYPE_NONE),
	FHideEventEnabled(false)
{
//	FTypes[0]=-1;
//	FTypes[1]=-1;
//	FTypes[2]=-1;
//	FTypes[3]=-1;

	ui->setupUi(this);

	FGraphicsView =  new QGraphicsView(FMapScene->instance(), this);
	FGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	FGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	FGraphicsView->setFrameStyle(QFrame::Sunken|QFrame::StyledPanel);
	FGraphicsView->setMouseTracking(true);

	ui->frmLocation->raise();
	ui->frmMapCenter->raise();
	ui->frmSelection->raise();
	ui->frmScale->raise();
	ui->frmMapType->raise();
	ui->frmJoystick->raise();
	ui->mapScale->raise();

	QStyle *style = QApplication::style();
	ui->pbDown->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
	ui->pbLeft->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	ui->pbUp->setIcon(style->standardIcon(QStyle::SP_ArrowUp));
	ui->pbReload->setIcon(style->standardIcon(QStyle::SP_BrowserReload));
	ui->pbRight->setIcon(style->standardIcon(QStyle::SP_ArrowRight));

	Shortcuts::bindObjectShortcut(SCT_MAP_REFRESH, ui->pbReload);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_LEFT, ui->pbLeft);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_RIGHT, ui->pbRight);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_UP, ui->pbUp);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_DOWN, ui->pbDown);

	ui->frmMapCenter->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	ui->lblMapCenter->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblMapCenter->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblMapCenterLatLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblMapCenterLatLabel->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblMapCenterLonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblMapCenterLonLabel->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblMapCenterLat->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblMapCenterLat->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblMapCenterLon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblMapCenterLon->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->frmLocation->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->frmLocation->hide();

	ui->lblLocation->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblLocation->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblLocationLatLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblLocationLatLabel->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblLocationLonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblLocationLonLabel->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblLocationLat->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblLocationLat->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblLocationLon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblLocationLon->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->frmSelection->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	ui->lblSelection->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblSelection->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblSelectionLatLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblSelectionLatLabel->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblSelectionLonLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblSelectionLonLabel->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblSelectionLat->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblSelectionLat->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->lblSelectionLon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui->lblSelectionLon->setGraphicsEffect(new QGraphicsDropShadowEffect());

	ui->frmSelection->hide();

	ui->mapScale->setAttribute(Qt::WA_TransparentForMouseEvents, true);

//----
	QString selectStyleSheet = "color: rgb(0,215,127);";
	ui->lblSelection->setStyleSheet(selectStyleSheet);
	ui->lblSelectionLat->setStyleSheet(selectStyleSheet);
	ui->lblSelectionLon->setStyleSheet(selectStyleSheet);
//----

	connect(ui->sldScale, SIGNAL(valueChanged(int)), FMap, SLOT(onSliderValueChanged(int)));
	connect(ui->sldScale, SIGNAL(valueChanged(int)), ui->lcdScale, SLOT(display(int)));
	connect(ui->sldScale, SIGNAL(sliderMoved(int)), ui->lcdScale, SLOT(display(int)));
	connect(FMap, SIGNAL(zoomChanged(int)), ui->sldScale, SLOT(setValue(int)));

	connect(ui->pbLeft, SIGNAL(clicked()), SLOT(onStepLeft()));
	connect(ui->pbRight, SIGNAL(clicked()), SLOT(onStepRight()));
	connect(ui->pbUp, SIGNAL(clicked()), SLOT(onStepUp()));
	connect(ui->pbDown, SIGNAL(clicked()), SLOT(onStepDown()));
	connect(ui->pbReload, SIGNAL(clicked()), FMapScene->instance(), SLOT(reloadMap()));

	connect(ui->cmbMapSource, SIGNAL(currentIndexChanged(int)),SLOT(onSourceSelected(int)));

	connect(FMapScene->instance(), SIGNAL(mppChanged(double)),SLOT(onMppChanged(double)));
	connect(FMapScene->instance(), SIGNAL(sceneRectChanged(QRectF)), SLOT(onSceneRectChanged(QRectF)));

	// Map scene on-screen display initialization
	float ZVLine = 6.0;
	QPen pen(Qt::red);
	pen.setWidth(1);

	QGraphicsScene *scene=FMapScene->instance();
	FLineX  = scene->addLine(0, 0, 0, 0, pen);    FLineX->setZValue(ZVLine);
	FLine1X = scene->addLine(0, -2, 0, +2, pen);    FLine1X->setZValue(ZVLine);
	FLine2X = scene->addLine(0, -4, 0, +4, pen);    FLine2X->setZValue(ZVLine);
	FLineX1 = scene->addLine(0, -2, 0, +2, pen);    FLineX1->setZValue(ZVLine);
	FLineX2 = scene->addLine(0, -4, 0, +4, pen);    FLineX2->setZValue(ZVLine);

	FLineY  = scene->addLine(0, 0, 0, 0, pen);    FLineY->setZValue(ZVLine);
	FLine1Y = scene->addLine(-2, 0, +2, 0, pen);    FLine1Y->setZValue(ZVLine);
	FLine2Y = scene->addLine(-4, 0, +4, 0, pen);    FLine2Y->setZValue(ZVLine);
	FLineY1 = scene->addLine(-2, 0, +2, 0, pen);    FLineY1->setZValue(ZVLine);
	FLineY2 = scene->addLine(-4, 0, +4, 0, pen);    FLineY2->setZValue(ZVLine);

	FMapScene->instance()->setSceneRect(0, 0, width(), height());
}
//--------------------------------------------
MapForm::~MapForm()
{
	delete ui;
}

void MapForm::showCentralPage(bool AMinimized)
{
	if (!AMinimized)
		showWindow();
	else
		showMinimizedWindow();
}

QIcon MapForm::centralPageIcon() const
{
	return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(FMapSource?FMapSource->getIconId():MNI_MAP);
}

QString MapForm::centralPageCaption() const
{
	MapSource *source=FMapScene->mapSource();
	return (source)?tr("Map (%1: %2)").arg(FMapScene->mapSource()->getFriendlyName())
									  .arg(FMapScene->mapSource()->getModeNames().value(Options::node(OPV_MAP_MODE).value().toInt()))
				   :tr("Map");
}

void MapForm::updateMapTitle()
{
	if (isWindow())
	{
		setWindowIcon(centralPageIcon());
		setWindowTitle(centralPageCaption());
	}
	else
		emit centralPageChanged();
}

void MapForm::closeOptions()
{
	saveWindowGeometry();
}

void MapForm::setMapCenter(const QPointF &ACoords)
{
	QString latitude, longitude;
	getCoordStrings(ACoords, latitude, longitude);
	ui->lblMapCenterLat->setText(latitude);
	ui->lblMapCenterLon->setText(longitude);
}

/// Events
void MapForm::changeEvent(QEvent *e)
{
	QScrollArea::changeEvent(e);
	switch (e->type())
	{
		case QEvent::LanguageChange: ui->retranslateUi(this); break;
		default: break;
	}
}

void MapForm::resizeEvent(QResizeEvent *e)
{
	QScrollArea::resizeEvent(e);
	switch (e->type())
	{
		case QResizeEvent::Resize:
			graphicsViewResize(e);
			break;

		default:
			break;
	}
}

void MapForm::showEvent(QShowEvent *e)
{
	Q_UNUSED(e)
	Options::node(OPV_MAP_SHOWING).setValue(true);
}

void MapForm::hideEvent(QHideEvent *e)
{
	Q_UNUSED(e)
	if (FHideEventEnabled)   // To prevent show/hide state saving during application shutdown
		if (!Options::node(OPV_MAP_ATTACH_TO_ROSTER).value().toBool())
			Options::node(OPV_MAP_SHOWING).setValue(false);
}

bool MapForm::event(QEvent *AEvent)
{
	if (AEvent->type() == QEvent::ToolTip)
	{
		QString toolTipText;
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(AEvent);
		SceneObject *sceneObject = FMapScene->activeObject();
		if (sceneObject)
		{
			MapObject *mapObject = sceneObject->mapObject();
			if (mapObject)
				toolTipText=mapObject->toolTipText(helpEvent);
		}
		if (toolTipText.isEmpty())
		{
			QToolTip::hideText();
			AEvent->ignore();
		}
		else
			QToolTip::showText(helpEvent->globalPos(), toolTipText);
		return true;
	}
	return QScrollArea::event(AEvent); // Invoke parent event handler
}

void MapForm::showWindow()
{
	if (isWindow())
		WidgetManager::showActivateRaiseWindow(this);
	else
		emit centralPageShow(false);
}

void MapForm::hideWindow()
{
	if (isWindow() && !isMinimized())
		hide();
	else
		emit centralPageDestroyed();
}

void MapForm::showMinimizedWindow()
{
	if (!isWindow())
		emit centralPageShow(true);
	else if (!isVisible())
		showMinimized();
}

//TODO: Move into MapScene class!
QRect MapForm::sceneRect() const
{
	QRectF rectf=FMapScene->instance()->sceneRect();
	QRect rect(rectf.x(), rectf.y(), rectf.width(), rectf.height());
	rect.translate(-rect.width()/2, -rect.height()/2);
	return rect;
}

//--------
void MapForm::graphicsViewResize(QResizeEvent *AResizeEvent)
{
	Q_UNUSED(AResizeEvent)
	QRect pv(QPoint(0, 0), geometry().size());
	FMapScene->instance()->setSceneRect(pv);
	FGraphicsView->setGeometry(pv);
}

void MapForm::setOsdFont(const QFont &AFont)
{
	ui->lblMapCenter->setFont(AFont);
	ui->lblMapCenterLatLabel->setFont(AFont);
	ui->lblMapCenterLonLabel->setFont(AFont);
	ui->lblMapCenterLat->setFont(AFont);
	ui->lblMapCenterLon->setFont(AFont);

	ui->lblLocation->setFont(AFont);
	ui->lblLocationLatLabel->setFont(AFont);
	ui->lblLocationLonLabel->setFont(AFont);
	ui->lblLocationLat->setFont(AFont);
	ui->lblLocationLon->setFont(AFont);

	ui->lblSelection->setFont(AFont);
	ui->lblSelectionLatLabel->setFont(AFont);
	ui->lblSelectionLonLabel->setFont(AFont);
	ui->lblSelectionLat->setFont(AFont);
	ui->lblSelectionLon->setFont(AFont);

	ui->cmbMapSource->setFont(AFont);
}

void MapForm::setOsdTextColor(const QColor &ATextColor)
{
	QString css = QString("QLabel {color: %1}").arg(ATextColor.name());
	ui->lblMapCenter->setStyleSheet(css);
	ui->lblMapCenterLatLabel->setStyleSheet(css);
	ui->lblMapCenterLonLabel->setStyleSheet(css);
	ui->lblMapCenterLat->setStyleSheet(css);
	ui->lblMapCenterLon->setStyleSheet(css);
}

void MapForm::setOsdBoxColor(QPalette::ColorRole ARole, QColor AColor)
{
	// Set frame palette
	FBoxPalette.setColor(ARole, AColor);
	ui->frmLocation->setPalette(FBoxPalette);
	ui->frmMapCenter->setPalette(FBoxPalette);
	ui->frmJoystick->setPalette(FBoxPalette);
	ui->frmMapType->setPalette(FBoxPalette);
	ui->frmSelection->setPalette(FBoxPalette);
	ui->frmScale->setPalette(FBoxPalette);
}

void MapForm::setOsdBoxShape(int AShape)
{   // Set frame shape
	QFrame::Shape shape=(QFrame::Shape)AShape;
	ui->frmLocation->setFrameShape(shape);
	ui->frmMapCenter->setFrameShape(shape);
	ui->frmJoystick->setFrameShape(shape);
	ui->frmMapType->setFrameShape(shape);
	ui->frmSelection->setFrameShape(shape);
	ui->frmScale->setFrameShape(shape);
}

void MapForm::setOsdBoxShadow(int AShadow)
{   // Set frame shadow
	QFrame::Shadow shadow=(QFrame::Shadow)AShadow;
	ui->frmLocation->setFrameShadow(shadow);
	ui->frmMapCenter->setFrameShadow(shadow);
	ui->frmJoystick->setFrameShadow(shadow);
	ui->frmMapType->setFrameShadow(shadow);
	ui->frmSelection->setFrameShadow(shadow);
	ui->frmScale->setFrameShadow(shadow);
}

void MapForm::setOsdBoxBgTransparent(bool ATransparent)
{
	ui->frmLocation->setAutoFillBackground(!ATransparent);
	ui->frmMapCenter->setAutoFillBackground(!ATransparent);
	ui->frmJoystick->setAutoFillBackground(!ATransparent);
	ui->frmMapType->setAutoFillBackground(!ATransparent);
	ui->frmSelection->setAutoFillBackground(!ATransparent);
	ui->frmScale->setAutoFillBackground(!ATransparent);
}

void MapForm::setOsdControlColor(QPalette::ColorRole ARole, const QColor &AColor)
{
	FControlPalette.setColor(ARole, AColor);
	for (int i=0; i<ui->layoutModeButtons->count(); ++i)
		ui->layoutModeButtons->itemAt(i)->widget()->setPalette(FControlPalette);
	ui->lcdScale->setPalette(FControlPalette);
	ui->sldScale->setPalette(FControlPalette);
	ui->mapScale->setPalette(FControlPalette);
}

void MapForm::setOsdControlBgTransparent(bool ATransparent)
{
	for (int i=0; i<ui->layoutModeButtons->count(); ++i)
		ui->layoutModeButtons->itemAt(i)->widget()->setAutoFillBackground(!ATransparent);
	ui->lcdScale->setAutoFillBackground(!ATransparent);
	ui->sldScale->setAutoFillBackground(!ATransparent);
	ui->mapScale->setAutoFillBackground(!ATransparent);
}

//---shadow---
void MapForm::setOsdShadow(const QColor &AColor, qreal ABlur, const QPointF &AShift)
{
	QGraphicsDropShadowEffect *dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblMapCenter->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblMapCenterLat->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblMapCenterLatLabel->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblMapCenterLon->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblMapCenterLonLabel->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblLocation->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblLocationLat->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblLocationLatLabel->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblLocationLon->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblLocationLonLabel->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblSelection->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblSelectionLat->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblSelectionLatLabel->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblSelectionLon->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);

	dropShadowEffect=qobject_cast<QGraphicsDropShadowEffect *>(ui->lblSelectionLonLabel->graphicsEffect());
	dropShadowEffect->setColor(AColor);
	dropShadowEffect->setBlurRadius(ABlur);
	dropShadowEffect->setOffset(AShift);
}

void MapForm::setOsdCenterMarkerColor(const QColor &AColor)
{
	FLine1X->setPen(AColor);
	FLine1Y->setPen(AColor);
	FLine2X->setPen(AColor);
	FLine2Y->setPen(AColor);
	FLineX->setPen(AColor);
	FLineX1->setPen(AColor);
	FLineX2->setPen(AColor);
	FLineY->setPen(AColor);
	FLineY1->setPen(AColor);
	FLineY2->setPen(AColor);
}

void MapForm::setOsdCenterMarkerVisible(bool AVisible)
{
	FLine1X->setVisible(AVisible);
	FLine1Y->setVisible(AVisible);
	FLine2X->setVisible(AVisible);
	FLine2Y->setVisible(AVisible);
	FLineX->setVisible(AVisible);
	FLineX1->setVisible(AVisible);
	FLineX2->setVisible(AVisible);
	FLineY->setVisible(AVisible);
	FLineY1->setVisible(AVisible);
	FLineY2->setVisible(AVisible);
}

void MapForm::setZoomSliderTracknig(bool AEnable)
{
	ui->sldScale->setTracking(AEnable);
}

void MapForm::saveWindowGeometry()
{
	if (isWindow())
		Options::setFileValue(saveGeometry(), OPV_MAP_GEOMETRY);
}

void MapForm::loadWindowGeometry()
{
	if (isWindow())
		if (!restoreGeometry(Options::fileValue(OPV_MAP_GEOMETRY).toByteArray()))
			setGeometry(WidgetManager::alignGeometry(QSize(480,480),this,Qt::AlignCenter));
}

void MapForm::enableHideEvent()
{
	FHideEventEnabled=true;
}

void MapForm::disableHideEvent()
{
	FHideEventEnabled=false;
}

void MapForm::centerMousePointer() const
{
	QPoint center(graphicsView()->width()/2, graphicsView()->height()/2);
	QPoint globalCenter = graphicsView()->mapToGlobal(center);
	FMapScene->setIgnoreMouseMovement(true);
	QCursor::setPos(globalCenter);
}

void MapForm::showMapCenter(bool AShow)
{
	ui->frmMapCenter->setVisible(AShow);
}

void MapForm::addMapSource(const QString &AName, const QIcon &AIcon, const QUuid &AUuid)
{
	if (AIcon.isNull())
		ui->cmbMapSource->addItem(AName, AUuid.toString());
	else
		ui->cmbMapSource->addItem(AIcon, AName, AUuid.toString());
}

void MapForm::selectMapSource(const QUuid &AUuid)
{
	ui->cmbMapSource->setCurrentIndex(ui->cmbMapSource->findData(AUuid.toString()));
}

void MapForm::setMapSource(IMapSource *AMapSource)
{
	int index = chooseMapSource(FMapSource=AMapSource);
	FMapScene->setMapSource(AMapSource?AMapSource->mapSource():nullptr);
	if (index!=-1)
		selectMapMode(index);
}

void MapForm::setMapMode(qint8 AMode)
{
	FOldType = getType(AMode);

	QLayoutItem *item = ui->layoutModeButtons->itemAt(AMode);
	if (item)
	{
		QPushButton *button = qobject_cast<QPushButton *>(item->widget());
		if (button)
			button->setChecked(true);
	}

	FMapScene->selectMode(AMode);
	FMapScene->updateMercatorType();
	FMapScene->updateZoom();
	ui->sldScale->setMinimum(FMapScene->zoomMin());
	ui->sldScale->setMaximum(FMapScene->zoomMax());
	updateMapTitle();
}

QIcon MapForm::getIcon(int AIconIndex) const
{
	switch (AIconIndex)
	{
		case ICON_MAP:
			return FMap->getIcon(MPI_MAP);
		case ICON_MAP1:
			return FMap->getIcon(MPI_MAP1);
		case ICON_MAP2:
			return FMap->getIcon(MPI_MAP2);
		case ICON_MAP3:
			return FMap->getIcon(MPI_MAP3);
		case ICON_SATELLITE:
			return FMap->getIcon(MPI_SATELLITE);
		case ICON_SATELLITE1:
			return FMap->getIcon(MPI_SATELLITE1);
		case ICON_HYBRID:
			return FMap->getIcon(MPI_HYBRID);
		case ICON_HYBRID1:
			return FMap->getIcon(MPI_HYBRID1);
		case ICON_TERRAIN:
			return FMap->getIcon(MPI_TERRAIN);
		default:
			return QIcon();
	}
}

void MapForm::getCoordStrings(const QPointF &ACoords, QString &ALatitude, QString &ALongitude)
{
	ALatitude=getLatString(ACoords.y());
	ALongitude=getLonString(ACoords.x());
}

QString MapForm::getLatString(qreal ALatitude)
{
	QString lat;
	lat.setNum(ALatitude, 'f', -1);
	return getLatString(lat);
}

QString MapForm::getLonString(qreal ALongitude)
{
	QString lon;
	lon.setNum(ALongitude, 'f', -1);
	return getLonString(lon);
}

QString MapForm::getLatString(const QString &ALatitude)
{
	return (ALatitude[0]=='-')?tr("%1S").arg(ALatitude.mid(1, 8)):tr("%1N").arg(ALatitude.mid(0, 8));
}

QString MapForm::getLonString(const QString &ALongitude)
{
	return (ALongitude[0]=='-')?tr("%1W").arg(ALongitude.mid(1, 8)):tr("%1E").arg(ALongitude.mid(0, 8));
}

int MapForm::chooseMapSource(IMapSource *ASource)
{
	int index=-1;

	QList<QString> modeNames;
	QList<int> modeIcons;
	QList<int> modeTypes;

	if (ASource)
	{
		modeTypes=ASource->mapSource()->getModeTypes();
		modeNames=ASource->mapSource()->getModeNames();
		modeIcons=ASource->getModeIcons();
	}

	while (QLayoutItem *button = ui->layoutModeButtons->takeAt(0))
	{
		delete button->widget();
		delete button;
	}

	FTypes.clear();

	int i=0;
	for (QList<int>::ConstIterator it = modeTypes.constBegin(); it!=modeTypes.constEnd(); ++it, ++i)
	{		
		QIcon icon = getIcon(modeIcons.at(i));
		QPushButton *button = new QPushButton(icon, QString());
		button->setToolTip(modeNames.at(i));
		button->setAutoExclusive(true);
		button->setCheckable(true);
		button->setFlat(true);
		button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(button, SIGNAL(clicked(bool)), SLOT(onTypeSelectClicked(bool)));
		ui->layoutModeButtons->addWidget(button, 0, Qt::AlignCenter|Qt::AlignCenter);
		FTypes.insert(i, *it);
		if (*it==FOldType && index==-1)
			index=i;
	}

	if (ASource)
	{
		if (index==-1)  // Not found - check for neares match
		{
			if (FOldType != TYPE_NONE)                 // Initialization
			{
				int i=0;
				for (QList<int>::ConstIterator it=FTypes.constBegin(); it!=FTypes.constEnd(); ++it, ++i)
					if (FOldType==TYPE_HYBRID)
					{
						if (*it==TYPE_SATELLITE)
						{
							index=i;
							break;
						}
					}
					else if (FOldType==TYPE_SATELLITE)
					{
						if (*it==TYPE_HYBRID)
						{
							index=i;
							break;
						}
					}
			}

			if (index==-1)  // Not found - fall back to first available index
			{
				int i=0;
				for (QList<int>::ConstIterator it=FTypes.constBegin(); it!=FTypes.constEnd(); ++it, ++i)
					if (*it!=-1)
					{
						index=i;
						break;
					}
			}
		}
	}
	return index;
}

void MapForm::selectMapMode(qint8 AMode)
{
	if (Options::node(OPV_MAP_MODE).value().toInt() == AMode)
		setMapMode(AMode);
	else
		Options::node(OPV_MAP_MODE).setValue(AMode);
}

void MapForm::setOwnLocation(const QString &ALatitude, const QString &ALongitude, GeolocElement::Reliability AReliable)//data from "Geoloc"
{
	QString css = QString("QLabel {background-color: rgba(0, 0, 0, 0); color: %1; }")
			.arg(AReliable==GeolocElement::Reliable   ?"green":
				 AReliable==GeolocElement::WasReliable?"red":
				 AReliable==GeolocElement::NotReliable?"yellow":
													   "black");

	ui->lblLocation->setStyleSheet(css);
	ui->lblLocationLat->setStyleSheet(css);
	ui->lblLocationLatLabel->setStyleSheet(css);
	ui->lblLocationLon->setStyleSheet(css);
	ui->lblLocationLonLabel->setStyleSheet(css);

	ui->lblLocationLat->setText(getLatString(ALatitude));
	ui->lblLocationLon->setText(getLonString(ALongitude));
	if (!ui->frmLocation->isVisible())
		ui->frmLocation->show();
}

void MapForm::removeOwnLocation()
{
	if (ui->frmLocation->isVisible())
		ui->frmLocation->hide();
	ui->lblLocationLat->setText("");
	ui->lblLocationLon->setText("");
}

void MapForm::onSetNewCenter()
{
	SetLocation *newCenter = new SetLocation(FMapScene->mapCenter(), FMap->getIcon(MPI_NEWCENTER), this);
	if(newCenter->exec())
		FMapScene->setMapCenter(newCenter->coordinates());
	newCenter->deleteLater();
}

void MapForm::onSourceSelected(int AIndex)
{
	Options::node(OPV_MAP_SOURCE).setValue(ui->cmbMapSource->itemData(AIndex).toString());
}

/*************************************/
void MapForm::onTypeSelectClicked(bool ASelected)
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	if (button)
	{
		if (ASelected)
		{
			int index = ui->layoutModeButtons->indexOf(button);
			if (index!=-1)
				selectMapMode(index);
		}
		else
		{
			button->blockSignals(true);
			button->setChecked(true);
			button->blockSignals(false);
		}
	}
}

void MapForm::onMppChanged(double mpp)
{
	ui->mapScale->setMpp(mpp);
	adjustCentralRulers(FMapScene->center());
}

void MapForm::onSceneRectChanged(QRectF rect)
{
	adjustCentralRulers(rect.center());
}

//----------------
void MapForm::onStepLeft(int delta){ FMapScene->shiftMap(-delta, 0); FMap->stopFollowing();}
void MapForm::onStepRight(int delta){ FMapScene->shiftMap(delta, 0); FMap->stopFollowing();}
void MapForm::onStepUp(int delta){ FMapScene->shiftMap(0, -delta); FMap->stopFollowing();}
void MapForm::onStepDown(int delta){ FMapScene->shiftMap(0, delta); FMap->stopFollowing();}

void MapForm::adjustCentralRulers(const QPointF &ACenter)
{
	int     l=ui->mapScale->length();
	FLineX->setPos(ACenter.x(), ACenter.y());        FLineX->setLine(-ACenter.x(), -0, ACenter.x(), 0);
	FLine1X->setPos(ACenter.x()-l/2, ACenter.y());
	FLine2X->setPos(ACenter.x()-l, ACenter.y());
	FLineX1->setPos(ACenter.x()+l/2, ACenter.y());
	FLineX2->setPos(ACenter.x()+l, ACenter.y());

	FLineY->setPos(ACenter.x(), ACenter.y());        FLineY->setLine(0,-ACenter.y(), 0, ACenter.y());
	FLine1Y->setPos(ACenter.x(), ACenter.y()-l/2);
	FLine2Y->setPos(ACenter.x(), ACenter.y()-l);
	FLineY1->setPos(ACenter.x(), ACenter.y()+l/2);
	FLineY2->setPos(ACenter.x(), ACenter.y()+l);
}
//----------------

void MapForm::showSelectionBox(bool AShow)
{
	ui->frmSelection->setVisible(AShow);
}

void MapForm::setSelectionCoordinates(const MercatorCoordinates &ACoordinates)
{
	ui->lblSelectionLat->setText(getLatString(ACoordinates.latitude()));
	ui->lblSelectionLon->setText(getLonString(ACoordinates.longitude()));
}

QGraphicsView *MapForm::graphicsView() const
{
	return FGraphicsView;
}

/*************************************
MapGraphicsView::MapGraphicsView(QWidget *parent): QGraphicsView(parent)
{}

void MapGraphicsView::paintEvent(QPaintEvent *event)
{
	qDebug() << "MapGraphicsView::paintEvent(" << event << ")";
	QGraphicsView::paintEvent(event);
	qDebug() << "MapGraphicsView::paintEvent(): finished!";
}
*************************************/

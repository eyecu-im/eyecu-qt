#include <QGraphicsDropShadowEffect>
#include <QToolTip>
#include <MapObject>

#include <interfaces/imap.h>
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
	FTypes[0]=-1;
	FTypes[1]=-1;
	FTypes[2]=-1;
	FTypes[3]=-1;

	ui->setupUi(this);
	ui->graphicsView->setScene(FMapScene->instance());

	QStyle *style = QApplication::style();
	ui->btnDown->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
	ui->btnLeft->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
	ui->btnUp->setIcon(style->standardIcon(QStyle::SP_ArrowUp));
	ui->btnReload->setIcon(style->standardIcon(QStyle::SP_BrowserReload));
	ui->btnRight->setIcon(style->standardIcon(QStyle::SP_ArrowRight));

	Shortcuts::bindObjectShortcut(SCT_MAP_REFRESH, ui->btnReload);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_LEFT, ui->btnLeft);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_RIGHT, ui->btnRight);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_UP, ui->btnUp);
	Shortcuts::bindObjectShortcut(SCT_MAP_MOVE_DOWN, ui->btnDown);

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

	connect(ui->btnLeft, SIGNAL(clicked()), SLOT(onStepLeft()));
	connect(ui->btnRight, SIGNAL(clicked()), SLOT(onStepRight()));
	connect(ui->btnUp, SIGNAL(clicked()), SLOT(onStepUp()));
	connect(ui->btnDown, SIGNAL(clicked()), SLOT(onStepDown()));
	connect(ui->btnReload, SIGNAL(clicked()), FMapScene->instance(), SLOT(reloadMap()));

	connect(ui->rbtMode1, SIGNAL(clicked()), SLOT(onTypeSelected()));
	connect(ui->rbtMode2, SIGNAL(clicked()), SLOT(onTypeSelected()));
	connect(ui->rbtMode3, SIGNAL(clicked()), SLOT(onTypeSelected()));
	connect(ui->rbtMode4, SIGNAL(clicked()), SLOT(onTypeSelected()));
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
	//TODO: Move into MapScene class!!!
	FMapScene->instance()->setSceneRect(0, 0, AResizeEvent->size().width(), AResizeEvent->size().height());

	QRect gv(QPoint(0, 0), AResizeEvent->size());
	ui->graphicsView->setGeometry(gv);
	ui->frmScale->move(gv.x()+4, gv.height()-25);
	ui->frmJoystick->move(gv.x()+4, gv.height()-82);
	ui->frmMapType->move(gv.x()+61, gv.height()-82); //x=60 y=81
	ui->mapScale->move(ui->frmMapType->x()+ui->frmMapType->width()+1, gv.height()-82);
	ui->frmLocation->move(gv.x()+4,4);
	ui->frmMapCenter->move(gv.width()-104, 4);
	ui->frmSelection->move(gv.width()-104, gv.height()-54);
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
	ui->rbtMode1->setPalette(FControlPalette);
	ui->rbtMode2->setPalette(FControlPalette);
	ui->rbtMode3->setPalette(FControlPalette);
	ui->rbtMode4->setPalette(FControlPalette);
	ui->rbtMode4->setPalette(FControlPalette);
	ui->lcdScale->setPalette(FControlPalette);
	ui->sldScale->setPalette(FControlPalette);
	ui->mapScale->setPalette(FControlPalette);
}

void MapForm::setOsdControlBgTransparent(bool ATransparent)
{
	ui->rbtMode1->setAutoFillBackground(!ATransparent);
	ui->rbtMode2->setAutoFillBackground(!ATransparent);
	ui->rbtMode3->setAutoFillBackground(!ATransparent);
	ui->rbtMode4->setAutoFillBackground(!ATransparent);
	ui->rbtMode4->setAutoFillBackground(!ATransparent);
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
	QPoint center(ui->graphicsView->width()/2,ui->graphicsView->height()/2);
	QPoint globalCenter=ui->graphicsView->mapToGlobal(center);
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
	if (index==-1)
		index = 0;
	FMapScene->setMapSource(AMapSource?AMapSource->mapSource():NULL);
	selectMapMode(index);
}

void MapForm::setMapMode(qint8 AMode)
{
	FOldType = FTypes[AMode];
	switch (AMode)
	{
		case 0: ui->rbtMode1->setChecked(true); break;
		case 1: ui->rbtMode2->setChecked(true); break;
		case 2: ui->rbtMode3->setChecked(true); break;
		case 3: ui->rbtMode4->setChecked(true); break;
	}
	FMapScene->selectMode(AMode);
	FMapScene->updateMercatorType();
	FMapScene->updateZoom();
	ui->sldScale->setMinimum(FMapScene->zoomMin());
	ui->sldScale->setMaximum(FMapScene->zoomMax());
	updateMapTitle();
}

void MapForm::setImage(QLabel *ALabel, int AType)
{
	switch (AType)
	{
		case ICON_MAP:
			ALabel->setPixmap(FMap->getIcon(MPI_MAP).pixmap(16));
			break;
		case ICON_MAP1:
			ALabel->setPixmap(FMap->getIcon(MPI_MAP1).pixmap(16));
			break;
		case ICON_MAP2:
			ALabel->setPixmap(FMap->getIcon(MPI_MAP2).pixmap(16));
			break;
		case ICON_SATELLITE:
			ALabel->setPixmap(FMap->getIcon(MPI_SATELLITE).pixmap(16));
			break;
		case ICON_HYBRID:
			ALabel->setPixmap(FMap->getIcon(MPI_HYBRID).pixmap(16));
			break;
		case ICON_TERRAIN:
			ALabel->setPixmap(FMap->getIcon(MPI_TERRAIN).pixmap(16));
			break;
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

	for (int i=0; i<4; i++)
		FTypes[i]=i<modeTypes.size()?modeTypes.at(i):TYPE_NONE;

	if (FTypes[0]==TYPE_NONE)
	{
		ui->rbtMode1->hide();
		ui->lblType1->hide();
	}
	else
	{
		ui->rbtMode1->setToolTip(modeNames.at(0));
		ui->lblType1->setToolTip(modeNames.at(0));
		setImage(ui->lblType1, modeIcons.at(0));
		ui->rbtMode1->show();
		ui->lblType1->show();
		if (FTypes[0]==FOldType && index==-1)
			index=0;
	}

	if (FTypes[1]==TYPE_NONE)
	{
		ui->rbtMode2->hide();
		ui->lblType2->hide();
	}
	else
	{
		ui->rbtMode2->setToolTip(modeNames.at(1));
		ui->lblType2->setToolTip(modeNames.at(1));
		setImage(ui->lblType2, modeIcons.at(1));
		ui->rbtMode2->show();
		ui->lblType2->show();
		if (FTypes[1]==FOldType && index==-1)
			index=1;
	}

	if (FTypes[2]==TYPE_NONE)
	{
		ui->rbtMode3->hide();
		ui->lblType3->hide();
	}
	else
	{
		ui->rbtMode3->setToolTip(modeNames.at(2));
		ui->lblType3->setToolTip(modeNames.at(2));
		setImage(ui->lblType3, modeIcons.at(2));
		ui->rbtMode3->show();
		ui->lblType3->show();
		if (FTypes[2]==FOldType && index==-1)
			index=2;
	}

	if (FTypes[3]==TYPE_NONE)
	{
		ui->rbtMode4->hide();
		ui->lblType4->hide();
	}
	else
	{
		ui->rbtMode4->setToolTip(modeNames.at(3));
		ui->lblType4->setToolTip(modeNames.at(3));
		setImage(ui->lblType4, modeIcons.at(3));
		ui->rbtMode4->show();
		ui->lblType4->show();
		if (FTypes[3]==FOldType && index==-1)
			index=3;
	}

	if (FOldType==TYPE_NONE)                 // Initialization
		return -1;

	if (ASource)
	{
		if (index==-1)  // Not found - check for neares match
			for (int i=0; i<3; i++)
				if (FOldType==TYPE_HYBRID)
				{
					if (FTypes[i]==TYPE_SATELLITE)
					{
						index=i;
						break;
					}
				}
				else if (FOldType==TYPE_SATELLITE)
				{
					if (FTypes[i]==TYPE_HYBRID)
					{
						index=i;
						break;
					}
				}

		if (index==-1)  // Not found - fall back to first available index
			for (int i=0; i<3; i++)
				if (FTypes[i]!=-1)
				{
					index=i;
					break;
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
void MapForm::onTypeSelected()
{
	if (sender()==ui->rbtMode1)
		selectMapMode(0);
	else if (sender()==ui->rbtMode2)
		selectMapMode(1);
	else if (sender()==ui->rbtMode3)
		selectMapMode(2);
	else if (sender()==ui->rbtMode4)
		selectMapMode(3);
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
void MapForm::onStepLeft(int delta){ FMapScene->shiftMap(-delta, 0); }
void MapForm::onStepRight(int delta){ FMapScene->shiftMap(delta, 0); }
void MapForm::onStepUp(int delta){ FMapScene->shiftMap(0, -delta); }
void MapForm::onStepDown(int delta){ FMapScene->shiftMap(0, delta); }

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

QGraphicsView &MapForm::graphicsView() const
{
	return *ui->graphicsView;
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

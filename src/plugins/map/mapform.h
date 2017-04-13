#ifndef MAPFORM_H
#define MAPFORM_H

#include <QGraphicsView>
#include <QScrollArea>
#include <QLabel>
#include <QUuid>
#include <GeolocElement>

#include <interfaces/imap.h>
#include <interfaces/imainwindow.h>

// Mouse hit radius (pels)
#define DELTA_JST       20
#define MAP_MOUSE_HIT_RADIUS (float)4

//-------------------------

class MapScene;

namespace Ui {
	class MapForm;
}

class Map;

class MapForm : public QScrollArea,
				public IMainCentralPage
{
	Q_OBJECT
	Q_INTERFACES (IMainCentralPage)

public:
	MapForm(Map *AMap, MapScene *AMapScene, QWidget *parent = 0);
	~MapForm();

	// IMainCentralPage
	QWidget *instance() {return this;}
	void showCentralPage(bool AMinimized = false);
	QIcon centralPageIcon() const;
	QString centralPageCaption() const;

	void setMapSource(IMapSource *AMapSource);
	void setMapMode(qint8 AMode);
	void setMapCenter(const QPointF &ACoords);
	IMapSource *mapSource() {return FMapSource;}
	void updateMapTitle();
	void closeOptions();
	void setOwnLocation(const QString &ALatitude, const QString &ALongitude, GeolocElement::Reliability AReliable);
	void removeOwnLocation();
	MapScene *mapScene() const {return FMapScene; }
	QRect sceneRect() const;
	void showSelectionBox(bool AShow);
	void setSelectionCoordinates(const MercatorCoordinates &ACoordinates);
	inline qint8 getType(qint8 ATypeIndex) {return (ATypeIndex)||(ATypeIndex>FTypes.size()-1)?-1:FTypes.at(ATypeIndex);}
	QGraphicsView *graphicsView() const;
	void graphicsViewResize(QResizeEvent *AResizeEvent);
	void setOsdFont(const QFont &AFont);
	void setOsdTextColor(const QColor &ATextColor);
	void setOsdBoxColor(QPalette::ColorRole ARole, QColor AColor);
	void setOsdBoxShape(int AShape);
	void setOsdBoxShadow(int AShadow);
	void setOsdBoxBgTransparent(bool ATransparent);
	void setOsdControlColor(QPalette::ColorRole ARole, const QColor &AColor);
	void setOsdControlBgTransparent(bool ATransparent);
	void setOsdShadow(const QColor &AColor, qreal ABlur, const QPointF &AShift);
	void setOsdCenterMarkerColor(const QColor &AColor);
	void setOsdCenterMarkerVisible(bool AVisible);
	void setZoomSliderTracknig(bool AEnable);
	void saveWindowGeometry();
	void loadWindowGeometry();
	void enableHideEvent();
	void disableHideEvent();
	void centerMousePointer() const;
	void showMapCenter(bool AShow);
	void addMapSource(const QString &AName, const QIcon &AIcon = QIcon(), const QUuid &AUuid = QUuid());
	void selectMapSource(const QUuid &AUuid = QUuid());

protected: // Events
	void changeEvent(QEvent *e);
	void resizeEvent(QResizeEvent *e);
	void showEvent(QShowEvent *e);
	void hideEvent(QHideEvent *e);
	bool event(QEvent *AEvent);

protected:
	void adjustCentralRulers(const QPointF &ACenter);
	int  chooseMapSource(IMapSource *ASource);
	void selectMapMode(qint8 AMode);
	void setImage(QLabel *ALabel, int AType);
	void setImage(QAbstractButton *AButton, int AType);
	QIcon getIcon(int AIconIndex) const;
	static void getCoordStrings(const QPointF &ACoords, QString &ALatitude, QString &ALongitude);
	static QString getLatString(qreal ALatitude);
	static QString getLonString(qreal ALongitude);
	static QString getLatString(const QString &ALatitude);
	static QString getLonString(const QString &ALongitude);

public slots:
	void showWindow();
	void hideWindow();
	void showMinimizedWindow();

protected slots:
	void onSetNewCenter();
	void onSourceSelected(int AIndex);
	void onTypeSelected(bool ASelected);
	void onMppChanged(double mpp);
	void onSceneRectChanged(QRectF rect);

	void onStepLeft(int delta = DELTA_JST);
	void onStepRight(int delta = DELTA_JST);
	void onStepUp(int delta = DELTA_JST);
	void onStepDown(int delta = DELTA_JST);

signals:
	// IMainCentralPage
	void centralPageShow(bool AMinimized);
	void centralPageChanged();
	void centralPageDestroyed();

private:
	Ui::MapForm *ui;
	Map			*FMap;
	MapScene	*FMapScene;
	IMapSource	*FMapSource;
	QGraphicsView *FGraphicsView;
	QList<int>	FTypes;
	int			FOldType;
	bool		FHideEventEnabled;

// Map scene display
	QGraphicsLineItem * FLineX;
	QGraphicsLineItem * FLine1X;
	QGraphicsLineItem * FLine2X;
	QGraphicsLineItem * FLineX1;
	QGraphicsLineItem * FLineX2;

	QGraphicsLineItem * FLineY;
	QGraphicsLineItem * FLine1Y;
	QGraphicsLineItem * FLineY1;
	QGraphicsLineItem * FLine2Y;
	QGraphicsLineItem * FLineY2;

	QPalette            FControlPalette;
	QPalette            FBoxPalette;
};

#endif // MAPFORM_H

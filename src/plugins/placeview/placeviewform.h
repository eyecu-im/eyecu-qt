#ifndef PLACEVIEWFORM_H
#define PLACEVIEWFORM_H

#include <QWidget>
#include <QUuid>
#include <interfaces/imap.h>
#include <interfaces/iplaceview.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipoi.h>
#include "placeview.h"

namespace Ui {
class PlaceViewForm;
}

class PlaceViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit PlaceViewForm(QHash<QUuid, IPlaceViewProvider *> AProviders,IMap *AMap,PlaceView *APlaceView,QWidget *parent = 0);
    ~PlaceViewForm();

    void formDialogActivate(double ALat, double ALng);
    //get poi
    QHash<QString, GeolocElement > &getPois(void)        {return FPoiMassive;}

signals:
    void poisReady(void);
    void poisDelete();

protected:
    void hideEvent(QHideEvent *AHideEvent);
    void init();
    void selectProvider(QUuid AProviderUuid);
    void setTypeGooglePoi();
    void parseResult(QByteArray ASearchResult,QString AId);
	QString getCoordString(double ALat, double ALon) const;
    void superviseHttpRequest();
    void blocking(bool status, QString AError);

protected slots:
    void onProviderSelected();
    void onGetView();
    void repeatHttpRequest();
//    void onSaveToDisk();
    void onClearToMap();
    void onMoveToPosition();
    void onCurrentIndexChanged(int AType);
	void onCurrentRadiusChanged(double ARadius);
    void onBoxRankby(int APos);
    void onBoxWay(int APos);

    void onAboutReceived(const QByteArray &AResult,const QString &AId, bool AState);
    void onPhotoPlaceReceived(const QByteArray &AResult,const QString &AId, bool AState);
    void onImageReceived(const QByteArray &AResult,const QString &AId, bool AState);

    void onOptionsOpened();
    void onOptionsChanged(const OptionsNode &ANode);

private:
    Ui::PlaceViewForm *ui;
    QNetworkAccessManager *FNetworkAccessManager;
    HttpRequester       FHttpRequester;
    QUuid               FCurrentProvider;
    QActionGroup        *FProvidersGroup;
    IMap                *FMap;
    PlaceView           *FPlaceView;
    QHash<QUuid, IPlaceViewProvider *>  FProviders;
    int         FNumberHttpReuest;
    QHash<QString, GeolocElement >  FPoiMassive;
    QStringList FGoogleIcons;

    qulonglong  FCurrentID;
    QUuid       FLastProvider;
    bool        FNewQuery;
    double      FLat,FLng;

    QStringList FListTypes;
    QHash<QString,QString> FHashPoiTypes;
    QStringList FListWayToSearch;
    QStringList FListRankby;
    QString     FNextPage;

};

#endif // PLACEVIEWFORM_H

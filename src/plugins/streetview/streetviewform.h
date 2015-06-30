#ifndef STREETVIEWFORM_H
#define STREETVIEWFORM_H

#include <QWidget>
#include <interfaces/imap.h>
#include <interfaces/istreetview.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iconnectionmanager.h>
#include "streetview.h"

namespace Ui {
class StreetViewForm;
}

class StreetViewForm : public QWidget
{
    Q_OBJECT

public:
    StreetViewForm(QHash<QUuid, IStreetViewProvider *> AProviders, IMap *AMap, StreetView *AStreetView, /*IConnectionManager *AConnectionManager,*/QWidget *parent = 0);
    ~StreetViewForm();

    void setLocation(double ALatitude, double ALongitude);
    void setLocation(const MercatorCoordinates &ALocation);
    void setAzimuth(int AAzimuth);
    void setElevation(int AElevation);
    void startView();

protected:
    void init();
    void selectProvider(QUuid AProviderUuid);
    void hideEvent(QHideEvent *AHideEvent);

protected slots:
    void onProviderSelected();
    void onImageReceived(const QByteArray &AResult, const QUrl &AUrl);

    void onFovValueChange(int AValue);
    void onSizeSelected(int ASize);

    void onDirectionChange(qreal ADirection);
    void onDirectionTrack(qreal ADirection);
    void onAzimuthEditFinished();

    void onElevationChange(qreal AElevation);
    void onElevationTrack(qreal AElevation);
    void onElevationEditFinished();

    void onCopyClicked();
    void onSaveClicked();
    void onViewClicked();
//    void onSettings();
    void onMoveToPosition();
    void onOptionsOpened();
    void onOptionsChanged(const OptionsNode &ANode);

private:
    Ui::StreetViewForm  *ui;
    QNetworkAccessManager *FNetworkAccessManager;
    HttpRequester       FHttpRequester;
    QUuid               FCurrentProvider;
    QActionGroup        *FProvidersGroup;
    IMap                *FMap;
    StreetView          *FStreetView;
    //IConnectionManager  *FConnectionManager;
//    IconStorage *FIconStorage;
    QHash<QUuid, IStreetViewProvider *>  FProviders;
    qulonglong  FCurrentID;
    QUuid       FLastProvider;
    QByteArray  FImageData;
    QUrl        FUrl;

    void draw();
};

#endif // STREETVIEWFORM_H

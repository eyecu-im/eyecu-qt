#ifndef MAPSEARCHDIALOG_H
#define MAPSEARCHDIALOG_H

#include <QDialog>
#include <QUuid>
#include <QTreeWidgetItem>
#include <interfaces/imap.h>
#include <interfaces/ipoi.h>
#include <interfaces/imapsearch.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iconnectionmanager.h>

namespace Ui {
class MapSearchDialog;
}

class MapSearchDialog : public QDialog
{
    Q_OBJECT
    
public:
    MapSearchDialog(QHash<QUuid, IMapSearchProvider *> AProviders, IMap *AMap, IPoi *APoi, IOptionsManager *AOptionsManager, IConnectionManager *AConnectionManager, QWidget *AParent = 0);
    ~MapSearchDialog();
    void selectProvider(QUuid AProviderUuid);
    QString selectedId() const;
    void showPois(bool AShow);
    void setPoiColor(const QColor &AColor);        

public slots:
    void show(bool AAutoShow=false);
    int  exec(bool AAutoShow=false);

protected:
    void updateControls(const IMapSearchProvider *ANewProvider);
    void calculateSearchRange(double &AEast, double &ASouth, double &AWest, double &ANorth);

protected slots:
    void onProviderSelected();
    void onSearchClicked();
    void onPoiModified(const QString &AId, int AType);
    void onCustomContextMenuRequested(const QPoint &APos);
    void onItemActivated(QTreeWidgetItem *ASelectedIitem, int AIndex);
    void onShortcutActivated(const QString &AId, QWidget *AWidget);
    void onClearListClicked();
    void onShowPois(bool AShow);
    void onLimitSearchRange(bool AShow);
    void onReceivedPoi(const GeolocElement &APoi);
    void onSearchFinished(bool AMoreResultsAvailable);
    void onSettings();
    void onDefaultToggled(bool AState);
    void onResultsPerPageChanged(int ANewValue);
    void onMapSettingsChanged();

// public slots:
    void onOptionsOpened();
    void onOptionsChanged(const OptionsNode &ANode);

private:
    Ui::MapSearchDialog                 *ui;
    QNetworkAccessManager               *FNetworkAccessManager;
    HttpRequester                       FHttpRequester;
    QUuid                               FCurrentProvider;    
    QActionGroup                        *FProvidersGroup;
    IMap                                *FMap;
    IPoi                                *FPoi;
    IOptionsManager                     *FOptionsManager;
    IConnectionManager                  *FConnectionManager;
    QHash<QUuid, IMapSearchProvider *>  FProviders;
    qulonglong                          FCurrentID;
    bool                                FAutoShow;
    bool                                FPoiFound;
    int                                 FLimitMin;
    int                                 FLimitMax;
    int                                 FLimitDefault;

    bool                                FMoreResultsAvailable;
    int                                 FLastResultsPerPage;
    bool                                FLastLimitRange;
    QString                             FLastSearchString;
    double                              FLastEast;
    double                              FLastWest;
    double                              FLastSouth;
    double                              FLastNorth;
    QUuid                               FLastProvider;
};

#endif // MAPSEARCHDIALOG_H

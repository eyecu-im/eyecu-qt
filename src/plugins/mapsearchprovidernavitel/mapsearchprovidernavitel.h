#ifndef MAPSEARCHPROVIDERNAVITEL_H
#define MAPSEARCHPROVIDERNAVITEL_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imapsearch.h>
#include <interfaces/ipoi.h>

#include <utils/options.h>

#define MAPSEARCHPROVIDERNAVITEL_UUID "{8dc0b3e2-dc6a-368b-d427-1e32baf825d0}"

class MapSearchProviderNavitel : public QObject,
                                public IPlugin,
                                public IMapSearchProvider
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IMapSearchProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSearchProviderNavitel")
#endif
public:
    enum FieldType {
        Unknown,
        Region,
        SubRegion,
        Locality,
        Street
    };


    MapSearchProviderNavitel(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid   pluginUuid() const { return MAPSEARCHPROVIDERNAVITEL_UUID; }
    virtual void    pluginInfo(IPluginInfo *APluginInfo);
    virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool    initObjects();
    virtual bool    initSettings();
    virtual bool    startPlugin(){return true;}

    //IMapSearchProvider
    virtual void setHttpRequester(HttpRequester *AHttpRequester) {FHttpRequester=AHttpRequester;}
    virtual bool startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore);
    virtual QString sourceName() const;
    virtual QIcon   sourceIcon() const;
    virtual int     features() const {return FeatureNone;}
    virtual void    getPageValues(int *AMin, int *AMax, int *ADefault) const;

protected:
    QUrl searchRequest(const QString &ASearchString, qreal ALongitude, qreal ALatitude, int AZoom) const;
    QUrl infoRequest(int ALocationId) const;
    void parseSearchResult(QByteArray ASearchResult);
    void parseInfoResult(QByteArray ASearchResult, qulonglong AId);
//TODO: Maybe get rid of it
    void fullNavitelTypes();
    bool loadFieldTypes();

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
    void receivedPoi(const GeolocElement &APoi);
    void searchFinished(bool AMoreResultsAvailable);

private:
    HttpRequester *FHttpRequester;
    QMap<qulonglong, GeolocElement> FRequestedPois;
    QMap<int, QString> FNavitelTypes;
    QHash<FieldType, QString> FFieldTypes;
};

#endif // MAPSEARCHPROVIDERNAVITEL_H

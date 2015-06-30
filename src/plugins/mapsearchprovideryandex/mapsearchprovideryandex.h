#ifndef MAPSEARCHPROVIDERYANDEX_H
#define MAPSEARCHPROVIDERYANDEX_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imapsearch.h>
#include <interfaces/ipoi.h>

#include <utils/options.h>

#define MAPSEARCHPROVIDERYANDEX_UUID "{d6bc503d-27b4-dc31-9b3c-d4a81062b714}"

class MapSearchProviderYandex : public QObject,
                                public IPlugin,
                                public IMapSearchProvider
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IMapSearchProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSearchProviderYandex")
#endif
public:
    MapSearchProviderYandex(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid   pluginUuid() const { return MAPSEARCHPROVIDERYANDEX_UUID; }
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
    virtual int     features() const {return FeatureLimitRange|FeatureMaxResults|FeatureNextPage;}
    virtual void    getPageValues(int *AMin, int *AMax, int *ADefault) const;

protected:
    QUrl searchRequest(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, bool ALimitRange, int AMaxResults, bool AMore) const;
    void parseResult(QByteArray ASearchResult);

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
    void receivedPoi(const GeolocElement &APoi);
    void searchFinished(bool AMoreResultsAvailable);

private:
    HttpRequester   *FHttpRequester;
    mutable int     FLastResult;
};

#endif // MAPSEARCHPROVIDERYANDEX_H

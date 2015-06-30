#ifndef MAPSEARCHPROVIDERGOOGLE_H
#define MAPSEARCHPROVIDERGOOGLE_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imapsearch.h>
#include <interfaces/ipoi.h>

#include <utils/options.h>

#define MAPSEARCHPROVIDERGOOGLE_UUID "{8c42dfa0-ba37-218c-b6d0-824fcbaf22fe}"

class MapSearchProviderGoogle : public QObject,
                                public IPlugin,
                                public IMapSearchProvider
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IMapSearchProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSearchProviderGoogle")
#endif
public:
    MapSearchProviderGoogle(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid   pluginUuid() const { return MAPSEARCHPROVIDERGOOGLE_UUID; }
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
    QUrl searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast);
    void parseResult(QByteArray ASearchResult);

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
    void receivedPoi(const GeolocElement &APoi);
    void searchFinished(bool AMoreResultsAvailable);

private:
    HttpRequester *FHttpRequester;
};

#endif // MAPSEARCHPROVIDERGOOGLE_H

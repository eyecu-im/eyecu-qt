#ifndef IMAPSEARCH_H
#define IMAPSEARCH_H

#include <QObject>
#include <QNetworkAccessManager>
#include <interfaces/ipoi.h>
#include <HttpRequest>

#define MAPSEARCH_UUID "{b9fc623b-92fa-bca1-6d5e-fac52301ba21}"

class IMapSearchProvider
{
public:
    enum Feature {
        FeatureNone,
        FeatureLimitRange,
        FeatureLimitRangeAlways,
        FeatureMaxResults   = 0x04,
        FeatureNextPage     = 0x08,
        FeatureMoreResults  = 0x10
    };

    virtual QObject *instance() = 0;
    virtual bool    startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore) = 0;
    virtual void    setHttpRequester(HttpRequester *AHttpRequester) = 0;
    virtual QString sourceName() const = 0;
    virtual QIcon   sourceIcon() const = 0;
    virtual int     features() const = 0;
    virtual void    getPageValues(int *AMin, int *AMax, int *ADefault) const = 0;

protected:
    virtual void receivedPoi(const GeolocElement &APoi) = 0;
    virtual void searchFinished(bool AMoreResultsAvailable) = 0;
};

Q_DECLARE_INTERFACE(IMapSearchProvider, "eyeCU.Plugin.IMapSearchProvider/1.0")

#endif	//IMAPSEARCH_H

#ifndef IWEATHER_H
#define IWEATHER_H

#include <QObject>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QtUtil/httprequest.h>

#define WEATHER_UUID "{7de9475a-9e59-4cc7-98d6-97845169b40d}"

class IWeatherProvider {

public:
    virtual QObject *instance() =0;
    virtual void    setHttpRequester(HttpRequester *AHttpRequester) = 0;
    virtual QString sourceName() const = 0;
    virtual QIcon   sourceIcon() const = 0;
protected:
    virtual void imageReceived(const QByteArray &AResult, const QUrl &AImageUrl) = 0;
};

Q_DECLARE_INTERFACE(IWeatherProvider , "eyeCU.Plugin.IWeatherProvider /1.0")

#endif // IWEATHER_H

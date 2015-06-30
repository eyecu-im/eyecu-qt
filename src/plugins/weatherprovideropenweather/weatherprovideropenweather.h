#ifndef WEATHERPROVIDEROPENWEATHER_H  
#define WEATHERPROVIDEROPENWEATHER_H

#include <QObject>
#include <interfaces/iweather.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>
#include <utils/options.h>

#define WEATHERPROVIDEROPENWEATHER_UUID "{c1a584f9-0aba-4d8a-8063-79184aee224e}"

class WeatherProviderOpenweather: public QObject,
                                public IPlugin,
                                public IWeatherProvider
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IWeatherProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IWeatherProviderOpenweather")
#endif
public:
    explicit WeatherProviderOpenweather(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return WEATHERPROVIDEROPENWEATHER_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IWeatherProvider
    //virtual bool getStreetView(QSize size,double ALat, double ALng, int AHeading,int AFov,int APitch);
    virtual void setHttpRequester(HttpRequester *AHttpRequester) {FHttpRequester=AHttpRequester;}
    virtual QString sourceName() const;
    virtual QIcon   sourceIcon() const;

protected:
    QUrl formUrl(QSize size, qreal ALat, qreal ALng, int AHeading, int AFov, int APitch);

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
    void imageReceived(const QByteArray &AResult, const QUrl &AImageUrl);

private:
    HttpRequester *FHttpRequester;

};

#endif // WEATHERPROVIDEROPENWEATHER_H

#ifndef STREETVIEWPROVIDERGOOGLE_H
#define STREETVIEWPROVIDERGOOGLE_H

#include <QObject>
#include <interfaces/istreetview.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imap.h>

#include <utils/options.h>

#define STREETVIEWPROVIDERGOOGLE_UUID "{0de9f755-31c3-4c8c-90c6-b599a29a04ed}"

class StreetViewProviderGoogle: public QObject,
                                public IPlugin,
                                public IStreetViewProvider
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IStreetViewProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IStreetViewProviderGoogle")
#endif
public:
    explicit StreetViewProviderGoogle(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return STREETVIEWPROVIDERGOOGLE_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IStreetViewProvider
    virtual bool getStreetView(QSize size,double ALat, double ALng, int AHeading,int AFov,int APitch);
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

#endif // STREETVIEWPROVIDERGOOGLE_H

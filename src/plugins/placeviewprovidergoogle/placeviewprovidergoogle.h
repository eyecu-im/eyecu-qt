#ifndef PLACEVIEWPROVIDERGOOGLE_H
#define PLACEVIEWPROVIDERGOOGLE_H

#include <QObject>
#include <interfaces/iplaceview.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imap.h>

#define PLACEVIEWPROVIDERGOOGLE_UUID "{515bb127-0d6a-49a3-9ca5-2b3fc486cfae}"
#define MOT_MAPPOINT         610

class PlaceViewProviderGoogle : public QObject,
                                public IPlugin,
								public IPlaceViewProvider
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IPlaceViewProvider)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.PlaceViewProviderGoogle")
#endif
public:
    explicit PlaceViewProviderGoogle(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return PLACEVIEWPROVIDERGOOGLE_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects() {return true;}
    virtual bool initSettings();
    virtual bool startPlugin() {return true;}

    //IPlaceViewProvider
    virtual bool getAboutPlace(double ALat, double ALng,long ARadius, QString ATypes,QString ARankby, QString AKeyword,QString AWayToSearch,QString APagetoken,QString ALanguage, QString AId);
    virtual bool getPhotoPlace(QSize imSize,QString APhotoRefer,QString AId);
    virtual bool getImage(QString AIconRefer,QString AId);
    virtual void setHttpRequester(HttpRequester *AHttpRequester) {FHttpRequester=AHttpRequester;}
    virtual QString sourceName() const{ return tr("Google");}
    virtual QIcon   sourceIcon() const;

protected slots:
    void onAboutReceived(const QByteArray &AResult,const QString &AId);
    void onGetPhotoPlace(const QByteArray &AResult,const QString &AId);
    void onGetImage(const QByteArray &AResult,const QString &AId);

signals:
    void photoPlaceReceived(const QByteArray &AResult,const QString &AId,bool AMoreResultsAvailable);
    void aboutPlaceReceived(const QByteArray &AResult,const QString &AId,bool AMoreResultsAvailable);
    void imageReceived(const QByteArray &AResult,const QString &AId,bool AMoreResultsAvailable);

private:
    HttpRequester *FHttpRequester;
};

#endif // PLACEVIEWPROVIDERGOOGLE_H

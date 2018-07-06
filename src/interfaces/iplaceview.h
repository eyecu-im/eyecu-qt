#ifndef IPLACEVIEW_H
#define IPLACEVIEW_H

#include <QObject>
#include <QIcon>
#include <QNetworkAccessManager>
#include <HttpRequest>

#define PLACEVIEW_UUID "{ca41e58a-c87e-4f25-9998-0d58fbd51072}"

class IPlaceViewProvider {

public:
    virtual QObject *instance() =0;
    virtual bool    getAboutPlace(double ALat, double ALng,long ARadius, QString ATypes,QString ARankby, QString AKeyword,QString AWayToSearch,QString APagetoken,QString ALanguage, QString AId)=0;
    virtual bool    getPhotoPlace(QSize imSize,QString APhotoRefer,QString AId)=0;
    virtual bool    getImage(QString AIconRefer,QString AId)=0;
    virtual void    setHttpRequester(HttpRequester *AHttpRequester) = 0;
    virtual QString sourceName()  const = 0;
    virtual QIcon   sourceIcon()  const = 0;
protected:
    virtual void photoPlaceReceived(const QByteArray &AResult,const QString &AId,bool AMoreResultsAvailable) = 0;
    virtual void aboutPlaceReceived(const QByteArray &AResult,const QString &AId,bool AMoreResultsAvailable)=0;
    virtual void imageReceived(const QByteArray &AResult,const QString &AId,bool AMoreResultsAvailable)=0;
};

Q_DECLARE_INTERFACE(IPlaceViewProvider, "eyeCU.Plugin.IPlaceViewProvider /1.0")

#endif // IPLACEVIEW_H

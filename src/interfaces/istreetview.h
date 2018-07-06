#ifndef ISTREETVIEW_H
#define ISTREETVIEW_H

#include <QObject>
#include <QIcon>
#include <QNetworkAccessManager>
#include <HttpRequest>

#define STREETVIEW_UUID "{3322a1aa-3fa4-40b8-ab68-af4da153a7e7}"
                     
class IStreetViewProvider {

public:
    virtual QObject *instance() =0;
    virtual bool    getStreetView(QSize size,double ALat, double ALng, int AHeading,int AFov,int APitch)=0;
    virtual void    setHttpRequester(HttpRequester *AHttpRequester) = 0;
    virtual QString sourceName() const = 0;
    virtual QIcon   sourceIcon() const = 0;
protected:
    virtual void imageReceived(const QByteArray &AResult, const QUrl &AImageUrl) = 0;
};

Q_DECLARE_INTERFACE(IStreetViewProvider , "eyeCU.Plugin.IStreetViewProvider /1.0")

#endif	//ISTREETVIEW_H

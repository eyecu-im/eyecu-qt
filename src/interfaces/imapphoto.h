#ifndef IMAPPHOTO_H
#define IMAPPHOTO_H

#include <QObject>
#include <QNetworkAccessManager>
#include <utils/httprequest.h>

#define MAPPHOTO_UUID "{6d013d33-d987-43f7-b7b7-933c0a596aeb}"
                     
class IMapPhoto {

public:
    virtual QObject *instance() =0;
    virtual bool getView(const QString &ASearchString) = 0;
    virtual void setHttpRequester(HttpRequester *AHttpRequester) = 0;

};

Q_DECLARE_INTERFACE(IMapPhoto , "RWS.Plugin.IMapPhoto /1.0")

#endif	//IMAPPHOTO_H

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>

#include "streetviewprovidergoogle.h"

StreetViewProviderGoogle::StreetViewProviderGoogle(QObject *parent) :
    QObject(parent)
{
}
void StreetViewProviderGoogle::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Street View Provider Google");
    APluginInfo->description = tr("Allows to use Google as a Street View provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(STREETVIEW_UUID);
}

bool StreetViewProviderGoogle::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)

    AInitOrder = 200;
    return true;
}

bool StreetViewProviderGoogle::initObjects(){ return true;}

bool StreetViewProviderGoogle::initSettings(){ return true;}

QString StreetViewProviderGoogle::sourceName() const{ return tr("Google");}

QIcon StreetViewProviderGoogle::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_GOOGLE);
}

bool StreetViewProviderGoogle::getStreetView(QSize size, double ALat, double ALng, int AHeading, int AFov, int APitch)
{
    QUrl request=formUrl(size,ALat,ALng,AHeading,AFov,APitch);
    return FHttpRequester->request(request, "request", this, SLOT(onResultReceived(QByteArray,QString)));
}

QUrl StreetViewProviderGoogle::formUrl(QSize size,qreal ALat, qreal ALng, int AHeading,int AFov,int APitch)
{
    if(AHeading<0 || AHeading >360) AHeading=0;
    if(AFov<90) AFov=90;
        else if(AFov>120) AFov=120;
    if(APitch<-90) APitch=-90;
        else if( APitch >90)APitch=90;

    QString request=QString("http://maps.googleapis.com/maps/api/streetview?size=%1x%2&location=%3,%4&heading=%5&fov=%6&pitch=%7&sensor=false")
            .arg(QString().setNum(size.width())).arg(QString().setNum(size.height()))
            .arg(QString().setNum(ALat,'g',10)).arg(QString().setNum(ALng,'g',10))
            .arg(QString().setNum(AHeading)).arg(QString().setNum(AFov)).arg(QString().setNum(APitch));
    QUrl url(request);
    return url;
}

void StreetViewProviderGoogle::onResultReceived(const QByteArray &AResult, const QString &AId)
{
	Q_UNUSED(AId)

    HttpRequest *request = qobject_cast<HttpRequest *>(sender());
    emit imageReceived(AResult, request->url());
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_streetviewprovidergoogle, StreetViewProviderGoogle)
#endif

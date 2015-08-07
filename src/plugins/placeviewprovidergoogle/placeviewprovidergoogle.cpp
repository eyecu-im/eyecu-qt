#include <QDebug>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <utils/iconstorage.h>

#include "placeviewprovidergoogle.h"

//! organization:   RWS, site:  http://rwsoftware.ru
#define API_KEY  "AIzaSyAsEtCNC8-Gp2uYbqHBdZRt9EKipUpUf2c"

PlaceViewProviderGoogle::PlaceViewProviderGoogle(QObject *parent): QObject(parent)
{}

void PlaceViewProviderGoogle::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Place view provider Google");
	APluginInfo->description = tr("Allows to view photos of places from Google");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(PLACEVIEW_UUID);
}

bool PlaceViewProviderGoogle::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)
	Q_UNUSED(AInitOrder)
    return true;
}

bool PlaceViewProviderGoogle::initSettings()
{
    return true;
}

QIcon PlaceViewProviderGoogle::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_GOOGLE);
}

// * https://maps.googleapis.com/maps/api/place/nearbysearch/xml?
//      rankby =prominence,distance. need param [keyword, name or types].
bool PlaceViewProviderGoogle::getAboutPlace(double ALat, double ALng,long ARadius, QString ATypes,QString ARankby, QString AKeyword,QString AWayToSearch,QString APagetoken,QString ALanguage, QString AId)
{
 //   QString AGoogleKey=Options::node(OPV_PLACE_GOOGLE_KEY).value().toString();
    if(ARadius<0 || ARadius>50000)
        ARadius=1000;

    QString URL=QString("https://maps.googleapis.com/maps/api/place/%1/xml?location=%2,%3")
        .arg(AWayToSearch).arg(QString().setNum(ALat,'g',10)).arg(QString().setNum(ALng,'g',10));
    if(ARadius>0)
        URL.append(QString("&radius=%1").arg(QString().setNum(ARadius)));
    if(ATypes!=NULL)
        URL.append(QString("&types=%1").arg(ATypes));
    if(ARankby!=NULL)
        URL.append(QString("&rankby=%1").arg(ARankby));
    if(AKeyword!=NULL)
        URL.append(QString("&keyword=%1").arg(AKeyword));
    if(APagetoken!=NULL)
        URL.append(QString("&pagetoken=%1").arg(APagetoken));
    if(ALanguage!=NULL)
        URL.append(QString("&language=%1").arg(ALanguage));
	URL.append(QString("&sensor=false&key=%1").arg(API_KEY));
    QUrl request(URL);
//qDebug()<<"PlaceViewProviderGoogle::getAboutPlace/request=\n"<<request;
    return FHttpRequester->request(request, AId, this, SLOT(onAboutReceived(QByteArray,QString)));
}

bool PlaceViewProviderGoogle::getPhotoPlace(QSize imSize, QString APhotoRefer, QString AId)
{
    QString URL=QString("https://maps.googleapis.com/maps/api/place/photo?maxwidth=%1&photoreference=%2&sensor=false&key=%3")
            .arg(QString().setNum(imSize.width()))
            .arg(APhotoRefer)
			.arg(API_KEY);
    QUrl request(URL);
    return FHttpRequester->request(request, AId, this, SLOT(onGetPhotoPlace(QByteArray,QString)));
}

bool PlaceViewProviderGoogle::getImage(QString AIconRefer, QString AId)
{
    QUrl request(QString().append(AIconRefer));
    return FHttpRequester->request(request,AId,this, SLOT(onGetImage(QByteArray,QString)));
}

void PlaceViewProviderGoogle::onAboutReceived(const QByteArray &AResult, const QString &AId)
{
    emit aboutPlaceReceived(AResult,AId,true);
}

void PlaceViewProviderGoogle::onGetPhotoPlace(const QByteArray &AResult, const QString &AId)
{
    emit photoPlaceReceived(AResult,AId,true);
}

void PlaceViewProviderGoogle::onGetImage(const QByteArray &AResult, const QString &AId)
{
    emit imageReceived(AResult,AId,true);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_placeviewprovidergoogle, PlaceViewProviderGoogle)
#endif

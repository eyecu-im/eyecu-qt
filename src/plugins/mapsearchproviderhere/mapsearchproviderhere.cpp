#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>
#include "mapsearchproviderhere.h"

MapSearchProviderHere::MapSearchProviderHere(QObject *parent):
    QObject(parent),
    FHttpRequester(NULL),
    FPoi(NULL),
    FNextPageInformation(0)
{}

void MapSearchProviderHere::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search Provider Here");
    APluginInfo->description = tr("Allows to use Nokia's Here service as a map search provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(POI_UUID);
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool MapSearchProviderHere::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin=APluginManager->pluginInterface("IPoi").value(0, NULL);
    if (plugin)
        FPoi=qobject_cast<IPoi *>(plugin->instance());
    else
        return false;
    AInitOrder = 200;
    return true;
}

bool MapSearchProviderHere::initObjects()
{
    fillCountryCodes();
    return true;
}

bool MapSearchProviderHere::initSettings()
{
    return true;
}

bool MapSearchProviderHere::startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore)
{
    Q_UNUSED(AZoom)
    QUrl request = searchRequest(ASearchString, ALatSouth, ALngWest, ALatNorth, ALngEast, ALimitRange, AMaxResults, AMore);
    return FHttpRequester->request(request, "request", this, SLOT(onResultReceived(QByteArray,QString)));
}

QString MapSearchProviderHere::sourceName() const
{
    return tr("Here");
}

QIcon MapSearchProviderHere::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_HERE);
}

void MapSearchProviderHere::getPageValues(int *AMin, int *AMax, int *ADefault) const
{
    if (AMin)
        *AMin = 1;
    if (AMax)
        *AMax = 9999;
    if (ADefault)
        *ADefault = 10;
}

#define APP_ID "1swEGDl77q7amGOicUvQ"
#define APP_CODE "X6klY4o6aZFJfFLyUMWOIQ"
#define GEN 7
QUrl MapSearchProviderHere::searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, bool ALimitRange, int AMaxResults, bool AMore)
{   
// http://geocoder.api.here.com/6.2/geocode.xml?app_id=xqf6Zmqj7im2nZgH3uew&app_code=z1tEmMuNMRdJgNIkW_gBsw&gen=4&searchtext=%D0%9C%D0%B0%D0%B3%D0%BD%D0%B8%D1%82%D0%BE%D0%B3%D0%BE%D1%80%D1%81%D0%BA,+%D0%9D%D0%B0%D0%B1%D0%B5%D1%80%D0%B5%D0%B6%D0%BD%D0%B0%D1%8F,+16&language=ru-RU
    QString locale=QLocale().name();
    locale[2]='-';
    QString url("http://geocoder.api.here.com/6.2/geocode.xml?app_id=%1&app_code=%2&gen=%3&searchtext=%4&language=%5");
    url.append(ALimitRange?"&mapview=%6,%7;%8,%9":"&bbox=%6,%7;%8,%9");
    if (AMaxResults != -1)
        url.append("&maxresults=").append(QString::number(AMaxResults));
    if (AMore)
        url.append("&pageinformation=").append(QString::number(FNextPageInformation));
    return QUrl(url.arg(APP_ID).arg(APP_CODE).arg(GEN)
                   .arg(ASearchString)
                   .arg(locale)
                   .arg(ALatNorth)
                   .arg(ALngWest)
                   .arg(ALatSouth)
                   .arg(ALngEast));
}

void MapSearchProviderHere::parseResult(QByteArray ASearchResult)
{
    QDomDocument doc;
    doc.setContent(ASearchResult);
    QDomElement response = doc.documentElement().firstChildElement("Response");
    if (!response.isNull())
    {
        QDomElement metaInfo = response.firstChildElement("MetaInfo");
        if (!metaInfo.isNull())
        {
            QDomElement nextPageInformation = metaInfo.firstChildElement("NextPageInformation");
            FNextPageInformation = nextPageInformation.isNull()?0:nextPageInformation.text().toInt();
        }
        QDomElement view = response.firstChildElement("View");
        if (!view.isNull())
            for(QDomElement result=view.firstChildElement("Result");
                !result.isNull();
                result=result.nextSiblingElement("Result"))
            {
                GeolocElement poi;

                QDomElement matchLevel = result.firstChildElement("MatchLevel");
                if (!matchLevel.isNull())
                {
                    QString value = matchLevel.text();
                    if (value=="country")
						poi.setType("place:country");
                    else if (value=="state")
						poi.setType("place:state");
                    else if (value=="county")
						poi.setType("place:county");
                    else if (value=="city")
						poi.setType("place:city");
                    else if (value=="district")
						poi.setType("place:district");
                    else if (value=="street")
						poi.setType("highway:secondary");
                    else if (value=="houseNumber")
						poi.setType("misc:housenumber");
                    else if (value=="postalCode")
						poi.setType("geographic:building");
                    else if (value=="building")
						poi.setType("geographic:building");
                }

                QDomElement location = result.firstChildElement("Location");
                if (!location.isNull())
                {
                    QDomElement displayPosition = location.firstChildElement("DisplayPosition");
                    if (!displayPosition.isNull())
                    {
                        QDomElement latitude = displayPosition.firstChildElement("Latitude");
                        if (!latitude.isNull())
							poi.setLat(latitude.text().toDouble());
                        QDomElement longitude = displayPosition.firstChildElement("Longitude");
                        if (!longitude.isNull())
							poi.setLon(longitude.text().toDouble());
                    }

                    QDomElement address = location.firstChildElement("Address");
                    if (!address.isNull())
                    {
                        QDomElement label = address.firstChildElement("Label");
                        if (!label.isNull())
							poi.setDescription(label.text());

                        QDomElement country = address.firstChildElement("Country");
                        if (!country.isNull())
							poi.setCountryCode(FCountryCodes.value(country.text()));

						QString region;
                        QDomElement state = address.firstChildElement("State");
                        if (!state.isNull())
							region = state.text();

                        QDomElement county = address.firstChildElement("County");
                        if (!county.isNull())
                        {							
							if (region.isEmpty())
								region = county.text();
							else
								region.append(", ").append(county.text());
                        }
						poi.setRegion(region);

                        QDomElement city = address.firstChildElement("City");
                        if (!city.isNull())
							poi.setLocality(city.text());

                        QDomElement district = address.firstChildElement("District");
                        if (!district.isNull())
							poi.setArea(district.text());

                        QDomElement street = address.firstChildElement("Street");
                        if (!street.isNull())
							poi.setStreet(street.text());

                        QDomElement houseNumber = address.firstChildElement("HouseNumber");
                        if (!houseNumber.isNull())
							poi.setBuilding(houseNumber.text());

                        QDomElement postalCode = address.firstChildElement("PostalCode");
                        if (!postalCode.isNull())
							poi.setPostalCode(postalCode.text());

                        for(QDomElement additionalData=address.firstChildElement("AdditionalData");
                            !additionalData.isNull();
                            additionalData=additionalData.nextSiblingElement("AdditionalData"))
                        {
                            if (additionalData.attribute("key") == "CountryName")
								poi.setCountry(additionalData.text());
    //                        else if (additionalData.attribute("key") == "StateName")
    //                            qDebug() << "State name:" << additionalData.text();
                        }

                        QString text;
						if (poi.hasProperty(GeolocElement::Street))
                        {							
							text=poi.street();
							if (poi.hasProperty(GeolocElement::Building))
								text.append(", ").append(poi.building());
                        }
						else if (poi.hasProperty(GeolocElement::Area))
							text=poi.area();
						else if (poi.hasProperty(GeolocElement::Locality))
							text=poi.locality();
						else if (poi.hasProperty(GeolocElement::Region))
							text=poi.region();
                        else if (!state.isNull())
                            text=state.text();
						else if (poi.hasProperty(GeolocElement::Country))
							text=poi.country();
						else if (poi.hasProperty(GeolocElement::Description))
							text=poi.description();
                        if (!text.isNull())
							poi.setText(text);
                    }
                    emit receivedPoi(poi);
                }
            }
    }
    emit searchFinished(FNextPageInformation != 0);  // Signal search finished
}

void MapSearchProviderHere::onResultReceived(const QByteArray &AResult, const QString &AId)
{
    Q_UNUSED(AId)
    if (!AResult.isNull())
        parseResult(AResult);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearchproviderhere, MapSearchProviderHere)
#endif

#include <QLocale>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>
#include "mapsearchprovideryandex.h"

MapSearchProviderYandex::MapSearchProviderYandex(QObject *parent):
    QObject(parent),
    FLastResult(0)
{}

void MapSearchProviderYandex::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search Provider Yandex");
    APluginInfo->description = tr("Allows to use Yandex as a map search provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool MapSearchProviderYandex::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)

    AInitOrder = 200;
    return true;
}

bool MapSearchProviderYandex::initObjects()
{
    return true;
}

bool MapSearchProviderYandex::initSettings()
{
    return true;
}

QString MapSearchProviderYandex::sourceName() const
{
    return tr("Yandex");
}

QIcon MapSearchProviderYandex::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_YANDEX);
}

void MapSearchProviderYandex::getPageValues(int *AMin, int *AMax, int *ADefault) const
{
    if (AMin)
        *AMin = 1;
    if (AMax)
        *AMax = 9999;
    if (ADefault)
        *ADefault = 10;
}

bool MapSearchProviderYandex::startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore)
{
    Q_UNUSED(AZoom)
    QUrl request = searchRequest(ASearchString, ALatSouth, ALngWest, ALatNorth, ALngEast, ALimitRange, AMaxResults, AMore);
    return FHttpRequester->request(request, "request", this, SLOT(onResultReceived(QByteArray,QString)));
}

QUrl MapSearchProviderYandex::searchRequest(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, bool ALimitRange, int AMaxResults, bool AMore) const
{    
// http://geocode-maps.yandex.ru/1.x/?geocode=%D0%9C%D0%B0%D0%B3%D0%BD%D0%B8%D1%82%D0%BE%D0%B3%D0%BE%D1%80%D1%81%D0%BA&ll=58.97,53.4199999938438&spn=0.0639438629150391,0.0340451179686454&format=json&results=15
    QString lang=QLocale().name();
    lang[2]='-';
    qreal longitude = (ALngWest + ALngEast)/2;
    qreal latitude  = (ALatNorth + ALatSouth)/2;
    QString url("http://geocode-maps.yandex.ru/1.x/?geocode=%1&ll=%2,%3&spn=%4,%5&format=xml&lang=%6&rspn=%7");
    if (AMaxResults!=-1)
        url.append("&results=").append(QString::number(AMaxResults));
    if (AMore)
        url.append("&skip=").append(QString::number(FLastResult));
    else
        FLastResult=0;
    return QUrl(url.arg(ASearchString)
                   .arg(longitude)
                   .arg(latitude)
                   .arg(longitude-ALngWest)
                   .arg(latitude-ALatSouth)
                   .arg(lang)
                   .arg(ALimitRange));
}

void MapSearchProviderYandex::parseResult(QByteArray ASearchResult)
{
    QDomDocument doc;
    doc.setContent(ASearchResult);
    QDomElement ymaps = doc.documentElement();
    QDomElement geoObjectCollection = ymaps.firstChildElement("GeoObjectCollection");
    bool more = false;
    if (!geoObjectCollection.isNull())
    {
        QDomElement metaDataProperty = geoObjectCollection.firstChildElement("metaDataProperty");
        if (!metaDataProperty.isNull())
        {
            QDomElement geocoderResponseMetaData = metaDataProperty.firstChildElement("GeocoderResponseMetaData");
            if (!geocoderResponseMetaData.isNull())
            {
                QDomElement results = geocoderResponseMetaData.firstChildElement("results");
                if (!results.isNull())
                {
                    FLastResult+=results.text().toInt();
                    QDomElement found = geocoderResponseMetaData.firstChildElement("found");
                    if (!found.isNull())
                    {
                        int total = found.text().toInt();
                        if (FLastResult<total)
                            more = true;
                    }
                }
            }
        }
        for (QDomElement featureMember = geoObjectCollection.firstChildElement("featureMember"); !featureMember.isNull(); featureMember=featureMember.nextSiblingElement("featureMember"))
        {
            QDomElement geoObject = featureMember.firstChildElement("GeoObject");
            if (!geoObject.isNull())
            {
                GeolocElement poi;
                QDomElement name = geoObject.firstChildElement("name");
                if (!name.isNull())
					poi.setText(name.text());
                QDomElement description = geoObject.firstChildElement("description");
                if (!description.isNull())
					poi.setDescription(description.text());
                QDomElement point = geoObject.firstChildElement("Point");
                if (!point.isNull())
                {
                    QDomElement pos = point.firstChildElement("pos");
                    if (!pos.isNull())
                    {
                        QStringList coordinates = pos.text().split(' ');
                        if (coordinates.size()==2)
                        {
							poi.setLon(coordinates[0].toDouble());
							poi.setLat(coordinates[1].toDouble());
                        }
                    }
                }
                QDomElement metaDataProperty = geoObject.firstChildElement("metaDataProperty");
                if (!metaDataProperty.isNull())
                {
                    QDomElement geocoderMetaData = metaDataProperty.firstChildElement("GeocoderMetaData");
                    if (!geocoderMetaData.isNull())
                    {
                        QDomElement kind = geocoderMetaData.firstChildElement("kind");
                        if (!kind.isNull())
                        {
                            QString type;
                            QString k = kind.text();
                            if (k == "province")
                                type = "place:administrative";
                            if (k == "locality")
                                type = "place:locality";
                            else if (k == "hydro")
                                type = "waterway";
                            else if (k == "street")
                                type = "highway";
                            else if (k == "railway")
                                type = "railway";
                            else if (k == "metro")
                                type = "railway:subway";
                            else if (k == "airport")
                                type = "airway:airport";
                            else if (k == "house")
                            {
								if (poi.hasProperty(GeolocElement::Building))
                                    type = "misc:housenumber";
                                else
                                    type = "place:house";
                            }
                            if (!type.isEmpty())
								poi.setType(type);
                        }
//                        QDomElement precision = geocoderMetaData.firstChildElement("precision");
                        QDomElement text = geocoderMetaData.firstChildElement("text");
                        if (!text.isNull())
							poi.setDescription(text.text()); // Override "description" with more verbose "text" element!
                        QDomElement addressDetails = geocoderMetaData.firstChildElement("AddressDetails");
                        if (!addressDetails.isNull())
                        {
                            QDomElement country = addressDetails.firstChildElement("Country");
                            if (!country.isNull())
                            {
                                QDomElement countryName = country.firstChildElement("CountryName");
                                if (!countryName.isNull())
									poi.setCountry(countryName.text());
                                QDomElement countryNameCode = country.firstChildElement("CountryNameCode");
                                if (!countryNameCode.isNull())
									poi.setCountryCode(countryNameCode.text());
                                QDomElement administrativeArea = country.firstChildElement("AdministrativeArea");
                                if (!administrativeArea.isNull())
                                {
                                    QDomElement administrativeAreaName = administrativeArea.firstChildElement("AdministrativeAreaName");
                                    if (!administrativeAreaName.isNull())
										poi.setRegion(administrativeAreaName .text());
                                    QDomElement subAdministrativeArea = administrativeArea.firstChildElement("SubAdministrativeArea");
                                    if (!subAdministrativeArea.isNull())
                                    {
                                        QDomElement locality = subAdministrativeArea.firstChildElement("Locality");
                                        if (!locality.isNull())
                                        {
                                            QDomElement localityName = locality.firstChildElement("LocalityName");
                                            if (!localityName.isNull())
												poi.setLocality(localityName.text());
                                            QDomElement thoroughfare = locality.firstChildElement("Thoroughfare");
                                            if (!thoroughfare.isNull())
                                            {
                                                QDomElement thoroughfareName = thoroughfare.firstChildElement("ThoroughfareName");
                                                if (!thoroughfare.isNull())
													poi.setStreet(thoroughfareName.text());
                                                QDomElement premise = thoroughfare.firstChildElement("Premise");
                                                if (!premise.isNull())
                                                {
                                                    QDomElement premiseNumber = premise.firstChildElement("PremiseNumber");
                                                    if (!premiseNumber.isNull())
														poi.setBuilding(premiseNumber.text());
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                emit receivedPoi(poi);
            }
        }
    }
    emit searchFinished(more);  // Signal search finished
}

void MapSearchProviderYandex::onResultReceived(const QByteArray &AResult, const QString &AId)
{
    Q_UNUSED(AId)
    if (!AResult.isNull())
        parseResult(AResult);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearchprovideryandex, MapSearchProviderYandex)
#endif

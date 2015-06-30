#include <definitions/resources.h>
#include <definitions/menuicons.h>

#include <utils/iconstorage.h>
#include <utils/logger.h>

#include "mapsearchproviderosm.h"

MapSearchProviderOsm::MapSearchProviderOsm(QObject *parent) : QObject(parent), FPoi(NULL)
{}

void MapSearchProviderOsm::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search Provider OSM");
    APluginInfo->description = tr("Allows to use OSM as a map search provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(POI_UUID);
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool MapSearchProviderOsm::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin=APluginManager->pluginInterface("IPoi").value(0, NULL);
    if (plugin)
        FPoi=qobject_cast<IPoi *>(plugin->instance());
    else
        return false;
    AInitOrder = 200;
    return true;
}

bool MapSearchProviderOsm::initObjects()
{
    return true;
}

bool MapSearchProviderOsm::initSettings()
{
    return true;
}

bool MapSearchProviderOsm::startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore)
{
    Q_UNUSED(AZoom)
	Q_UNUSED(AMore)

    QUrl request = searchRequest(ASearchString, ALatSouth, ALngWest, ALatNorth, ALngEast, ALimitRange, AMaxResults);
    return FHttpRequester->request(request, "request", this, SLOT(onResultReceived(QByteArray,QString)));
}

QString MapSearchProviderOsm::sourceName() const
{
    return tr("OSM");
}

QIcon MapSearchProviderOsm::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_OSM);
}

void MapSearchProviderOsm::getPageValues(int *AMin, int *AMax, int *ADefault) const
{
    if (AMin)
        *AMin = 1;
    if (AMax)
        *AMax = 9999;
    if (ADefault)
        *ADefault = 9999;
}

QUrl MapSearchProviderOsm::searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int ABounded, int ALimit)
{
// http://nominatim.openstreetmap.org/search?q=%D0%9D%D0%B0%D0%B1%D0%B5%D1%80%D0%B5%D0%B6%D0%BD%D0%B0%D1%8F,+16&format=xml&polygon=1&addressdetails=1&accept-language=ru-RU&viewbox=58,54,60,52&bounded=1
    QString url("http://nominatim.openstreetmap.org/search?q=%1&format=xml&addressdetails=1&accept-language=%2&viewbox=%3,%4,%5,%6&bounded=%7");
    if (ALimit != -1)
        url.append("&limit=").append(QString::number(ALimit));
    return QUrl(url.arg(ASearchString)
                   .arg(QLocale().name())
                   .arg(ALngWest)
                   .arg(ALatNorth)
                   .arg(ALngEast)
                   .arg(ALatSouth)
                   .arg(ABounded));
}

void MapSearchProviderOsm::parseResult(QByteArray ASearchResult)
{
    QDomDocument doc;
    doc.setContent(ASearchResult);

    QDomElement searchresults=doc.firstChildElement("searchresults");

    if (!searchresults.isNull())
        for(QDomElement place=searchresults.firstChildElement("place");
            !place.isNull();
            place=place.nextSiblingElement("place"))
        {
            GeolocElement poi;
            QString label;
            bool    noLabel=false;

            if (place.hasAttribute("type"))
            {
                QString type=place.attribute("type");                
                label=place.firstChildElement(type).text();
                if (label.isEmpty())
                    noLabel=true;

                if (type == "tram_stop")
                    type = "tram";
                else if (type == "subway_entrance")
                    type = "subway";

                type=FPoi->getFullType(type, place.attribute("class"));
                if (!type.isNull())
					poi.setType(type);
            }
            else
				LOG_WARNING("No \"Type\" attribute found!");

			poi.setLat(place.attribute("lat").toDouble());
			poi.setLon(place.attribute("lon").toDouble());

            QDomElement country=place.firstChildElement("country");
            if (!country.isNull())
				poi.setCountry(country.text());

            QDomElement country_code=place.firstChildElement("country_code");
            if (!country_code.isNull())
				poi.setCountryCode(country_code.text().toUpper());

            QDomElement postcode=place.firstChildElement("postcode");
            if (!postcode.isNull())
				poi.setPostalCode(postcode.text());

            QDomElement state=place.firstChildElement("state");
            if (!state.isNull())
				poi.setRegion(state.text());

            QDomElement locality=place.firstChildElement("city");
            if (locality.isNull())
            {
                locality=place.firstChildElement("town");
                if (locality.isNull())
                {
                    locality=place.firstChildElement("village");
                    if (locality.isNull())
                    {
                        locality=place.firstChildElement("hamlet");
                        if (locality.isNull())
                        {
                            locality=place.firstChildElement("isolated_dwelling");
                            if (locality.isNull())
                                locality=place.firstChildElement("locality");
                        }
                    }
                }
            }

            if (!locality.isNull())
				poi.setLocality(locality.text());

            QDomElement road=place.firstChildElement("road");
            if (!road.isNull())
				poi.setStreet(road.text());

            QDomElement house_number=place.firstChildElement("house_number");
            if (!house_number.isNull())
				poi.setBuilding(house_number.text());

            if (label.isEmpty())
            {
                if (!road.isNull())
                    label=road.text();
                if (!house_number.isNull())
                {
                    if (!label.isNull())
                        label.append(", ");
                    label.append(house_number.text());
                }
                if (label.isEmpty())
                {
                    if (!locality.isNull())
                        label=locality.text();
                    else if (!state.isNull())
                        label=state.text();
                    else if (!country.isNull())
                        label=country.text();
                }
            }

			poi.setDescription(place.attribute("display_name"));
			poi.setText(label.isEmpty()?place.attribute("display_name"):label);

            emit receivedPoi(poi);
        }
    emit searchFinished(false);  // Signal search finished
}

void MapSearchProviderOsm::onResultReceived(const QByteArray &AResult, const QString &AId)
{
    Q_UNUSED(AId)
    if (!AResult.isNull())
        parseResult(AResult);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearchproviderosm, MapSearchProviderOsm)
#endif

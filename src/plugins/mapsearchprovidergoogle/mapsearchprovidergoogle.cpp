#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>
#include "mapsearchprovidergoogle.h"

MapSearchProviderGoogle::MapSearchProviderGoogle(QObject *parent) : QObject(parent)
{}

void MapSearchProviderGoogle::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search Provider Google");
    APluginInfo->description = tr("Allows to use Google as a map search provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool MapSearchProviderGoogle::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)

    AInitOrder = 200;
    return true;
}

bool MapSearchProviderGoogle::initObjects()
{
    return true;
}

bool MapSearchProviderGoogle::initSettings()
{
    return true;
}

bool MapSearchProviderGoogle::startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore)
{
    Q_UNUSED(AZoom)
	Q_UNUSED(AMore)
	Q_UNUSED(ALimitRange)
	Q_UNUSED(AMaxResults)

    QUrl request = searchRequest(ASearchString, ALatSouth, ALngWest, ALatNorth, ALngEast);
    return FHttpRequester->request(request, "request", this, SLOT(onResultReceived(QByteArray,QString)));
}

QString MapSearchProviderGoogle::sourceName() const
{
    return tr("Google");
}

QIcon MapSearchProviderGoogle::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_GOOGLE);
}

void MapSearchProviderGoogle::getPageValues(int *AMin, int *AMax, int *ADefault) const
{
    if (AMin)
        *AMin = 0;
    if (AMax)
        *AMax = 0;
    if (ADefault)
        *ADefault = 0;
}

QUrl MapSearchProviderGoogle::searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast)
{
    QLocale local;
    QString request("http://maps.googleapis.com/maps/api/geocode/xml?sensor=false&address=%1&language=%2&bounds=%3,%4|%5,%6");
    QUrl url(request.arg(ASearchString).arg(local.name().left(2)).arg(ALatSouth).arg(ALngWest).arg(ALatNorth).arg(ALngEast));
    return url;
}

void MapSearchProviderGoogle::parseResult(QByteArray ASearchResult)
{
    QDomDocument doc;
    doc.setContent(ASearchResult);

    QDomElement geocodeResponse=doc.documentElement();
    QDomElement status=geocodeResponse.firstChildElement("status");
    if (!status.isNull() && status.text() == "OK")
    {
        for (QDomElement result=geocodeResponse.firstChildElement("result");
             !result.isNull();
             result=result.nextSiblingElement("result"))
        {
            GeolocElement poi;
            QDomElement formatted_address=result.firstChildElement("formatted_address");
            if (!formatted_address.isNull())
				poi.setDescription(formatted_address.text());

            for(QDomElement address_component=result.firstChildElement("address_component");
                !address_component.isNull();
                address_component=address_component.nextSiblingElement("address_component"))
            {
                QString name;
                QDomElement name_element=address_component.firstChildElement("long_name");
                if (!name_element.isNull())
                    name=name_element.text();
                for(QDomElement type=address_component.firstChildElement("type");
                    !type.isNull();
                    type=type.nextSiblingElement("type"))
                {
                    QString type_text=type.text();
                    if (type_text=="street_number")
                    {
						poi.setBuilding(name);
                        break;
                    }
                    else if (type_text=="route")
                    {
						poi.setStreet(name);
                        break;
                    }
                    else if (type_text=="locality")
                    {
						poi.setLocality(name);
                        break;
                    }
                    else if (type_text=="administrative_area_level_2")
                    {
// Don't know what to do with it!
//                        poi.insert("locality", name);
                        break;
                    }
                    else if (type_text=="administrative_area_level_1")
                    {
						poi.setRegion(name);
                        break;
                    }
                    else if (type_text=="country")
                    {
						poi.setCountry(name);
                        name_element=address_component.firstChildElement("short_name");
                        if (!name_element.isNull())
							poi.setCountryCode(name_element.text());
                        break;
                    }
                    else if (type_text=="postal_code")
                    {
						poi.setPostalCode(name);
                        break;
                    }
                }
            }

            QString text;
			if (poi.hasProperty(GeolocElement::Street))
            {
				text = poi.street();
				if (poi.hasProperty(GeolocElement::Building))
					text.append(", ").append(poi.building());
            }
			else if (poi.hasProperty(GeolocElement::Building))
				text = poi.building();
			else if (poi.hasProperty(GeolocElement::Locality))
				text = poi.locality();
			else if (poi.hasProperty(GeolocElement::Region))
				text = poi.region();
			else if (poi.hasProperty(GeolocElement::Country))
				text = poi.country();
			poi.setText(text);

            // Examine types
            for(QDomElement type=result.firstChildElement("type");
                !type.isNull();
                type=type.nextSiblingElement("type"))
            {
                QString value = type.text();
                if (value == "street_address")
                {
					if (poi.hasProperty(GeolocElement::Building))
						poi.setType("misc:housenumber");
                    else
						poi.setType("highway:secondary");
                    break;
                }
                else if (value == "route")
                {
					poi.setType("highway:primary");
                    break;
                }
                else if (value == "intersection")
                {
					poi.setType("highway:secondary");
                    break;
                }
                else if (value == "political")
                {
//                    poi.insert("type", "highway:secondary");
//                    break;
                }
                else if (value == "intersection")
                {
					poi.setType("highway:secondary_link");
                    break;
                }
                else if (value == "administrative_area_level_1")
                {
					poi.setType("place:county");
                    break;
                }
                else if (value == "administrative_area_level_2")
                {
//                    break;
                }
                else if (value == "administrative_area_level_3")
                {
//                    break;
                }
                else if (value == "colloquial_area")
                {
//                    break;
                }
                else if (value == "locality")
                {
					poi.setType("place:city");
                    break;
                }
                else if (value == "sublocality")
                {
					poi.setType("place:district");
                    break;
                }
                else if (value == "neighborhood")
                {
					poi.setType("place:suburbs");
                    break;
                }
                else if (value == "premise")
                {
					poi.setType("geographic:building");
                    break;
                }
                else if (value == "subpremise")
                {
					poi.setType("place:house");
                    break;
                }
                else if (value == "postal_code")
                {
					poi.setType("place:building");
                    break;
                }
                else if (value == "natural_feature")
                {
					poi.setType("nature");
                    break;
                }
                else if (value == "airport")
                {
					poi.setType("aeroway");
                    break;
                }
                else if (value == "park")
                {
					poi.setType("leisure:park");
                    break;
                }
                else if (value == "point_of_interest")
                {
					poi.setType("sightseeing");
                    break;
                }
            }

            QDomElement geometry=result.firstChildElement("geometry");
            if (!geometry.isNull())
            {
                QDomElement location=geometry.firstChildElement("location");
                if (!location.isNull())
                {
                    QDomElement lat=location.firstChildElement("lat");
                    if (!lat.isNull())
						poi.setLat(lat.text().toDouble());
                    QDomElement lng=location.firstChildElement("lng");
                    if (!lng.isNull())
						poi.setLon(lng.text().toDouble());
                }
            }
            emit receivedPoi(poi);
        }
    }
    emit searchFinished(false);  // Signal search finished
}

void MapSearchProviderGoogle::onResultReceived(const QByteArray &AResult, const QString &AId)
{
    Q_UNUSED(AId)
    if (!AResult.isNull())
        parseResult(AResult);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearchprovidergoogle, MapSearchProviderGoogle)
#endif

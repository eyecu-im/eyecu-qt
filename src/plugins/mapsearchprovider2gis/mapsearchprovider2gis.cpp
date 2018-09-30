#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <utils/iconstorage.h>
#include "mapsearchprovider2gis.h"
#include "mapsearchprovider2gisoptions.h"

MapSearchProvider2gis::MapSearchProvider2gis(QObject *parent):
	QObject(parent), FOptionsManager(nullptr), FLastResultFound(0)
{}

void MapSearchProvider2gis::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search Provider 2GIS");
    APluginInfo->description = tr("Allows to use 2GIS as a map search provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool MapSearchProvider2gis::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
    AInitOrder = 200;
    return true;
}

bool MapSearchProvider2gis::initObjects()
{
	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

bool MapSearchProvider2gis::initSettings()
{
	Options::setDefaultValue(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE, MapSearchProvider2GisOptions::SearchFirm);

    return true;
}

QMultiMap<int, IOptionsDialogWidget *> MapSearchProvider2gis::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (ANodeId == OPN_MAPSEARCH)
		widgets.insertMulti(OWO_MAPSEARCH_PROVIDER_2GIS, new MapSearchProvider2GisOptions(AParent));
    return widgets;
}

bool MapSearchProvider2gis::startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore)
{
	Q_UNUSED(AZoom)
	Q_UNUSED(ALimitRange)

    QUrl request = Options::node(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE).value().toInt() == MapSearchProvider2GisOptions::SearchFirm?
                searchRequest(ASearchString, ALatSouth, ALngWest, ALatNorth, ALngEast, AMaxResults, AMore):
                searchRequest(ASearchString, ALatSouth, ALngWest, ALatNorth, ALngEast, AMaxResults);

    return FHttpRequester->request(request, "request", this, SLOT(onResultReceived(QByteArray,QString)));
}

QString MapSearchProvider2gis::sourceName() const
{
    return tr("2GIS");
}

QIcon MapSearchProvider2gis::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_2GIS);
}

int MapSearchProvider2gis::features() const
{
    int features = FeatureLimitRange|FeatureLimitRangeAlways|FeatureMaxResults;
    if (Options::node(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE).value().toInt() == MapSearchProvider2GisOptions::SearchFirm)
        features|=FeatureNextPage;
    return features;
}

void MapSearchProvider2gis::getPageValues(int *AMin, int *AMax, int *ADefault) const
{
    if (Options::node(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE).value().toInt() == MapSearchProvider2GisOptions::SearchFirm)
    {
        if (AMin)
            *AMin = 5;
        if (AMax)
            *AMax = 50;
		if (ADefault)
			*ADefault = 20;
    }
    else
    {
        if (AMin)
			*AMin = 1;
        if (AMax)
			*AMax = 2000;
		if (ADefault)
			*ADefault = 1;
    }    
}

QUrl MapSearchProvider2gis::searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int APageSize, int AMore)
{
    if (!AMore)
        FLastResultFound = 0;
    int page;
    QString request("http://catalog.api.2gis.ru/search?what=%1&bound[point1]=%2,%3&bound[point2]=%4,%5&page=%6&key=ruihvk0699&version=1.3&sort=distance&output=xml");
    if (APageSize != -1)
    {
        page = FLastResultFound/APageSize+1;
        request.append("&pagesize=%1").append(APageSize);
    }
    else
        page = 1;
    return QUrl(request.arg(ASearchString).arg(ALngWest).arg(ALatNorth).arg(ALngEast).arg(ALatSouth).arg(page));
}

QUrl MapSearchProvider2gis::searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int ALimit)
{
	QString request("http://catalog.api.2gis.ru/geo/search?q=%1&bound[point1]=%2,%3&bound[point2]=%4,%5&key=ruihvk0699&version=1.3&sort=distance&output=xml");
	if (ALimit!=-1)
		request.append(QString("&limit=%1").arg(ALimit));
	return QUrl(request.arg(ASearchString).arg(ALngWest).arg(ALatNorth).arg(ALngEast).arg(ALatSouth));
}


void MapSearchProvider2gis::parseResult(QByteArray ASearchResult)
{    
    QDomDocument doc;
    doc.setContent(ASearchResult);
    QDomElement root=doc.documentElement();
    bool moreResultsAvailable = false;
    if (!root.isNull())
    {
        QDomElement result=root.firstChildElement("result");
        if (!result.isNull())
        {
            QDomElement element = result.firstChildElement("filial");
            if (element.isNull())    // No "filial" elements! Let's try geoObjects...
            {
                for(QDomElement geoObject=result.firstChildElement("geoObject");
                    !geoObject.isNull();
                    geoObject=geoObject.nextSiblingElement("geoObject"))
                {
                    GeolocElement poi;
					poi.setCountry(tr("Russian Federation (Russia)"));
					poi.setCountryCode("RU");

                    QDomElement type = geoObject.firstChildElement("type");
                    if (!type.isNull())
                    {
                        QString value=type.text();
                        if (value=="district")
							poi.setType("place:district");
                        else if (value=="house")
							poi.setType("geographic:building");
                        else if (value=="city")
							poi.setType("place:city");
                        else if (value=="city")
							poi.setType("place:city");
                        else if (value=="settlement")
							poi.setType("place:village");
                        else if (value=="city")
							poi.setType("place:city");
                        else if (value=="station")
							poi.setType("public_transport:station");
                        else if (value=="station_platform")
							poi.setType("public_transport:platform");
                        else if (value=="street")
							poi.setType("highway:primary");
                        else if (value=="living_area")
							poi.setType("highway:residential");
                        else if (value=="place")
							poi.setType("place");
                        else if (value=="sight")
							poi.setType("sightseeing");
                        else if (value=="crossroad")
							poi.setType("highway:road");
                        else if (value=="metro")
							poi.setType("railway:subway");
                    }

                    QDomElement centroid = geoObject.firstChildElement("centroid");
                    if (!centroid.isNull())
                    {
                        QString value = centroid.text();
                        if (value.startsWith("POINT(") && value.endsWith(")"))
                        {
                            QStringList coordinates = value.mid(6, value.length()-7).split(' ');
                            if (coordinates.size()==2)
                            {
								poi.setLon(coordinates[0].toDouble());
								poi.setLat(coordinates[1].toDouble());
                            }
                        }
                    }

                    QDomElement attributes = geoObject.firstChildElement("attributes");
                    if (!attributes.isNull())
                    {
                        QDomElement city = attributes.firstChildElement("city");
                        if (!city.isNull())
							poi.setLocality(city.text());
                        QDomElement district = attributes.firstChildElement("district");
                        if (!district.isNull())
							poi.setArea(district.text());
                        QDomElement microdistrict = attributes.firstChildElement("microdistrict");
                        if (!microdistrict.isNull())
                        {
							if (poi.hasProperty(GeolocElement::Area))
								poi.setArea(poi.area() + " " + microdistrict.text());
                            else
								poi.setArea(microdistrict.text());
                        }
                        QDomElement street = attributes.firstChildElement("street");
                        if (!street.isNull())
							poi.setStreet(street.text());
                        QDomElement number = attributes.firstChildElement("number");
                        if (!number.isNull())
							poi.setBuilding(number.text());
                        QDomElement buildingname = attributes.firstChildElement("buildingname");
                        if (!buildingname.isNull())
                        {
							poi.setDescription(buildingname.text());
							if (!poi.hasProperty(GeolocElement::Building))
								poi.setBuilding(buildingname.text());
                        }
                        QDomElement index = attributes.firstChildElement("index");
                        if (!index.isNull())
							poi.setPostalCode(index.text());
                    }

                    QDomElement synonym = geoObject.firstChildElement("synonym");
                    if (!synonym.isNull())
                    {
						poi.setText(synonym.text());
						if (!poi.hasProperty(GeolocElement::Description))
							poi.setDescription(synonym.text());
                    }

                    QDomElement shortName = geoObject.firstChildElement("short_name");
                    if (!shortName.isNull())
						if (!poi.hasProperty(GeolocElement::Text))
							poi.setText(shortName.text());

                    QDomElement name = geoObject.firstChildElement("name");
                    if (!name.isNull())
                    {
						if (!poi.hasProperty(GeolocElement::Text))
							poi.setText(name.text());
						if (!poi.hasProperty(GeolocElement::Description))
							poi.setDescription(name.text());
                    }
                    emit receivedPoi(poi);
                }
				moreResultsAvailable = false;
            }
            else
            {
                for(QDomElement filial=element;
                    !filial.isNull();
                    filial=filial.nextSiblingElement("filial"))
                {
                    GeolocElement poi;
					poi.setCountry(tr("Russian Federation (Russia)"));
					poi.setCountryCode("RU");

                    QString name=filial.firstChildElement("name").text();
                    if (!name.isNull())
						poi.setText(name);

                    QDomElement city_name=filial.firstChildElement("city_name");
                    if (!city_name.isNull())
						poi.setLocality(city_name.text());

                    QString address=filial.firstChildElement("address").text();

                    QStringList split_address=address.split(", ");
					poi.setStreet(split_address[0]);
                    if (split_address.size()>1)
						poi.setBuilding(split_address[1]);
					poi.setDescription(address);

                    QDomElement lat=filial.firstChildElement("lat");
                    if (!lat.isNull())
						poi.setLat(lat.text().toDouble());

                    QDomElement lon=filial.firstChildElement("lon");
                    if (!lon.isNull())
						poi.setLon(lon.text().toDouble());

                    QString description;
                    QDomElement micro_comment=filial.firstChildElement("micro_comment");
                    if (!micro_comment.isNull())
                        description.append(micro_comment.text());

                    QDomElement fas_warning=filial.firstChildElement("fas_warning");
                    if (!fas_warning.isNull())
                    {
                        if (!description.isEmpty())
                            description.append('\n');
                        description.append(QString("FAS Warning: %1").arg(fas_warning.text()));
                    }

                    QDomElement rubrics=filial.firstChildElement("rubrics");
                    if (!rubrics.isNull())
                    {
                        if (!description.isEmpty())
                            description.append('\n');
                        description.append(tr("Rubrics:"));
                        for(QDomElement rubric=rubrics.firstChildElement("rubric");
                            !rubric.isNull();
                            rubric=rubric.nextSiblingElement("rubric"))
                            description.append('\n').append(rubric.text());
                    }
                    if (!description.isEmpty())
						poi.setDescription(description);
                    emit receivedPoi(poi);
                    FLastResultFound++;
                }
				moreResultsAvailable = FLastResultFound < result.attribute("total").toInt();
            }            
        }
    }
    emit searchFinished(moreResultsAvailable);
}

void MapSearchProvider2gis::onResultReceived(const QByteArray &AResult, const QString &AId)
{
    Q_UNUSED(AId)
    if (!AResult.isNull())
        parseResult(AResult);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearchprovider2gis, MapSearchProvider2gis)
#endif

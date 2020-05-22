#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>
#include <QHashBuilder>
#if QT_VERSION > 0x050000
#include <QUrlQuery>
#endif
#include "mapsearchproviderhere.h"
#include "mapsearchproviderhereoptions.h"

MapSearchProviderHere::MapSearchProviderHere(QObject *parent):
	QObject(parent),
	FOptionsManager(nullptr),
	FPoi(nullptr),
	FHttpRequester(nullptr),
	FNextPageInformation(0),
	fLocales(QHashBuilder<QString,QString>()
		.insert("ar","ara")
		.insert("eu","baq")
		.insert("ca", "cat")
		.insert("zh", "chi")
		.insert("zh", "chi")
//TODO: implement traditional chinese language
//		.insert("zh", "cht")
		.insert("cs", "cze")
		.insert("da", "dan")
		.insert("nl", "dut")
		.insert("en", "eng")
		.insert("fi", "fin")
		.insert("fr", "fre")
		.insert("de", "ger")
		.insert("ga", "gle")
		.insert("el", "gre")
		.insert("he", "heb")
		.insert("hi", "hin")
		.insert("id", "ind")
		.insert("it", "ita")
		.insert("no", "nor")
		.insert("no", "nor")
		.insert("fa", "per")
		.insert("pl", "pol")
		.insert("pt", "por")
		.insert("ru", "rus")
		.insert("si", "sin")
		.insert("si", "sin")
		.insert("es", "spa")
		.insert("es", "spa")
		.insert("sv", "swe")
		.insert("sv", "swe")
		.insert("th", "tha")
		.insert("tr", "tur")
		.insert("uk", "ukr")
		.insert("uk", "ukr")
		.insert("ur", "urd")
		.insert("vi", "vie")
		.insert("cy", "wel")
		.result()),
	fCountries(QMapBuilder<QLocale::Country,QString>()
		.insert(QLocale::AnyCountry,"DEF")
		.insert(QLocale::UnitedArabEmirates,"ARE")
		.insert(QLocale::Argentina,"ARG")
		.insert(QLocale::Bahrain,"BHR")
		.insert(QLocale::Egypt,"EGY")
		.insert(QLocale::India,"IND")
		.insert(QLocale::Kuwait,"KWT")
		.insert(QLocale::Morocco,"MAR")
		.insert(QLocale::Oman,"OMN")
		.insert(QLocale::Pakistan,"PAK")
		.insert(QLocale::Qatar,"QAT")
#if QT_VERSION < 0x050000
		.insert(QLocale::RussianFederation,"RUS")
		.insert(QLocale::VietNam,"VIE")
#else
	   .insert(QLocale::Russia,"RUS")
	   .insert(QLocale::Vietnam,"VIE")
#endif
		.result())
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
	IPlugin *plugin=APluginManager->pluginInterface("IPoi").value(0, nullptr);
	if (plugin)
		FPoi = qobject_cast<IPoi *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0, nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	AInitOrder = 200;
	return true;
}

bool MapSearchProviderHere::initObjects()
{
	fillCountryCodes();

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);

	return true;
}

bool MapSearchProviderHere::initSettings()
{
	Options::setDefaultValue(OPV_MAP_SEARCH_PROVIDER_HERE_POLITICALVIEW, QString());
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

QMultiMap<int, IOptionsDialogWidget *> MapSearchProviderHere::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_MAPSEARCH)
		widgets.insertMulti(OWO_MAPSEARCH_PROVIDER_HERE,
							new MapSearchProviderHereOptions(this, AParent));
	return widgets;
}

QString MapSearchProviderHere::country(QLocale::Country aCountry)
{
	return fCountries.value(aCountry, QString());
}

QString MapSearchProviderHere::country(const QLocale &aLocale)
{
	return country(aLocale.country());
}

QList<QLocale::Country> MapSearchProviderHere::countries()
{
	return fCountries.keys();
}

#define APP_ID "VKGZLTjvEdi1xsdDMKJp"
#define APP_CODE "I7t8K4lZHm7pLLZ0k8XIwg"
#define GEN 9

QUrl MapSearchProviderHere::searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, bool ALimitRange, int AMaxResults, bool AMore)
{
	QString language(lang(QLocale()));
	QUrl url("http://geocoder.api.here.com/6.2/geocode.xml");
	QList<QPair<QString,QString> > queryItems;
	queryItems.append(QPair<QString,QString>("app_id", APP_ID));
	queryItems.append(QPair<QString,QString>("app_code", APP_CODE));
	queryItems.append(QPair<QString,QString>("gen", QString::number(GEN)));
	queryItems.append(QPair<QString,QString>("searchtext", ASearchString));
	queryItems.append(QPair<QString,QString>("language", language));
	QString viewport(QString("%1,%2;%3,%4").arg(ALatNorth)
										   .arg(ALngWest)
										   .arg(ALatSouth)
										   .arg(ALngEast));
	queryItems.append(QPair<QString,QString>(ALimitRange?"mapview":"bbox",
											 viewport));
	if (AMaxResults != -1)
		queryItems.append(QPair<QString,QString>("maxresults",
												 QString::number(AMaxResults)));
	if (AMore)
		queryItems.append(QPair<QString,QString>("pageinformation",
												 QString::number(FNextPageInformation)));
	QString politicalView = Options::node(OPV_MAP_SEARCH_PROVIDER_HERE_POLITICALVIEW).value().toString();
	if (politicalView.isEmpty())
		politicalView = country(QLocale());
	queryItems.append(QPair<QString,QString>("politicalview", politicalView));

#if QT_VERSION > 0x050000
	QUrlQuery query;
	query.setQueryItems(queryItems);
	url.setQuery(query);
#else
	url.setQueryItems(queryItems);
#endif
	return url;
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

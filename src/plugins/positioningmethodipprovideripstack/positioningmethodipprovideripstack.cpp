#include "positioningmethodipprovideripstack.h"

#include <definitions/resources.h>
#include <definitions/menuicons.h>

#include <utils/iconstorage.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

#define	REQUEST_ID "ipstack"
#define API_KEY "ed5ff0672b9782bd2372f39dcf6aae4c"
#define REQUEST_URL "http://api.ipstack.com/check?output=xml&fields=main"

PositioningMethodIpProviderFreegeoip::PositioningMethodIpProviderFreegeoip(QObject *parent): QObject(parent)
{}

void PositioningMethodIpProviderFreegeoip::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Positining Method IP Provider ipstack");
	APluginInfo->description = tr("Allows to use ipstack as an IP positioning provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(POSITIONINGMETHODIP_UUID);
}

bool PositioningMethodIpProviderFreegeoip::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)
	Q_UNUSED(AInitOrder)
    return true;
}

bool PositioningMethodIpProviderFreegeoip::request()
{
	QUrl url(REQUEST_URL);
	URL_ADD_QUERY_ITEM(url, "access_key", API_KEY)

	return FHttpRequester->request(url, REQUEST_ID, this, SLOT(onResultReceived(QByteArray,QString)));
}

QString PositioningMethodIpProviderFreegeoip::name() const
{
	return tr("ipstack");
}

QIcon PositioningMethodIpProviderFreegeoip::icon() const
{
	return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_POSITIONING_IP_IPSTACK);
}

void PositioningMethodIpProviderFreegeoip::parseResult(QByteArray ASearchResult)
{
    QDomDocument doc;
	doc.setContent(ASearchResult);

	QDomElement root = doc.documentElement();
	if (root.tagName() == "result")	// Ok
	{
		GeolocElement geoloc;
		QDomElement countryCode = root.firstChildElement("country_code");
		if (!countryCode.isNull()  && !countryCode.text().isEmpty())
			geoloc.setCountryCode(countryCode.text());
		QDomElement countryName = root.firstChildElement("country_name");
		if (!countryName.isNull() && !countryName.text().isEmpty())
			geoloc.setCountry(countryName.text());
		QDomElement regionName = root.firstChildElement("region_name");
		if (!regionName.isNull() && !regionName.text().isEmpty())
			geoloc.setRegion(regionName.text());
		QDomElement city = root.firstChildElement("city");
		if (!city.isNull() && !city.text().isEmpty())
			geoloc.setLocality(city.text());
		QDomElement zip = root.firstChildElement("zip");
		if (!zip.isNull() && !zip.text().isEmpty())
			geoloc.setPostalCode(zip.text());
		QDomElement latitude = root.firstChildElement("latitude");
		if (!latitude.isNull() && !latitude.text().isEmpty())
		{
			bool ok;
			double lat = latitude.text().toDouble(&ok);
			if (ok)
				geoloc.setLat(lat);
		}
		QDomElement longitude = root.firstChildElement("longitude");
		if (!longitude.isNull() && !longitude.text().isEmpty())
		{
			bool ok;
			double lon = longitude.text().toDouble(&ok);
			if (ok)
				geoloc.setLon(lon);
		}
		emit newPositionAvailable(geoloc);
	}
	else
		emit requestError();
}

void PositioningMethodIpProviderFreegeoip::onResultReceived(const QByteArray &AResult, const QString &AId)
{
	if (AId == REQUEST_ID)
	{
		if (!AResult.isNull())
			parseResult(AResult);
		else
			emit requestError();
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_positioningmethodipproviderfreegeoip, PositioningMethodIpProviderFreegeoip)
#endif

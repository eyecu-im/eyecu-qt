#include <definitions/resources.h>
#include <definitions/menuicons.h>

#include <utils/iconstorage.h>
#include <utils/logger.h>

#include "positioningmethodipproviderfreegeoip.h"

#define	REQUEST_ID "freegeoip"

PositioningMethodIpProviderFreegeoip::PositioningMethodIpProviderFreegeoip(QObject *parent): QObject(parent)
{}

void PositioningMethodIpProviderFreegeoip::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Positining Method IP Provider freegeoip.net");
    APluginInfo->description = tr("Allows to use freegeoip.net as an IP positioning provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool PositioningMethodIpProviderFreegeoip::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)
	Q_UNUSED(AInitOrder)
    return true;
}

bool PositioningMethodIpProviderFreegeoip::request()
{
	return FHttpRequester->request(QUrl("https://freegeoip.net/xml/"), REQUEST_ID, this, SLOT(onResultReceived(QByteArray,QString)));
}

QString PositioningMethodIpProviderFreegeoip::name() const
{
	return tr("freegeoip.net");
}

QIcon PositioningMethodIpProviderFreegeoip::icon() const
{
	return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_POSITIONING_PROV_FREEGEOIP);
}

void PositioningMethodIpProviderFreegeoip::parseResult(QByteArray ASearchResult)
{
    QDomDocument doc;
	doc.setContent(ASearchResult);

	QDomElement root = doc.documentElement();
	if (root.tagName() == "Response")	// Ok
	{
		GeolocElement geoloc;
		QDomElement countryCode = root.firstChildElement("CountryCode");
		if (!countryCode.isNull()  && !countryCode.text().isEmpty())
			geoloc.setCountryCode(countryCode.text());
		QDomElement countryName = root.firstChildElement("CountryName");
		if (!countryName.isNull() && !countryName.text().isEmpty())
			geoloc.setCountry(countryName.text());
		QDomElement regionName = root.firstChildElement("RegionName");
		if (!regionName.isNull() && !regionName.text().isEmpty())
			geoloc.setRegion(regionName.text());
		QDomElement city = root.firstChildElement("City");
		if (!city.isNull() && !city.text().isEmpty())
			geoloc.setLocality(city.text());
		QDomElement zipCode = root.firstChildElement("ZipCode");
		if (!zipCode.isNull() && !zipCode.text().isEmpty())
			geoloc.setPostalCode(zipCode.text());
		QDomElement latitude = root.firstChildElement("Latitude");
		if (!latitude.isNull() && !latitude.text().isEmpty())
		{
			bool ok;
			double lat = latitude.text().toDouble(&ok);
			if (ok)
				geoloc.setLat(lat);
		}
		QDomElement longitude = root.firstChildElement("Longitude");
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

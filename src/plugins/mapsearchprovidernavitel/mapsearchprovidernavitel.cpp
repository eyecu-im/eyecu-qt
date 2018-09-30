#include <QLocale>
#if QT_VERSION < 0x050000
#include <QScriptEngine>
#include <QScriptValue>
#define JSON_ARRAY 	QVariantList
#define TO_ARRAY 	toList
#define TO_LONGLONG	toLongLong
#else
#include <QJsonDocument>
#include <QJsonArray>
#define JSON_ARRAY 	QJsonArray
#define TO_ARRAY 	toArray
#define TO_LONGLONG	toDouble
#endif
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <utils/iconstorage.h>
#include "mapsearchprovidernavitel.h"

MapSearchProviderNavitel::MapSearchProviderNavitel(QObject *parent) : QObject(parent)
{}

void MapSearchProviderNavitel::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Map Search Provider Navitel");
    APluginInfo->description = tr("Allows to use Navitel as a map search provider");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(MAPSEARCH_UUID);
}

bool MapSearchProviderNavitel::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)

    AInitOrder = 200;
    return true;
}

bool MapSearchProviderNavitel::initObjects()
{
    loadFieldTypes();
    fullNavitelTypes();
    return true;
}

bool MapSearchProviderNavitel::initSettings()
{
    return true;
}

bool MapSearchProviderNavitel::startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore)
{
	Q_UNUSED(AMore)
	Q_UNUSED(ALimitRange)
	Q_UNUSED(AMaxResults)

    qreal longitude = (ALngWest + ALngEast)/2;
    qreal latitude  = (ALatNorth + ALatSouth)/2;
    QUrl request = searchRequest(ASearchString, latitude, longitude, AZoom);
	return FHttpRequester->request(request, "search", this, SLOT(onResultReceived(QByteArray,QString)));
}

QString MapSearchProviderNavitel::sourceName() const
{
    return tr("Navitel");
}

QIcon MapSearchProviderNavitel::sourceIcon() const
{
    return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAP_NAVITEL);
}

QUrl MapSearchProviderNavitel::searchRequest(const QString &ASearchString, qreal ALongitude, qreal ALatitude, int AZoom) const
{    
	QUrl url(QString("http://maps.navitel.ru/webmaps/searchTwoStep?s=%1&lon=%2&lat=%3&z=%4")
		.arg(ASearchString)
        .arg(ALongitude)
        .arg(ALatitude)
        .arg(AZoom));

	return url;
}

QUrl MapSearchProviderNavitel::infoRequest(int ALocationId) const
{
	qDebug() << "MapSearchProviderNavitel::infoRequest(" << ALocationId << ")";
	QUrl url(QString("http://maps.navitel.ru/webmaps/searchTwoStepInfo?id=%1").arg(ALocationId));
	return url;
}

void MapSearchProviderNavitel::parseSearchResult(QByteArray ASearchResult)
{
	qDebug() << "MapSearchProviderNavitel::parseSearchResult(" << ASearchResult << ")";
#if QT_VERSION < 0x050000
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("("+QString::fromUtf8(ASearchResult)+")");
    if (value.isValid())
    {
        QVariantList list = value.toVariant().toList();
#else
	QJsonDocument jsonDoc = QJsonDocument::fromJson(ASearchResult);
	if (jsonDoc.isArray())
	{
		QJsonArray list = jsonDoc.array();
#endif
		if (!list.isEmpty())
		{
			for (JSON_ARRAY::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
			{
				JSON_ARRAY result = (*it).TO_ARRAY();
				qulonglong id = qulonglong(result.first().TO_LONGLONG());
				JSON_ARRAY info1 = result[1].TO_ARRAY();

				GeolocElement poi;
				int type = result[2].toInt();
				QString typeValue = FNavitelTypes.value(type);

				if (!typeValue.isEmpty())
					poi.setType(typeValue);
				if (info1.size()==2)
				{
					poi.setBuilding(info1[0].toString());
					poi.setStreet(info1[1].toString());
					poi.setText(info1[1].toString()+", "+info1[0].toString());
				}
				else if (info1.size()==1)
				{
					poi.setText(info1.first().toString());
					if (type==33)
						poi.setLocality(info1.first().toString());
				}
//TODO: Find out what is this id for
//				int id2 = result.value(3).toLongLong();
				QString country;
				QString region;
				QString locality;
				QString street;
				JSON_ARRAY info2 = result.at(4).TO_ARRAY();
				for (JSON_ARRAY::ConstIterator it=info2.constBegin(); it!=info2.constEnd(); it++)
                {
                    QString value = (*it).toString();
                    FieldType type(Unknown);
                    for (QHash<FieldType, QString>::ConstIterator it1 = FFieldTypes.constBegin(); it1!= FFieldTypes.constEnd(); it1++)
                    {
                        QString text = it1.value();
                        bool suffix = text[0]==' ';
                        if (suffix) // Suffix
                        {
                            if (value.endsWith(text))
                            {
                                type = it1.key();
                                break;
                            }
                        }
                        else        // prefix
                        {
                            if (value.startsWith(text))
                            {
                                type = it1.key();
                                break;
                            }
                        }

                    }

                    switch (type)
                    {
                        case Locality:
                            if (!locality.isEmpty())
                                locality.append(", ");
                            locality.append(value);
                            break;
                        case Street:
                            if (!street.isEmpty())
                                street.append(", ");
                            street.append(value);
                            break;
                        case Region:
                        case SubRegion:
                            if (!region.isEmpty())
                                region.append(", ");
                            region.append(value);
                            break;
                        case Unknown:
                            if (it+1==info2.constEnd()) // The last item in the list
                                country = value;
                            break;
                    }
                }

                if (!country.isEmpty())
					poi.setCountry(country);
                if (!region.isEmpty())
					poi.setRegion(region);
                if (!locality.isEmpty())
					poi.setLocality(locality);
                if (!street.isEmpty())
					poi.setStreet(street);
                FRequestedPois.insert(id, poi);
                QUrl request = infoRequest(id);
                FHttpRequester->request(request, QString::number(id), this, SLOT(onResultReceived(QByteArray,QString)));
            }
            return;
        }
    }
    emit searchFinished(false);  // Signal search finished
}

void MapSearchProviderNavitel::parseInfoResult(QByteArray ASearchResult, qulonglong AId)
{
#if QT_VERSION < 0x050000
    QScriptEngine engine;
    QScriptValue value = engine.evaluate("("+QString::fromUtf8(ASearchResult)+")");
    if (value.isValid())
    {
		QVariantList list = value.toVariant().toList();
#else
	QJsonDocument jsonDoc = QJsonDocument::fromJson(ASearchResult);
	if (jsonDoc.isArray())
	{
		QJsonArray list = jsonDoc.array();
#endif
		if (list.size()==3)
		{
			GeolocElement poi = FRequestedPois.take(AId);
			if (!poi.isEmpty())
			{
				poi.setLon(list[0].toDouble());
				poi.setLat(list[1].toDouble());
                emit receivedPoi(poi);  // Send POI
            }
        }
    }
    emit searchFinished(false);  // Signal search finished
}

void MapSearchProviderNavitel::onResultReceived(const QByteArray &AResult, const QString &AId)
{
    Q_UNUSED(AId)

	if (AResult.isNull())
		emit searchFinished(false);  // Signal search finished
	else
    {
        if (AId=="search")
            parseSearchResult(AResult);
        else
            parseInfoResult(AResult, AId.toULongLong());
    }
}

void MapSearchProviderNavitel::getPageValues(int *AMin, int *AMax, int *ADefault) const
{
    if (AMin)
        *AMin = 0;
    if (AMax)
        *AMax = 0;
    if (ADefault)
        *ADefault = 0;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_mapsearchprovidernavitel, MapSearchProviderNavitel)
#endif

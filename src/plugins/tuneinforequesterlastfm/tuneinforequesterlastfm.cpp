#include <QDomDocument>

#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionwidgetorders.h>
#include <TuneInfoHttpQuery>

#include "tuneinforequesterlastfm.h"
#include "tuneinforequesterlastfmoptions.h"

const QString TuneInfoRequesterLastFm::api_key("237f5601b11659c9feecb46902349e14");
const QStringList TuneInfoRequesterLastFm::image_sizes(QStringList() << "small" << "medium" << "large" << "extralarge" << "mega");

TuneInfoRequesterLastFm::TuneInfoRequesterLastFm(QObject *parent) :
    QObject(parent),
    FOptionsManager(NULL)
{}

void TuneInfoRequesterLastFm::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Data Requester last.fm");
    APluginInfo->description = tr("Allows Tune plugin to request tune data from last.fm service");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneInfoRequesterLastFm::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)
    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
    return true;
}

bool TuneInfoRequesterLastFm::initObjects()
{
    return true;
}

bool TuneInfoRequesterLastFm::initSettings()
{    
    Options::setDefaultValue(OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE, Medium);
    Options::setDefaultValue(OPV_TUNE_INFOREQUESTER_LASTFM_AUTOCORRECT, true);
    if (FOptionsManager)
        FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> TuneInfoRequesterLastFm::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_PEP)
		widgets.insertMulti(OWO_PEP_USERTUNE_LASTFM, new TuneInfoRequesterLastFmOptions(AParent));
    return widgets;
}

bool TuneInfoRequesterLastFm::requestTuneInfo(QNetworkAccessManager *ANetworkAccessManager, const QString &AArtist, const QString &AAlbum, const QString &ATrack)
{
    QUrl url;
    if (!AArtist.isEmpty())
    {
        if (!ATrack.isEmpty())
            url = QString("http://ws.audioscrobbler.com/2.0/?method=track.getInfo&api_key=%1&artist=%2&track=%3&autocorrect=%4").arg(api_key).arg(AArtist).arg(ATrack).arg(Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_AUTOCORRECT).value().toBool()?1:0);
        else if (!AAlbum.isEmpty())
            url = QString("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=%1&artist=%2&album=%3&autocorrect=%4").arg(api_key).arg(AArtist).arg(AAlbum).arg(Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_AUTOCORRECT).value().toBool()?1:0);
        else
            url = QString("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&api_key=%1&artist=%2&autocorrect=%4").arg(api_key).arg(AArtist).arg(Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_AUTOCORRECT).value().toBool()?1:0);
    }
    if (url.isEmpty())
        return false;
    TuneInfoHttpQuery *httpRequest=new TuneInfoHttpQuery(url, AArtist, AAlbum, ATrack);
    connect(httpRequest, SIGNAL(resultReceived(QByteArray,QString,QString,QString)), SLOT(onResultReceived(QByteArray,QString,QString,QString)));
    httpRequest->sendRequest(ANetworkAccessManager);
    return true;
}

QHash<QString, QString> TuneInfoRequesterLastFm::parseResult(const QByteArray &AResult) const
{
    QHash<QString, QString> result;
    QDomDocument doc;
    doc.setContent(AResult);
    QDomElement lfm=doc.firstChildElement("lfm");
    if (lfm.attribute("status")=="ok")
    {
        QDomElement entity=lfm.firstChildElement("track");
        if (entity.isNull())
        {
            entity=lfm.firstChildElement("album");
            if (entity.isNull())
                entity=lfm.firstChildElement("artist");
        }
        if (!entity.isNull())
        {
            QString imageSize=image_sizes.value(Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE).value().toInt());
            QString imageUrl;
            for (QDomElement image=entity.firstChildElement("image"); !image.isNull(); image=image.nextSiblingElement("image"))
            {
                if (!image.text().isEmpty())
                {
                    imageUrl=image.text();
                    if (image.attribute("size")==imageSize)
                        break;
                }
            }
            if (!imageUrl.isEmpty())
                result.insert("image", imageUrl);
            QDomElement url=entity.firstChildElement("url");
            if (!url.isNull() && !url.text().isEmpty())
                result.insert("url", url.text());
        }
    }
    return result;
}

// SLOTS
void TuneInfoRequesterLastFm::onResultReceived(const QByteArray &AResult, const QString &AArtist, const QString &AAlbum, const QString &ATrack)
{
    QHash<QString, QString> tuneInfo = parseResult(AResult);
    emit tuneInfoReceived(AArtist, AAlbum, ATrack, tuneInfo);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tuneinforequesterlastfm, TuneInfoRequesterLastFm)
#endif

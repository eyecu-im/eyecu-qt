#include <QFile>
#include <QTextStream>

#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include "tunelistenerfile.h"
#include "tunelistenerfileoptions.h"

/**
 * \class TuneListenerFile
 * \brief Currently playing tune listener for AIMP.
 */

TuneListenerFile::TuneListenerFile():FOptionsManager(NULL),
                                     FWaitForCreated(true)
{}

TuneListenerFile::~TuneListenerFile()
{}

void TuneListenerFile::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener File");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from a file on the disk");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerFile::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)
	Q_UNUSED(AInitOrder)

	IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
    return true; //FMessageWidgets!=NULL
}

bool TuneListenerFile::initObjects()
{
    return true;
}

bool TuneListenerFile::initSettings()
{
    Options::setDefaultValue(OPV_TUNE_LISTENER_FILE_NAME, QString());
    Options::setDefaultValue(OPV_TUNE_LISTENER_FILE_FORMAT, Plain);
    if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> TuneListenerFile::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_PEP)
		widgets.insertMulti(OWO_PEP_USERTUNE_FILE, new TuneListenerFileOptions(AParent));
    return widgets;
}

// Methods to be implemented
TuneData TuneListenerFile::currentTune() const
{
    return FCurrentTune;
}

void TuneListenerFile::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_TUNE_LISTENER_FILE_NAME));
    onOptionsChanged(Options::node(OPV_TUNE_LISTENER_FILE_FORMAT));
}

void TuneListenerFile::onOptionsClosed()
{}

void TuneListenerFile::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_TUNE_LISTENER_FILE_NAME)
        FFileName=ANode.value().toString();
    else if (ANode.path() == OPV_TUNE_LISTENER_FILE_FORMAT)
        FFileFormat=(FileFormat)ANode.value().toInt();
}

void TuneListenerFile::check()
{
    TuneData existedTune = FCurrentTune;
    FCurrentTune.clear(); // just a reset
    if (QFile::exists(FFileName))
    {
        QFile file(FFileName);
        if (file.open(QIODevice::ReadOnly))
            switch (FFileFormat)
            {
                case Plain:
                {
                    QTextStream stream( &file );
                    stream.setCodec("UTF-8");
                    stream.setAutoDetectUnicode(true);
                    FCurrentTune.title = stream.readLine();
                    FCurrentTune.artist = stream.readLine();
                    FCurrentTune.source = stream.readLine();
                    FCurrentTune.track = stream.readLine();
                    FCurrentTune.length = stream.readLine().toInt();
                    break;
                }

                case XML:
                {
                    QDomDocument d;
                    d.setContent(&file, false);
                    QDomElement now_playing=d.documentElement();
                    QDomElement song=now_playing.firstChildElement("song");
                    if (!song.firstChildElement("title").text().isNull())
                        FCurrentTune.title = song.firstChildElement("title").text();
                    if (!song.firstChildElement("artist").text().isNull())
                        FCurrentTune.artist = song.firstChildElement("artist").text();
                    if (!song.firstChildElement("album").text().isNull())
                        FCurrentTune.source = song.firstChildElement("album").text();
                    if (!song.firstChildElement("track").text().isNull())
                        FCurrentTune.track = song.firstChildElement("track").text();
                    if (!song.firstChildElement("length").text().isNull())
                        FCurrentTune.length = song.firstChildElement("length").text().toInt();
                }
            }
    }
    else if (!FWaitForCreated && existedTune.isEmpty())
    {
        FWaitForCreated = true;
        return; // we will return to this function when file created. just exit for now.
    }
    // Common code
    TuneData tune = currentTune();
    if (FPreviousTune != tune)
    {
        FPreviousTune = tune;
        if (tune.isEmpty())
            emit stopped();
        else
            emit playing(tune);
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tunelistenerfile, TuneListenerFile)
#endif

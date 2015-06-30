#include <QFileInfo>

#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include "tunelistenerz.h"
#include "tunelistenerzoptions.h"
#include "utils/qt4qt5compat.h"
/**
 * \class TuneListenerZ
 * \brief Currently playing tune listener for Z! player.
 */

TuneListenerZ::TuneListenerZ():FOptionsManager(NULL)
{}

TuneListenerZ::~TuneListenerZ()
{}

void TuneListenerZ::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener Z!");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from Z! player");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerZ::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
    return true; //FMessageWidgets!=NULL
}

bool TuneListenerZ::initObjects()
{
    return true;
}

bool TuneListenerZ::initSettings()
{
    Options::setDefaultValue(OPV_TUNE_LISTENER_Z_PIPENAME, "\\PIPE\\ZMP3");
    if (FOptionsManager)
        FOptionsManager->insertOptionsHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> TuneListenerZ::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_PEP)
		widgets.insertMulti(OWO_PEP_USERTUNE_Z, new TuneListenerZOptions(AParent));
    return widgets;
}

// Methods to be implemented
TuneData TuneListenerZ::currentTune() const
{
    return FPreviousTune;
}

void TuneListenerZ::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_TUNE_LISTENER_Z_PIPENAME));
}

void TuneListenerZ::onOptionsClosed()
{}

void TuneListenerZ::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_TUNE_LISTENER_Z_PIPENAME)
        FPipeName=ANode.value().toString();
}

void TuneListenerZ::check()
{
    TuneData currentTune;
    HPIPE   pipeHandle;
    ULONG   ulAction;
    ULONG   ulWritten;

	if (!DosOpen((unsigned char *)FPipeName.TOASCII().data(),
            &pipeHandle, &ulAction, 0, FILE_NORMAL, FILE_OPEN,
			OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE, (PEAOP2) NULL))
    {
		if (!DosWrite(pipeHandle, "*rawinfo", 8, &ulWritten))
        {
            struct RawInfo rawInfo;
            ULONG ulBytesRead;
            if (!DosRead(pipeHandle, (PVOID)&rawInfo, sizeof(rawInfo), &ulBytesRead) && ulBytesRead>1)
            {
                QString track=QString::fromLocal8Bit(rawInfo.track);
                if (!track.isEmpty())
                    currentTune.title = track;
                else
                {
                    track=QString::fromLocal8Bit(rawInfo.fname);
                    if (!track.isEmpty())
                        currentTune.title = QFileInfo(track).fileName();
                }
                QString artist=QString::fromLocal8Bit(rawInfo.artist);
                if (!artist.isEmpty())
                    currentTune.artist = artist;
                QString album=QString::fromLocal8Bit(rawInfo.album);
                if (!album.isEmpty())
                    currentTune.source = album;
                QString playtime=QString::fromLocal8Bit(rawInfo.playtime);
                if (!playtime.isEmpty())
                {
                    QTime time=QTime::fromString(playtime, "m:ss");
                    currentTune.length = time.minute()*60+time.second();
                }
            }
        }
        DosClose(pipeHandle);
    }

    // Common code
    if (FPreviousTune != currentTune)
    {
        FPreviousTune = currentTune;
        if (currentTune.isEmpty())
            emit stopped();
        else
            emit playing(currentTune);
    }
}

Q_EXPORT_PLUGIN2(plg_tunelistenerz, TuneListenerZ)

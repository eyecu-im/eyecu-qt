#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include "tunelistenerquplayer.h"
#include "tunelistenerquplayeroptions.h"
#include "utils/qt4qt5compat.h"

/**
 * \class TuneListenerQuPlayer
 * \brief Currently playing tune listener for QuPlayer.
 */

TuneListenerQuPlayer::TuneListenerQuPlayer():FOptionsManager(NULL)
{}

TuneListenerQuPlayer::~TuneListenerQuPlayer()
{}

void TuneListenerQuPlayer::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener QuPlayer");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from QuPlayer");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerQuPlayer::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
    return true;
}

bool TuneListenerQuPlayer::initObjects()
{
    return true;
}

bool TuneListenerQuPlayer::initSettings()
{
    Options::setDefaultValue(OPV_TUNE_LISTENER_QUPLAYER_PIPENAME, "\\PIPE\\QU2");
    if (FOptionsManager)
        FOptionsManager->insertOptionsHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> TuneListenerQuPlayer::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_PEP)
		widgets.insertMulti(OWO_PEP_USERTUNE_QUPLAYER, new TuneListenerQuPlayerOptions(AParent));
    return widgets;
}

TuneData TuneListenerQuPlayer::currentTune() const
{
    return FPreviousTune;
}

void TuneListenerQuPlayer::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_TUNE_LISTENER_QUPLAYER_PIPENAME));
}

void TuneListenerQuPlayer::onOptionsClosed()
{}

void TuneListenerQuPlayer::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_TUNE_LISTENER_QUPLAYER_PIPENAME)
        FPipeName=ANode.value().toString();
}

void TuneListenerQuPlayer::check()
{
    if (FPipeHandle)
    {
        #define PIPESIZE 256
        BYTE    abBuf[PIPESIZE];
        ULONG   ulWritten;

        TuneData currentTune;

		if (!DosWrite(FPipeHandle, "info", 4, &ulWritten))     /* Writes to the pipe    */
        {
            ULONG ulBytesRead;
            if (!DosRead(FPipeHandle, abBuf, sizeof(abBuf), &ulBytesRead))
            {
                QString result=QString::fromLocal8Bit((char *)abBuf, ulBytesRead);
				if (result.length()>1) // Not Empty
                {
                    int dash = result.indexOf(" - ");
                    if (dash != -1)
                    {
                        currentTune.artist = result.left(dash);
                        currentTune.title = result.mid(dash+3);
                    }
                    else
                        currentTune.title = result;
                }
                else
					if (!DosWrite(FPipeHandle, "filename", 8, &ulWritten))     /* Writes to the pipe    */
                        if (!DosRead(FPipeHandle, abBuf, sizeof(abBuf), &ulBytesRead))
                        {
                            result=QString::fromLocal8Bit((char *)abBuf, ulBytesRead);
							if (result.length()>1)
                                currentTune.title = result;
                        }
            }

        }
        else    // Pipe broken
        {
            DosClose(FPipeHandle);
            FPipeHandle=NULLHANDLE;
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
    else
    {
        ULONG ulAction;
		DosOpen((unsigned char *)FPipeName.TOASCII().data(),
                &FPipeHandle,
                &ulAction,
                0,
                FILE_NORMAL,
                FILE_OPEN,
                OPEN_ACCESS_READWRITE |
                OPEN_SHARE_DENYNONE,
                (PEAOP2) NULL);
    }
}

Q_EXPORT_PLUGIN2(plg_tunelistenerquplayer, TuneListenerQuPlayer)

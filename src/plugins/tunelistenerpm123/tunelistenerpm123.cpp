#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include "tunelistenerpm123.h"
#include "tunelistenerpm123options.h"
#include "utils/qt4qt5compat.h"

/**
 * \class TuneListenerPm123
 * \brief Currently playing tune listener for PM123.
 */

TuneListenerPm123::TuneListenerPm123(): FOptionsManager(NULL),
                                        FPlaying(false),
                                        FOldPlayer(false),
                                        FPipeHandle(NULLHANDLE),
                                        FRequest(None)
{}

TuneListenerPm123::~TuneListenerPm123()
{}

void TuneListenerPm123::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener PM123");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from PM123");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerPm123::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));
    return true; //FMessageWidgets!=NULL
}

bool TuneListenerPm123::initObjects()
{
    return true;
}

bool TuneListenerPm123::initSettings()
{
    Options::setDefaultValue(OPV_TUNE_LISTENER_PM123_PIPENAME, "\\PIPE\\PM123");
    if (FOptionsManager)
        FOptionsManager->insertOptionsDialogHolder(this);
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> TuneListenerPm123::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_PEP)
		widgets.insertMulti(OWO_PEP_USERTUNE_PM123, new TuneListenerPm123Options(AParent));
    return widgets;
}

// Methods to be implemented
TuneData TuneListenerPm123::currentTune() const
{
    return FPreviousTune;
}

void TuneListenerPm123::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_TUNE_LISTENER_PM123_PIPENAME));
}

void TuneListenerPm123::onOptionsClosed()
{}

void TuneListenerPm123::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_TUNE_LISTENER_PM123_PIPENAME)
        FPipeName=ANode.value().toString();
}

void TuneListenerPm123::check()
{
    bool  isPlaying=FPlaying;
    bool  isDataReady=false;

    if (FPipeHandle)
    {
        ULONG ulBytesRead;
        ULONG ulPipeState;
        AVAILDATA adBytesAvail;	

        if (!DosPeekNPipe(FPipeHandle, NULL, 0, &ulBytesRead, &adBytesAvail, &ulPipeState) &&
                ulPipeState==NP_STATE_CONNECTED) // At least, pipe exists and connected
        {
            if (adBytesAvail.cbpipe)
            {
                PVOID pbBuf;
                if (!DosAllocMem(&pbBuf, adBytesAvail.cbpipe, PAG_READ|PAG_WRITE|PAG_COMMIT|OBJ_ANY))
                {
                    if (!DosRead(FPipeHandle, pbBuf, adBytesAvail.cbpipe, &ulBytesRead))
                    {
                        QString result=QString::fromLocal8Bit((PCHAR)pbBuf, ulBytesRead);
                        if (!result.isEmpty())
                            result.chop(1);

                        if (!result.isEmpty())
                            switch (FRequest)
                            {
                                case Status:
                                {
                                    FRequest=Pos;
									isPlaying=result.contains("playing");
                                    isDataReady=true;
                                    break;
                                }

                                case Tag:
                                {
                                    QString artist;
                                    int colon = result.indexOf(": ");
                                    if (colon != -1)
                                    {
                                        artist=result.left(colon);
                                        result=result.mid(colon+2);
                                    }

                                    QString album;
                                    QString year;
                                    int doubleDash = result.lastIndexOf(" -- ");
                                    if (doubleDash != -1)
										result.truncate(doubleDash);
                                    if (result.right(1) == ")")
                                    {
										int parenthesis=result.indexOf(" (");
										if (parenthesis != -1)
										{
											QString album_year=result.mid(parenthesis+2, result.length()-parenthesis-3);
											int comma=album_year.lastIndexOf(", ");
											if (comma != -1)
											{
												album=album_year.left(comma);
												year=album_year.mid(comma+2);
											}
											result.truncate(parenthesis);
										}
                                    }
                                    FCurrentTune.title = result;
                                    FCurrentTune.artist = artist;
                                    FCurrentTune.source = album;
                                    if (FOldPlayer)
                                    {
                                        DosClose(FPipeHandle);
                                        FPipeHandle = NULLHANDLE;
                                        isPlaying=true;
                                        isDataReady=true;
                                    }
                                    else
										FRequest=Status;
                                    break;
                                }

                                case Format:
                                {
                                    FCurrentTune.clear();
                                    QStringList splitted=result.split('\n');
                                    for (QStringList::const_iterator it=splitted.constBegin(); it!=splitted.constEnd(); it++)
										if ((*it).startsWith("SONGLENGTH="))
										{
											FCurrentTune.length = (*it).mid(11, (*it).indexOf('.')-11).toInt();
											break;
										}

                                    FRequest=Meta;
                                    break;
                                }

                                case Pos:
                                {
                                    FCurrentTune.clear();
                                    QString length=result.left(result.indexOf(' '));
                                    FCurrentTune.length = length.toInt();
                                    FRequest=Tag;
                                    break;
                                }

                                case Query:
                                {
									isPlaying=result.at(0) == '1';
                                    FRequest=Format;
                                    isDataReady=true;
                                    break;
                                }

                                case Meta:
                                {
                                    QStringList splitted=result.split('\n');
                                    for (QStringList::const_iterator it=splitted.constBegin(); it!=splitted.constEnd(); it++)
                                    {
                                        QStringList pair=(*it).split('=');
                                        if (pair.size()==2)
                                        {
                                            if (pair[0]=="TITLE")
                                                FCurrentTune.title = pair[1];
                                            else if (pair[0]=="ARTIST")
                                                FCurrentTune.artist = pair[1];
                                            else if (pair[0]=="ALBUM")
                                                FCurrentTune.source = pair[1];
                                            else if (pair[0]=="TRACK")
                                                FCurrentTune.track = pair[1];
                                        }
                                    }
                                    FRequest=Query;
                                    break;
                                }

                                default:
                                    break;
                            }
                    }
                    DosFreeMem(pbBuf);
                }
            }
            else
                switch (FRequest)
                {
                    case None:
                    case Status:
                        FRequest=Format;
                        break;

                    case Query:
                    case Meta:
                    case Format:
                        FRequest=Pos;
                        break;

                    case Tag:
                        DosClose(FPipeHandle);
                        FPipeHandle = NULLHANDLE;
                        FOldPlayer = true;
                        break;

                    case Pos:
                        FRequest=Tag;

                    default:
                        break;
                }
        }
        else    // Pipe broken
        {
            isPlaying=false;
            DosClose(FPipeHandle);
            FPipeHandle=NULLHANDLE;
            FOldPlayer=false;
            FRequest=None;
        }
    }

    if (!FPipeHandle)
    {
        ULONG ulAction;
		DosOpen((unsigned char *)FPipeName.toLatin1().data(),
            &FPipeHandle,
            &ulAction,
            0,
            FILE_NORMAL,
            OPEN_ACTION_FAIL_IF_NEW  | OPEN_ACTION_OPEN_IF_EXISTS,
            OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE | OPEN_FLAGS_FAIL_ON_ERROR,
            (PEAOP2) NULL);
    }

    if (FPipeHandle)
    {
        char *request = NULL;
        switch (FRequest)
        {
            case Status:
				request="*status";
				break;

            case Tag:
				request="*status tag";
				break;

            case Format:
				request="*info format";
				break;

            case Pos:
				request="*status pos";
				break;

            case Query:
				request="*query play";
				break;

            case Meta:
				request="*info meta";
				break;

            default:
				break;
        }

        if (request)
        {
            ULONG ulWritten;
			DosWrite(FPipeHandle, request, strlen(request)+1, &ulWritten);
        }
    }

    if (isDataReady )
    {
        if (isPlaying && (!FPlaying || FPreviousTune != FCurrentTune))
            emit playing(FCurrentTune);
        FPreviousTune=FCurrentTune;
    }

    if (!isPlaying && FPlaying)
        emit stopped();

    FPlaying=isPlaying;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tunelistenerpm123, TuneListenerPm123)
#endif

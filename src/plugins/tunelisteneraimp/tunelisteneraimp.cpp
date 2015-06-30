#include "tunelisteneraimp.h"
#include "AIMPSDKCommon.h"
#include "AIMPSDKRemote.h"

/**
 * \class TuneListenerAimp
 * \brief Currently playing tune listener for AIMP.
 */

const WCHAR* TuneListenerAimp::AIMP_REMOTE_CLASS = (WCHAR *)L"AIMP2_RemoteInfo";

TuneListenerAimp::TuneListenerAimp(): FTuneSent(false)
{}

TuneListenerAimp::~TuneListenerAimp()
{}

void TuneListenerAimp::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener AIMP");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from AIMP");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerAimp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)
	Q_UNUSED(AInitOrder)

    return true; //FMessageWidgets!=NULL
}

bool TuneListenerAimp::initObjects()
{
    return true;
}

bool TuneListenerAimp::initSettings()
{    
    return true;
}

// Methods to be implemented
TuneData TuneListenerAimp::currentTune() const
{
    return FCurrentTune;
}

TuneData TuneListenerAimp::getTune() const
{
    HANDLE aFile=OpenFileMapping(FILE_MAP_READ, TRUE, AIMP_REMOTE_CLASS);
    TAIMPFileInfo *aInfo=(TAIMPFileInfo *)MapViewOfFile(aFile, FILE_MAP_READ, 0, 0, AIMPRemoteAccessMapFileSize);
    if (aInfo != NULL)
    {
        wchar_t *str = (wchar_t *)((char*)aInfo + aInfo->StructSize);
        QString album = QString::fromWCharArray(str, aInfo->AlbumLength);
        str += aInfo->AlbumLength;
        QString artist = QString::fromWCharArray(str, aInfo->ArtistLength);
        str += aInfo->ArtistLength + aInfo->DateLength;
        QString fileName = QString::fromWCharArray(str, aInfo->FileNameLength);
        str += aInfo->FileNameLength + aInfo->GenreLength;
        QString title = QString::fromWCharArray(str, aInfo->TitleLength);
        quint32 trackNumber = aInfo->TrackNumber;
        quint32 time = aInfo->Duration;
        quint32 rating = aInfo->Rating;

        TuneData tune;
        if (!fileName.isEmpty())
        {
            if (!title.isEmpty())
                tune.title = title;
            else
            {
                int index = fileName.replace("/", "\\").lastIndexOf("\\");
                if (index > 0)
                {
                    QString filename = fileName.right(fileName.length()-index-1);
                    index = filename.lastIndexOf(".");
                    title = (index > 0) ? filename.left(index) : filename;
                }
                else
                    title = fileName;
                tune.title = title;
            }
            if (trackNumber > 0)
                tune.track = QString::number(trackNumber);
            if (rating > 0)
                tune.rating = rating*2;
            if (time > 0)
                tune.length = time/1000;
            if (!artist.isEmpty())
                tune.artist = artist;
            if (!album.isEmpty())
                tune.source = album;
        }
        return tune;
    }
    UnmapViewOfFile(aInfo);
    CloseHandle(aFile);
    return TuneData();
}

HWND TuneListenerAimp::findAimp() const
{
    return FindWindow(AIMP_REMOTE_CLASS, AIMP_REMOTE_CLASS);
}

TuneListenerAimp::PlayerStatus TuneListenerAimp::getAimpStatus(const HWND &AHwnd) const
{
    if (AHwnd)
        return (PlayerStatus)SendMessage(AHwnd, WM_AIMP_PROPERTY, AIMP_RA_PROPERTY_PLAYER_STATE | AIMP_RA_PROPVALUE_GET, 0);
    return STOPPED;
}

void TuneListenerAimp::sendTune(const TuneData &ATune)
{
    if ((ATune != FCurrentTune) && !ATune.isEmpty())
    {
        FCurrentTune = ATune;
        FTuneSent = true;
    }
}

void TuneListenerAimp::clearTune()
{
    if (FTuneSent)
    {
        FCurrentTune.clear();
        FTuneSent = false;
    }
}

void TuneListenerAimp::check()
{
    HWND aimp = findAimp();
    if (getAimpStatus(aimp) == PLAYING)
        sendTune(getTune());
    else
        clearTune();

// Common code
    TuneData tune = currentTune();
    if (FPrevTune != tune)
    {
        FPrevTune = tune;
        if (tune.isEmpty())
            emit stopped();
        else
            emit playing(tune);
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tunelisteneraimp, TuneListenerAimp)
#endif

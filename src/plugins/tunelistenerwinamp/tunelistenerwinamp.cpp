#include <QTimer>
#include <qt_windows.h>

#ifdef Q_CC_MSVC
#pragma warning(push)
#pragma warning(disable: 4100)
#endif

// this file generates eight C4100 warnings, when compiled with MSVC2003
#include "wa_ipc.h"

#ifdef Q_CC_MSVC
#pragma warning(pop)
#endif

#include "tunelistenerwinamp.h"

template <typename char_type> const size_t length (const char_type * begin)
{
    const char_type * end = begin;
    for (; *end; ++end);
    return end - begin;
}

/**
 * \class TuneListenerWinAmp
 * \brief Currently playing tune listener for WinAmp.
 */

static const int NormInterval = 3000;
static const int AntiscrollInterval = 100;

TuneListenerWinamp::TuneListenerWinamp(): FAntiscrollCounter(0)
{}

TuneListenerWinamp::~TuneListenerWinamp()
{}

void TuneListenerWinamp::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener WINAMP");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from WINAMP");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerWinamp::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(APluginManager)
	Q_UNUSED(AInitOrder)

    return true; //FMessageWidgets!=NULL
}

bool TuneListenerWinamp::initObjects()
{
    return true;
}

bool TuneListenerWinamp::initSettings()
{    
    return true;
}

// Methods to be implemented
TuneData TuneListenerWinamp::currentTune() const
{
    return FCurrentTune;
}

TuneData TuneListenerWinamp::getTune(const HWND &AHwnd)
{
    TuneData tune;
    int position = (int)SendMessage(AHwnd, WM_WA_IPC, 0, IPC_GETLISTPOS);
    if (position != -1)
    {
        if (AHwnd && SendMessage(AHwnd,WM_WA_IPC,0,IPC_ISPLAYING) == 1)
        {
            QPair<bool, QString> trackpair(getTrackTitle(AHwnd));
            if (!trackpair.first)
            {
                // getTrackTitle wants us to retry in a few ms...
                int interval = AntiscrollInterval;
                if (++FAntiscrollCounter > 10)
                {
                    FAntiscrollCounter = 0;
                    interval = NormInterval;
                }
                QTimer::singleShot(interval, this, SLOT(check()));
                return tune;
            }
            FAntiscrollCounter = 0;
            QStringList numRest=trackpair.second.split(". ", QString::SkipEmptyParts);
            if (numRest.size()>1)
            {
                QStringList artistTitle=numRest[1].split(" - ", QString::SkipEmptyParts);
                if (artistTitle.size()>1)
                {
                    tune.artist = artistTitle.at(0);
                    tune.title = artistTitle.at(1);
                }
                tune.track = QString::number(position + 1);
            }
            tune.length = SendMessage(AHwnd, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);
        }
    }
    return tune;
}

QPair<bool, QString> TuneListenerWinamp::getTrackTitle(const HWND &AHwnd) const
{
    TCHAR waTitle[2048];
    QString title;

    // Get WinAmp window title. It always contains name of the track
    SendMessage (AHwnd, WM_GETTEXT, static_cast<WPARAM> (sizeof (waTitle) / sizeof (waTitle[0])), reinterpret_cast<LPARAM> (waTitle));
    // Now, waTitle contains WinAmp window title
    title = QString ((const QChar *) waTitle, length<TCHAR> ((const TCHAR *) waTitle));
    if (title[0] == '*' || (title.length () && title[title.length() - 1] == '*'))
        return QPair<bool, QString>(false, QString()); // request to be called again soon.

    // Check whether there is a need to do the all stuff
    if (!title.length())
        return QPair<bool, QString>(true,title);

    QString winamp (" - Winamp ***");
    int winampLength = winamp.length();

    // Is title scrolling on the taskbar enabled?
    title += title + title;
    int waLast = title.indexOf (winamp, -1);
    if (waLast != -1)
    {
        if (title.length())
            title.remove (waLast, title.length () - waLast);
        int waFirst;
        while ((waFirst = title.indexOf (winamp)) != -1)
            title.remove (0, waFirst + winampLength);
    }
    else
        title = QString ((const QChar *) waTitle, length<TCHAR> ((const TCHAR *) waTitle)); // Title is not scrolling

    // Remove leading and trailing spaces
    title  = title.trimmed();

    // Remove trailing " - Winamp" from title
    if (title.length ())
    {
        winamp = " - Winamp";
        winampLength = winamp.length ();
        int waFirst = title.indexOf (winamp);
        if (waFirst != -1)
            title.remove (waFirst, waFirst + winampLength);
    }

    // Remove track number from title
    if (title.length ())
    {
        QString dot(". ");
        int dotFirst = title.indexOf (dot);
        if (dotFirst != -1)
        {
            // All symbols before the dot are digits?
            bool allDigits = true;
            for (int pos = dotFirst; pos > 0; --pos)
                allDigits = allDigits && title[pos].isNumber();
            if (allDigits)
                title.remove(0, dotFirst + dot.length ());
        }
    }

    // Remove leading and trailing spaces
    if (title.length ())
    {
        while (title.length () && title[0] == ' ')
            title.remove (0, 1);
        while (title.length () && title[title.length () - 1] == ' ')
            title.remove (title.length () - 1, 1);
    }

    return QPair<bool, QString>(true,title);
}

void TuneListenerWinamp::check()
{
    TuneData tune;
#ifdef UNICODE
    HWND h = FindWindow(L"Winamp v1.x", NULL);
#else
    HWND h = FindWindow("Winamp v1.x", NULL);
#endif
    if (h && SendMessage(h, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
        tune = getTune(h);
    FCurrentTune = tune;

// Common code
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
Q_EXPORT_PLUGIN2(plg_tunelistenerwinamp, TuneListenerWinamp)
#endif

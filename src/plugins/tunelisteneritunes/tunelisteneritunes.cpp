#include <utils/logger.h>

#include "tunelisteneritunes.h"

/**
 * \class TuneListenerITunes
 * \brief A controller for the Mac OS X version of iTunes.
 */

QString TuneListenerITunes::CFStringToQString(CFStringRef AString)
{
    QString result;

    if (AString != NULL)
    {
        CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(AString), kCFStringEncodingUTF8) + 1;
        char* buffer = new char[length];
        if (CFStringGetCString(AString, buffer, length, kCFStringEncodingUTF8))
            result = QString::fromUtf8(buffer);
        else
			LOG_WARNING("itunesplayer.cpp: CFString conversion failed.");
        delete[] buffer;
    }
    return result;
}

TuneListenerITunes::TuneListenerITunes()
{
    // TODO: Poll iTunes for current playing tune
    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterAddObserver(center, this, ITunesController::iTunesCallback, CFSTR("com.apple.iTunes.playerInfo"), NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
}

TuneListenerITunes::~TuneListenerITunes()
{
    CFNotificationCenterRef center = CFNotificationCenterGetDistributedCenter();
    CFNotificationCenterRemoveObserver(center, this, CFSTR("com.apple.iTunes.playerInfo"), NULL);
}

void TuneListenerITunes::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener iTunes");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from iTunes");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerITunes::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    return true; //FMessageWidgets!=NULL
}

bool TuneListenerITunes::initObjects()
{
    return true;
}

bool TuneListenerITunes::initSettings()
{    
    return true;
}

// Methods to be implemented
TuneData TuneListenerITunes::currentTune() const
{
    return FCurrentTune;
}

void TuneListenerITunes::iTunesCallback(CFNotificationCenterRef, void *, CFStringRef, const void *, CFDictionaryRef AInfo)
{
    TuneData tune;
    ITunesController* controller = (ITunesController*) observer;

    CFStringRef cf_state = (CFStringRef) CFDictionaryGetValue(AInfo, CFSTR("Player State"));
    if (CFStringCompare(cf_state,CFSTR("Paused"),0) == kCFCompareEqualTo)
        emit controller->stopped();
    else if (CFStringCompare(cf_state,CFSTR("Stopped"),0) == kCFCompareEqualTo)
        emit controller->stopped();
    else if (CFStringCompare(cf_state,CFSTR("Playing"),0) == kCFCompareEqualTo)
    {
        tune.artist = CFStringToQString((CFStringRef) CFDictionaryGetValue(AInfo, CFSTR("Artist")));
        tune.title = CFStringToQString((CFStringRef) CFDictionaryGetValue(AInfo, CFSTR("Name")));
        tune.source = CFStringToQString((CFStringRef) CFDictionaryGetValue(AInfo, CFSTR("Album")));

        CFNumberRef cf_track = (CFNumberRef) CFDictionaryGetValue(AInfo, CFSTR("Track Number"));
        if (cf_track)
        {
            int tracknr;
            if (!CFNumberGetValue(cf_track,kCFNumberIntType,&tracknr))
				LOG_WARNING("itunesplayer.cpp: Number value conversion failed.");
            tune.track = QString::number(tracknr);
        }

        CFNumberRef cf_time = (CFNumberRef) CFDictionaryGetValue(AInfo, CFSTR("Total Time"));
        int time = 0;
        if (cf_time && !CFNumberGetValue(cf_time,kCFNumberIntType,&time))
			LOG_WARNING("itunesplayer.cpp: Number value conversion failed.");
        tune.length = time/1000;
        controller->currentTune_ = tune;
        emit controller->playing(tune);
    }
    else
		LOG_WARNING("itunesplayer.cpp: Unknown state.");
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tunelisteneritunes, TuneListenerITunes)
#endif

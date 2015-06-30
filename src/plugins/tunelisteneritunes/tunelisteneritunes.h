#ifndef TUNELISTENERITUNES_H
#define TUNELISTENERITUNES_H

#include <CoreFoundation/CoreFoundation.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>

#define TUNELISTENERITUNES_UUID "{bda2481c-6fcf-de26-9b31-da97c206bad1}"

class TuneListenerITunes: public QObject,
                          public IPlugin,
                          public ITuneListener
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITuneListenerITunes")
#endif
public:
    TuneListenerITunes();
    ~TuneListenerITunes();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERITUNES_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //ITuneListener
    virtual TuneData currentTune() const;
    virtual bool isPollingType() const {return false;}

protected:
    static void iTunesCallback(CFNotificationCenterRef, void*, CFStringRef, const void*, CFDictionaryRef AInfo);
    static QString CFStringToQString(CFStringRef AString);

signals:
    void playing(const TuneData &ATuneData);
    void stopped();

private:
    TuneData FCurrentTune;
};

#endif // TUNELISTENERITUNES_H

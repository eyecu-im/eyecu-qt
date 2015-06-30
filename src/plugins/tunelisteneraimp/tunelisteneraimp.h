#ifndef TUNELISTENERAIMP_H
#define TUNELISTENERAIMP_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>
#include <windows.h>

#define TUNELISTENERAIMP_UUID "{640fcba2-c762-22ac-c8db-23bc4adb302e}"

class TuneListenerAimp: public QObject,
            public IPlugin,
            public ITuneListener
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITuneListenerAimp")
#endif
    enum PlayerStatus
    {
        STOPPED = 0,
        PLAYING = 2
    };

public:
    TuneListenerAimp();
    ~TuneListenerAimp();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERAIMP_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //ITuneListener
    virtual TuneData currentTune() const;
    virtual bool isPollingType() const {return true;}

protected:
    TuneData getTune() const;
    HWND findAimp() const;
    PlayerStatus getAimpStatus(const HWND &AHwnd) const;
    void sendTune(const TuneData &ATune);
    void clearTune();

protected slots:
    // Common code
    virtual void check();

signals:
    void playing(const TuneData &ATuneData);
    void stopped();

private:
    static const WCHAR* AIMP_REMOTE_CLASS;
    TuneData FCurrentTune;
    bool     FTuneSent;
    // Common code
    TuneData FPrevTune;
};

#endif // TUNELISTENERAIMP_H

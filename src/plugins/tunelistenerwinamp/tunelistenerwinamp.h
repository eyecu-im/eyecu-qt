#ifndef TUNELISTENERWINAMP_H
#define TUNELISTENERWINAMP_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>

#define TUNELISTENERWINAMP_UUID "{dbf85304-2acf-ba38-4b50-cb18a207c4e2}"

class TuneListenerWinamp: public QObject,
            public IPlugin,
            public ITuneListener
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITuneListenerWinamp")
#endif
public:
    TuneListenerWinamp();
    ~TuneListenerWinamp();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERWINAMP_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //ITuneListener
    virtual TuneData currentTune() const;
    virtual bool isPollingType() const {return true;}

protected:
    TuneData getTune(const HWND &AHwnd);
    QPair<bool, QString> getTrackTitle(const HWND &AHwnd) const;

protected slots:
    virtual void check();

signals:
    void playing(const TuneData &ATuneData);
    void stopped();

private:
    TuneData    FPreviousTune;
    TuneData    FCurrentTune;
    int         FAntiscrollCounter;
};

#endif // TUNELISTENERWINAMP_H

#ifndef TUNELISTENERPM123_H
#define TUNELISTENERPM123_H

#include <utils/options.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>
#include <interfaces/ioptionsmanager.h>

#include <os2.h>

#define TUNELISTENERPM123_UUID "{842bd6a0-d2ac-9bf1-da8e-13cb7daf50c9}"

class TuneListenerPm123: public QObject,
						 public IPlugin,
						 public ITuneListener,
						 public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.TuneListenerPm123")
#endif
    enum Request
    {
        None,
        Status,
        Tag,
        Pos,
        Format,
        Query,
		Meta
    };

public:
    TuneListenerPm123();
    ~TuneListenerPm123();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERPM123_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
    virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //ITuneListener
    virtual TuneData currentTune() const;
    virtual bool isPollingType() const {return true;}

protected slots:
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);
    // Common code
    virtual void check();

signals:
    void playing(const TuneData &ATuneData);
    void stopped();

private:
    IOptionsManager *FOptionsManager;
    QString         FPipeName;
    TuneData        FCurrentTune;
    bool            FPlaying;
	bool			FOldPlayer;
    HPIPE           FPipeHandle;
    Request         FRequest;    
    TuneData        FPreviousTune;
};

#endif // TUNELISTENERPM123_H

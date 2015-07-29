#ifndef TUNELISTENERQUPLAYER_H
#define TUNELISTENERQUPLAYER_H

#include <utils/options.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>
#include <interfaces/ioptionsmanager.h>

#include <os2.h>

#define TUNELISTENERQUPLAYER_UUID "{bcf35d8a-4b29-c71c-08a1-c920b536da24}"

class TuneListenerQuPlayer: public QObject,
			public IPlugin,
			public ITuneListener,
			public IOptionsDialogHolder
{
	Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener IOptionsDialogHolder)
    
public:
    TuneListenerQuPlayer();
    ~TuneListenerQuPlayer();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERQUPLAYER_UUID; }
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
    HPIPE           FPipeHandle;

    // Common code
    TuneData        FPreviousTune;
};

#endif // TUNELISTENERQUPLAYER_H

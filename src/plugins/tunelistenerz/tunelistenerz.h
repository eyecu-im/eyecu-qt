#ifndef TUNELISTENERZ_H
#define TUNELISTENERZ_H

#include <utils/options.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>
#include <interfaces/ioptionsmanager.h>
#include <os2.h>

#define TUNELISTENERZ_UUID "{dac26b8a-0c54-d19b-875a-bd96c430da2b}"

struct RawInfo
{
    char fname[256], songinfo[256];
    char track[128], artist[128], album[128], comment[128];
    char year[6], playtime[10], timenow[10], genre[26], filesize[10];
};

class TuneListenerZ: public QObject,
					 public IPlugin,
					 public ITuneListener,
					 public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener IOptionsDialogHolder)
    
public:
    TuneListenerZ();
    ~TuneListenerZ();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERZ_UUID; }
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

    // Common code
    TuneData        FPreviousTune;
};

#endif // TUNELISTENERZ_H

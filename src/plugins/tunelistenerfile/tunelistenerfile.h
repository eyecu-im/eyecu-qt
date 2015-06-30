#ifndef TUNELISTENERFILE_H
#define TUNELISTENERFILE_H

#include <utils/options.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>
#include <interfaces/ioptionsmanager.h>

#define TUNELISTENERFILE_UUID "{dba76f3c-2b40-c12b-9176-bd53d088bc72}"

class TuneListenerFile: public QObject,
            public IPlugin,
            public ITuneListener,
			public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITuneListenerFile")
#endif
public:
    enum FileFormat
    {
        Plain,
        XML
    };

    TuneListenerFile();
    ~TuneListenerFile();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERFILE_UUID; }
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
    QString         FFileName;
    FileFormat      FFileFormat;
    TuneData        FCurrentTune;
    bool            FWaitForCreated;

    // Common code
    TuneData        FPreviousTune;
};

#endif // TUNELISTENERFILE_H

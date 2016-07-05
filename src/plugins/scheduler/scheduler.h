#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>

#include <utils/jid.h>

#define SCHEDULER_UUID "{3F202B41-24C9-A8B1-9C8D-B6F3794A5B67}"

class Scheduler: public QObject,
				 public IPlugin,
				 public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IScheduler")
#endif
public:
    Scheduler();
    ~Scheduler();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return SCHEDULER_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
	// IOptionsDialogHolder interface
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

protected slots:
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);

private:
	IOptionsManager *FOptionsManager;
};

#endif // SCHEDULER_H

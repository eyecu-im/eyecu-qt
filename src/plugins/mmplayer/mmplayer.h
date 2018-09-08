#ifndef MMPLAYER_H
#define MMPLAYER_H

#include <FifoDataBuffer>

#include <interfaces/imainwindow.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/immplayer.h>

#include <utils/iconstorage.h>

#include "playerwindow.h"

class MmPlayer:
    public QObject,
    public IPlugin,
    public IMmPlayer,
	public ITuneListener,
	public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMmPlayer ITuneListener IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMmPlayer")
#endif
public:
    MmPlayer();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MMPLAYER_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

	//ITuneListener
	TuneData currentTune() const {return FPlayerWindow?FPlayerWindow->currentTune():TuneData();}
	bool isPollingType() const {return false;}

protected slots:
    void onStreamCreated(IFileStream *AStream);
    void onFileStreamStateChanged();
	void onPlayerStatusChanged(int AStatusNew, int AstatusOld);
    void onPlayerDestroyed();
    void onShowOptions() const;
	void onStartPlayer(bool AState);
	void onPlayerWindowDestroyed();

signals:
	//ITuneListener
	void playing(const TuneData &ATuneData);
	void stopped();

private:
	IOptionsManager		*FOptionsManager;
	IMainWindowPlugin	*FMainWindowPlugin;
	PlayerWindow		*FPlayerWindow;
	Action				*FAction;
    QMap<IFileStream*, FifoDataBuffer*> FBuffers;
    QMap<QObject*, FifoDataBuffer*> FPlayerBuffers;
};

#endif // MMPLAYER_H

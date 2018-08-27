#include <QFileDialog>
#if QT_VERSION < 0x050000
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

#include <interfaces/ifilestreamsmanager.h>

#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionvalues.h>
#include <definitions/actiongroups.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/menuicons.h>
#include <definitions/toolbargroups.h>

#include <utils/shortcuts.h>
#include <utils/logger.h>

#include "mmplayer.h"
#include "multimediaplayeroptions.h"

MmPlayer::MmPlayer():
    FOptionsManager(NULL),
    FMainWindowPlugin(NULL),
	FPlayerWindow(NULL)
{}

//-----------------------------
void MmPlayer::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Multimedia player");
	APluginInfo->description = tr("Allows to play multimedia streams during file transfer");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
}

bool MmPlayer::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder)

    IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
    if (plugin)
        FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IFileStreamsManager").value(0,NULL);
    if (plugin)
        connect(plugin->instance(),SIGNAL(streamCreated(IFileStream*)),SLOT(onStreamCreated(IFileStream*)));

    return true;
}

bool MmPlayer::initObjects()
{
    Shortcuts::declareGroup(SCTG_MMPLAYER, tr("Multimedia player"), SGO_MMPLAYER);
	Shortcuts::declareShortcut(SCT_MMPLAYER_SHOW, tr("Show/Hide player window"), tr("F9", "Show/Hide"), Shortcuts::ApplicationShortcut);
	Shortcuts::declareShortcut(SCT_MMPLAYER_OPEN, tr("Open/Stop"), tr("Return", "Open/Stop"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MMPLAYER_PLAY, tr("Pause/Play"), tr("Space", "Pause/Play"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MMPLAYER_MUTE, tr("Mute/Unmute"), tr("M", "Mute/Unmute"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MMPLAYER_VOLUMEUP, tr("Volume up"), tr("Up", "Volume up"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MMPLAYER_VOLUMEDOWN, tr("Volume down"), tr("Down", "Volume down"), Shortcuts::WindowShortcut);
    Shortcuts::declareShortcut(SCT_MMPLAYER_OPTIONS, tr("Show options dialog"), tr("Ctrl+O", "Show options dialog"), Shortcuts::WindowShortcut);

	if (FMainWindowPlugin)
	{
		FAction = new Action(this);
		FAction->setText(tr("Multimedia player"));
		FAction->setIcon(RSR_STORAGE_MENUICONS, MNI_MMPLAYER);
		FAction->setCheckable(true);
		FAction->setShortcutId(SCT_MMPLAYER_SHOW);
		connect(FAction,SIGNAL(triggered(bool)),SLOT(onStartPlayer(bool)));
		FMainWindowPlugin->mainWindow()->topToolBarChanger()->insertAction(FAction, TBG_MWTTB_MMPLAYER);
	}
    return true;
}

bool MmPlayer::initSettings()
{
	QString mmLocation;
#if QT_VERSION >= 0x050000
	QStringList dirList = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
	if (dirList.isEmpty())
	{
		dirList = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
		if (dirList.isEmpty())
		{
			dirList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
			if (dirList.isEmpty())
			{
				dirList = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
				if (dirList.isEmpty())
					dirList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
			}
		}
	}
	if (!dirList.isEmpty())
		mmLocation = dirList.first();
#else
	mmLocation = QDesktopServices::storageLocation(QDesktopServices::MoviesLocation);
	if (mmLocation.isEmpty())
		mmLocation = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
	if (mmLocation.isEmpty())
		mmLocation = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
	if (mmLocation.isEmpty())
		mmLocation = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
	if (mmLocation.isEmpty())
		mmLocation = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    Options::setDefaultValue(OPV_MMPLAYER_SHOW, true);
    Options::setDefaultValue(OPV_MMPLAYER_VOLUME, 100);
    Options::setDefaultValue(OPV_MMPLAYER_MUTE, false);
    Options::setDefaultValue(OPV_MMPLAYER_ASPECTRATIOMODE, Qt::KeepAspectRatio);
    Options::setDefaultValue(OPV_MMPLAYER_SMOOTHRESIZE, false);
	Options::setDefaultValue(OPV_MMPLAYER_DIRECTORY, QDir::fromNativeSeparators(mmLocation));
	Options::setDefaultValue(OPV_MMPLAYER_FILTER, QString());

    if (FOptionsManager)	// for page in nastroikah
    {
		IOptionsDialogNode dnode = {ONO_MMPLAYER, OPN_MMPLAYER, MNI_MMPLAYER, tr("Multimedia player")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> MmPlayer::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
    if (FOptionsManager)
    {
		if (ANodeId == OPN_DATATRANSFER)
			widgets.insertMulti(OWO_MEDIAFILES_PLAY, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MMPLAYER_SHOW), tr("Play multimedia data during file transfer"), AParent));
        else if (ANodeId == OPN_MMPLAYER)
            widgets.insertMulti(OWO_MMPLAYER, new MultimediaPlayerOptions(AParent));
    }
    return widgets;
}

void MmPlayer::onShowOptions() const
{
    FOptionsManager->showOptionsDialog(OPN_MMPLAYER);
}

void MmPlayer::onStartPlayer(bool AState)
{
	if(AState)
	{
		if(FPlayerWindow==NULL)
		{
			FPlayerWindow = new PlayerWindow(this);
			connect(FPlayerWindow,SIGNAL(destroyed()),SLOT(onPlayerWindowDestroyed()));
			if (FOptionsManager)
				connect(FPlayerWindow,SIGNAL(showOptions()),SLOT(onShowOptions()));
		}
	}
	else
		FPlayerWindow->close();
}

void MmPlayer::onPlayerWindowDestroyed()
{
	FAction->setChecked(false);
	disconnect(FPlayerWindow,SIGNAL(destroyed()),this,SLOT(onPlayerWindowDestroyed()));
	FPlayerWindow=NULL;
}

void MmPlayer::onStreamCreated(IFileStream *AStream)
{
	LOG_DEBUG("MmPlayer::onStreamCreated()");
	if(AStream->streamKind()==IFileStream::ReceiveFile && Options::node(OPV_MMPLAYER_SHOW).value().toBool())
        connect(AStream->instance(),SIGNAL(stateChanged()),SLOT(onFileStreamStateChanged()));
}

void MmPlayer::onFileStreamStateChanged()
{
	LOG_DEBUG("MmPlayer::onFileStreamStateChanged()");
	IFileStream *stream = qobject_cast<IFileStream *>(sender());
	LOG_DEBUG(QString("Stream state: %1").arg(stream->streamState()));

	switch (stream->streamState())
	{		
		case IFileStream::Transfering:
        {
			LOG_DEBUG("Transferring");
            FifoDataBuffer *buffer=new FifoDataBuffer(0, this);
            if (buffer->input()->open(QIODevice::ReadOnly))
            {
				MediaPlayer *mediaStreamer=new MediaPlayer(QAudioDeviceInfo::defaultOutputDevice(), buffer->input(), this);
                if(mediaStreamer)
                    if(buffer->output()->open(QIODevice::WriteOnly))
                    {
                        stream->addOutputDevice(buffer->output());
                        FBuffers.insert(stream, buffer);
						FStreamerBuffers.insert(mediaStreamer, buffer);
						connect(mediaStreamer,SIGNAL(statusChanged(int,int)),SLOT(onMediaStreamStatusChanged(int,int)));
						connect(mediaStreamer,SIGNAL(destroyed()),SLOT(onMediaStreamerDestroyed()));
						mediaStreamer->setVolume(Options::node(OPV_MMPLAYER_MUTE).value().toBool()?0:Options::node(OPV_MMPLAYER_VOLUME).value().toInt());
						mediaStreamer->setStatus(MediaPlayer::Running);
                    }
            }
            else
				LOG_WARNING("MmPlayer::onFileStreamStateChanged(): device open failed!");

            if (!FBuffers.contains(stream))
                buffer->deleteLater();
			break;
		}

		case IFileStream::Finished:
		case IFileStream::Aborted:
		{
			LOG_DEBUG("Finished or Aborted");
			FifoDataBuffer *buffer = FBuffers.value(stream);
			if (buffer)
			{
				if (!FStreamerBuffers.key(buffer))
					FBuffers.remove(stream);
				buffer->output()->close();
			}
			break;
		}
	}
}

void MmPlayer::onMediaStreamStatusChanged(int AStatusNew, int AstatusOld)
{
	Q_UNUSED(AstatusOld)
	LOG_DEBUG(QString("MmPlayer::onMediaStreamStatusChanged(%1, %2").arg(AStatusNew).arg(AstatusOld));

	MediaPlayer *player = qobject_cast<MediaPlayer *>(sender());
	switch (AStatusNew)
    {
		case MediaPlayer::Running:
        {
			LOG_DEBUG("Running");
			IFileStream *fileStream = FBuffers.key(FStreamerBuffers[sender()]);
			if (fileStream)
			{
				PlayerWindow *window = new PlayerWindow(this, player, QFileInfo(fileStream->fileName()).fileName(), fileStream->fileSize());
				if (FOptionsManager)
					connect(window, SIGNAL(showOptions()), SLOT(onShowOptions()));
			}
            break;
        }

		case MediaPlayer::Error:
		case MediaPlayer::Finished:
        {
			LOG_DEBUG("Error or Finished");
            FifoDataBuffer *buffer=FStreamerBuffers.take(sender());
            if (buffer)
            {
                IFileStream *fileStream=FBuffers.key(buffer);
                if (fileStream)
                {
                    fileStream->removeOutputDevice(buffer->output());
                    FBuffers.remove(fileStream);
                }
                buffer->deleteLater();
			}
			sender()->deleteLater();
			emit stopped();
            break;
        }

        default:
            return;
    }
	disconnect(sender(),SIGNAL(statusChanged(int,int)),this,SLOT(onMediaStreamStatusChanged(int,int)));
}

void MmPlayer::onMediaStreamerDestroyed()
{
	LOG_DEBUG("MmPlayer::onMediaStreamerDestroyed()");
    FifoDataBuffer *buffer=FStreamerBuffers.take(sender());
    if (buffer)
    {
        IFileStream *stream=FBuffers.key(buffer);
        if (stream)
        {
            stream->removeOutputDevice(buffer->output());
            FBuffers.remove(stream);
        }
        buffer->deleteLater();
    }
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_MmPlayer, MmPlayer)
#endif

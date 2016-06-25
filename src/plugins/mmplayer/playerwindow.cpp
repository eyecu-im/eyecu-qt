#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <definitions/optionvalues.h>
#include <definitions/shortcuts.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/version.h>
#include <utils/iconstorage.h>
#include <utils/shortcuts.h>
#include <utils/logger.h>

#include "playerwindow.h"
#include "ui_playerwindow.h"

#define PLAY_WITH_DELETE	-1

// Constructors
/**
 * @brief PlayerWindow::PlayerWindow constructor, used when IMediaStreamer is started already
 * @param AMediaStreamer a started IMediaStreamer, which plays multimedia
 * @param AFileName a file name (without path), used to display in window title
 * @param AFileLenth a file length used to calculate timeline slider position
 * @param parent parent widget
 */

PlayerWindow::PlayerWindow(QObject *APlugin, MediaPlayer *AMediaStreamer, QString AFileName, qint64 AFileLenth, QWidget *AParent) :
	QWidget(AParent),
    FMediaStreamer(AMediaStreamer),
	FFile(NULL),
    ui(new Ui::PlayerWindow)
{
	LOG_DEBUG(QString("PlayerWindow(Streamer, %1").arg(AFileName));
    ui->setupUi(this);
	ui->btnOpen->hide();

	init(APlugin);

	calculate(AFileName, AFileLenth);

	connect(FMediaStreamer,SIGNAL(statusChanged(int,int)),SLOT(onStreamerStatusChanged(int,int)));

    show();
	LOG_DEBUG("PlayerWindow created");
}

/**
 * @brief PlayerWindow::PlayerWindow PlayerWindow::PlayerWindow an overloaded constructor, used to create a standalone player with no media loaded
 * @param AFFmpeg FFMpeg plugin
 * @param parent parent widget
 */

PlayerWindow::PlayerWindow(QObject *APlugin, QWidget *AParent):
	QWidget(AParent),
    FMediaStreamer(NULL),    
	FFile(NULL),
	FSupportedAudioFormats("*.asf *.au *.flac *.mka *.mp2 *.mp3 *.mpa *.ogg *.snd *.wav *.wma"),
	FSupportedVideoFormats("*.3gp *.avi *.flv *.flc *.fli *.mov *.mpg *.mpe *.mpeg *.mp4 *.mkv *.ogm *.dat *.wmv"),
    ui(new Ui::PlayerWindow)
{
	LOG_DEBUG("PlayerWindow(FFMpeg)");
	setAcceptDrops(true);
	QString tmp=FSupportedAudioFormats;
	tmp.append(FSupportedVideoFormats);
	tmp.remove("*.");		
	FMidiFormats << tmp.split(" ");

    ui->setupUi(this);
    setWindowTitle(tr("Multimedia player - %1").arg(CLIENT_NAME));

	ui->btnOpen->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MMPLAYER_EJECT));
	ui->btnOpen->setToolTip(tr("Open..."));

	init(APlugin);

	ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	ui->btnPlay->setToolTip(tr("Play"));
	ui->btnPlay->setDisabled(true);

    show();
	LOG_DEBUG("PlayerWindow created");
}

// Destructor
/**
 * @brief PlayerWindow::~PlayerWindow destructor
 */

PlayerWindow::~PlayerWindow()
{
	LOG_DEBUG("~PlayerWindow()");
	delete ui;
	LOG_DEBUG("PlayerWindow destroyed");
}

const TuneData &PlayerWindow::currentTune() const
{
	return FCurrentTune;
}

/**
 * @brief PlayerWindow::init method to initialize PlayerWindow, used by both constructor overloads
 */

void PlayerWindow::init(QObject *AParent)
{
	LOG_DEBUG("PlayerWindow::init()");
	setWindowIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MMPLAYER));

	bool    mute    = Options::node(OPV_MMPLAYER_MUTE).value().toBool();
	int     volume  = mute? 0 : Options::node(OPV_MMPLAYER_VOLUME).value().toInt();

	ui->btnOptions->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_OPTIONS_DIALOG));
	ui->btnMute->setIcon(style()->standardIcon(mute ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
	ui->btnMute->setChecked(mute);
	ui->sldVolume->setValue(volume);
	ui->lcdVolume->display(volume);
	ui->sldVolume->setDisabled(true);

	Shortcuts::bindObjectShortcut(SCT_MMPLAYER_OPEN, ui->btnOpen);
	Shortcuts::bindObjectShortcut(SCT_MMPLAYER_MUTE, ui->btnMute);
    Shortcuts::bindObjectShortcut(SCT_MMPLAYER_PLAY, ui->btnPlay);
    Shortcuts::bindObjectShortcut(SCT_MMPLAYER_OPTIONS, ui->btnOptions);
    Shortcuts::insertWidgetShortcut(SCT_MMPLAYER_VOLUMEUP, this);
    Shortcuts::insertWidgetShortcut(SCT_MMPLAYER_VOLUMEDOWN, this);

	connect(ui->btnMute,SIGNAL(toggled(bool)),SLOT(onMuteToggled(bool)));
	connect(ui->sldVolume,SIGNAL(valueChanged(int)),SLOT(onVolumeSliderValueChanged(int)));
	connect(ui->btnPlay,SIGNAL(clicked()),SLOT(onPlayClicked()));
	connect(ui->btnOpen,SIGNAL(clicked()),SLOT(onOpenClicked()));	
	connect(ui->btnOptions,SIGNAL(clicked()),SIGNAL(showOptions()));
	connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onVolumeShortcut(QString,QWidget*)));
    connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)), SLOT(onOptionsChanged(OptionsNode)));
	connect(this, SIGNAL(playing(TuneData)), AParent, SIGNAL(playing(TuneData)));
	connect(this, SIGNAL(stopped()), AParent, SIGNAL(stopped()));
	LOG_DEBUG("PlayerWindow::init(): finished");
}

/**
 * @brief PlayerWindow::calculate method used to calculate correct media data
 */

void PlayerWindow::calculate(const QString &AFileName, int AFileLength)
{
	LOG_DEBUG(QString("calculate(%1, %2)").arg(AFileName).arg(AFileLength));

	// Set window title
	QHash<QString, QString> metadata = FMediaStreamer->getMetadata();
	QString name;
	if (metadata.contains("title"))
	{
		QString artist;
		if (metadata.contains("artist"))
			artist = metadata.value("artist");
		else if (metadata.contains("album_artist"))
			artist = metadata.value("album_artist");
		if (metadata.contains("album"))
			if (artist.isEmpty())
				name=tr("%1 (%3)").arg(metadata.value("title")).arg(metadata.value("album"));
			else
				name=tr("%1 - %2 (%3)").arg(artist).arg(metadata.value("title")).arg(metadata.value("album"));
		else
			if (artist.isEmpty())
				name=tr("%1").arg(metadata.value("title"));
			else
				name=tr("%1 - %2").arg(artist).arg(metadata.value("title"));

	}
	else
		name = AFileName;

	TuneData tune;
	if (metadata.contains("title"))
		tune.title = metadata.value("title");
	else
		tune.title = AFileName;
	if (metadata.contains("artist"))
		tune.artist=metadata.value("artist");
	else if (metadata.contains("album_artist"))
		tune.artist=metadata.value("album_artist");
	if (metadata.contains("track"))
		tune.track=metadata.value("track");
	if (metadata.contains("album"))
		tune.source=metadata.value("album");

	if (!AFileName.isEmpty())
		setWindowTitle(tr("%1 - Multimedia player - %2", "%1: Track name; %2: Application name")
			.arg(name).arg(CLIENT_NAME));
	else
		setWindowTitle(tr("Multimedia player - %1").arg(CLIENT_NAME));

	// Calculate duration
	qint64 duration = FMediaStreamer->duration();
	if (duration <= 0)
	{
		int bitRate = FMediaStreamer->bitRate();
		duration = bitRate? (qint64)AFileLength*8000000/bitRate : -1;
	}
	if (duration >= 0)
	{
		ui->sldPosition->setRange(0, duration/1000000);    // Seconds
		MicroTime microTime(duration);
		ui->lblDuration->setText(microTime.toString(microTime.hour()>0?"h:mm:ss":"mm:ss"));
		tune.length = duration;
	}

	emit playing(FCurrentTune = tune);

	// Initialize video widget
	QWidget *videoWidget = FMediaStreamer->getVideoWidget();
	if(videoWidget)
	{
		FMediaStreamer->setSmoothResize(Options::node(OPV_MMPLAYER_SMOOTHRESIZE).value().toBool());
		FMediaStreamer->setAspectRatioMode((Qt::AspectRatioMode)Options::node(OPV_MMPLAYER_ASPECTRATIOMODE).value().toInt());
		videoWidget->setSizePolicy(QSizePolicy());
		ui->verticalLayout->insertWidget(0, videoWidget, 1);
		videoWidget->setCursor(Qt::PointingHandCursor);
		connect(videoWidget,SIGNAL(mouseClicked()),SLOT(onPlayClicked()));
		setMaximumSize(65536,65536);
	}
	else
		setMaximumSize(65536,80);

	// Initialize volume/mute controls
	if (FMediaStreamer->volumeAvailable())  // Add Volume and Mute controls, if volume available
	{
		bool    mute    = Options::node(OPV_MMPLAYER_MUTE).value().toBool();
		int     volume  = mute? 0 : Options::node(OPV_MMPLAYER_VOLUME).value().toInt();

		Shortcuts::bindObjectShortcut(SCT_MMPLAYER_MUTE, ui->btnMute);
		ui->btnMute->setIcon(style()->standardIcon(mute ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
		ui->btnMute->setChecked(mute);
		ui->sldVolume->setValue(volume);
		ui->lcdVolume->display(volume);
		ui->sldVolume->setEnabled(true);
		connect(ui->btnMute,SIGNAL(toggled(bool)),SLOT(onMuteToggled(bool)));
		connect(ui->sldVolume,SIGNAL(valueChanged(int)),SLOT(onVolumeSliderValueChanged(int)));
	}
	else
	{
		ui->btnMute->setDisabled(true);
		ui->sldVolume->setDisabled(true);
	}

	// Enable "Pause/Play" button
	ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	ui->btnPlay->setToolTip(tr("Pause"));
	ui->btnPlay->setEnabled(true);

	connect(FMediaStreamer,SIGNAL(playTimeChanged(qint64)),SLOT(onPlayTimeChanged(qint64)));

	LOG_DEBUG("PlayerWindow::calculate(): finished");
}

void PlayerWindow::start()
{
	LOG_DEBUG("start()");
	LOG_INFO(QString("File name: %1").arg(FFile->fileName()));
	if(FFile->open(QFile::ReadOnly))
	{
		FMediaStreamer = new MediaPlayer(QAudioDeviceInfo::defaultOutputDevice(), FFile, this);
		LOG_DEBUG("Media Streamer created!");
		connect(FMediaStreamer, SIGNAL(statusChanged(int,int)),SLOT(onStreamerStatusChanged(int,int)));
		FMediaStreamer->setVolume(Options::node(OPV_MMPLAYER_MUTE).value().toBool()?0:Options::node(OPV_MMPLAYER_VOLUME).value().toInt());
		FMediaStreamer->setStatus(MediaPlayer::Running);
	}
	LOG_DEBUG("start(): finished");
}

// QWidget events
void PlayerWindow::closeEvent(QCloseEvent *AEvent)
{
	LOG_DEBUG("closeEvent()");
	if (FFile)
		FFile->close();
    if(FMediaStreamer)
		FMediaStreamer->setStatus(MediaPlayer::Finished);
    AEvent->accept();
	LOG_DEBUG("closeEvent(): finished");
}

void PlayerWindow::wheelEvent(QWheelEvent *AEvent)
{
    ui->sldVolume->setValue(ui->sldVolume->value() + AEvent->delta()/40);
    AEvent->accept();
}

void PlayerWindow::resizeEvent(QResizeEvent *AEvent)
{
    Q_UNUSED(AEvent)
    if (!ui->verticalLayout->itemAt(0)->sizeHint().isEmpty())
    {
        QWidget *videoWidget = ui->verticalLayout->itemAt(0)->widget();
		if (videoWidget && videoWidget->sizePolicy() == QSizePolicy())
			videoWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    }
}

void PlayerWindow::hideEvent(QHideEvent *AEvent)
{
	if (!AEvent->spontaneous() && !FMediaStreamer)
		deleteLater();
}

void PlayerWindow::dropEvent(QDropEvent *event)
{
	LOG_DEBUG("dropEvent()");
	QApplication::processEvents();
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        QList <QUrl> inpFiles=mimeData->urls();
        QStringList playList;
		playList.clear();
		//! list filter url's
        while(!inpFiles.isEmpty())
        {
			QString fileName=inpFiles.first().toLocalFile();
			if(fileName.contains("."))
            {
				QStringList spl= fileName.split(".");
                QString curFmt = spl[spl.size()-1];
				if(FMidiFormats.contains(curFmt,Qt::CaseInsensitive))
					playList << fileName;
            }
            else{
				QDir dir(fileName);
                if (dir.exists())
                {
                    QStringList files=dir.entryList(QDir::Files | QDir::Dirs,QDir::Name);
                    files.removeOne(".");
                    files.removeOne("..");
					for(int i=0;i<files.size();i++)
						inpFiles << QUrl().fromLocalFile(QString("%1/%2").arg(dir.path()).arg(files.at(i)));
                }
            }
			inpFiles.removeFirst();
        }
        if(!playList.isEmpty())
            onDropClicked(playList);
//        event->acceptProposedAction();
		}
	LOG_DEBUG("dropEvent(): finished");
}

void PlayerWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()){
        event->accept();
    }
    else
        event->ignore();
}

void PlayerWindow::dragMoveEvent(QDragMoveEvent *event)
{
Q_UNUSED(event)
}

void PlayerWindow::dragLeaveEvent(QDragLeaveEvent *AEvent)
{
    AEvent->accept();
}

// SLOTS
void PlayerWindow::onStreamerStatusChanged(int AStatusNew, int AStatusOld)
{
	LOG_DEBUG(QString("onStreamerStatusChanged(%1, %2)").arg(AStatusNew).arg(AStatusOld));
	switch (AStatusNew)
	{
		case MediaPlayer::Running:
			LOG_DEBUG("Running");
			if (AStatusOld == MediaPlayer::Opened)	// Stream just started
			{
				calculate(QFileInfo(FFile->fileName()).fileName(), FFile->size());
				resize(sizeHint());
			}
			else
				emit playing(FCurrentTune);
			// Enable "Open/Stop" button
			setupControls(MediaPlayer::Running);
			break;

		case MediaPlayer::Paused:
			LOG_DEBUG("Paused");
			setupControls(MediaPlayer::Paused);
			emit stopped();
			break;

		case MediaPlayer::Error:
			QMessageBox::critical(this, tr("File open error!"), tr("An error occured while opening \"%1\"").arg(QFileInfo(*FFile).fileName()), QMessageBox::Ok);

		case MediaPlayer::Finished:
			LOG_DEBUG("Error or Finished");
			emit stopped();
			if (FFile)
			{
				FFile->close();
				if (AStatusNew == MediaPlayer::Error)
					FFile=NULL;
			}
			if(FMediaStreamer)
			{
				FMediaStreamer->deleteLater();
				FMediaStreamer=NULL;
				LOG_DEBUG("Media Streamer destroyed!");
			}
			if (isHidden())		// Player windows closed
				deleteLater();
			else if (ui->btnOpen->isVisible()) // Update controls
			{
				if(ui->verticalLayout->itemAt(0)->widget() != 0)
					delete ui->verticalLayout->itemAt(0)->widget();
				setupControls(AStatusNew);
				if(!FPlayList.isEmpty())
					newTrackPlay(PLAY_WITH_DELETE);
			}
			break;
	}
	LOG_DEBUG("onStreamerStatusChanged(): finished");
}

void PlayerWindow::setupControls(int AState)
{
	switch (AState)
	{
		case MediaPlayer::Running:
			ui->btnOpen->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
			ui->btnOpen->setToolTip(tr("Stop"));
			ui->btnOpen->setEnabled(true);
			ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
			ui->btnPlay->setToolTip(tr("Pause"));
			ui->btnPlay->setEnabled(true);
			break;
		case MediaPlayer::Paused:
			ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
			ui->btnPlay->setToolTip(tr("Play"));
			ui->btnPlay->setEnabled(true);
			break;
		case MediaPlayer::Error:
		case MediaPlayer::Finished:
			ui->btnOpen->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MMPLAYER_EJECT));
			ui->btnOpen->setToolTip(tr("Open..."));
			ui->btnOpen->setEnabled(true);
			ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
			ui->btnPlay->setToolTip(tr("Play"));
			ui->btnPlay->setEnabled(AState == MediaPlayer::Finished);
			ui->sldVolume->setDisabled(true);
			ui->lcdVolume->setDisabled(true);
			break;
		default:
			break;
	}
}

void PlayerWindow::onOpenClicked()
{	
	if(FMediaStreamer)
	{
		ui->btnOpen->setDisabled(true);
		ui->btnPlay->setDisabled(true);
		FPlayList.clear();
		FMediaStreamer->setStatus(MediaPlayer::Finished);
	}
	else
	{
		QString filter = Options::node(OPV_MMPLAYER_FILTER).value().toString();
        FPlayList = QFileDialog::getOpenFileNames(this, tr("Select file"), Options::node(OPV_MMPLAYER_DIRECTORY).value().toString(),
            tr("All Supported Formats") + QString(" (%1 %2);;").arg(FSupportedAudioFormats).arg(FSupportedVideoFormats) +
            tr("Audio") + QString(" (%1);;").arg(FSupportedAudioFormats) +
            tr("Video") + QString(" (%1);;").arg(FSupportedVideoFormats) +
            tr("All Files")+" *;;", &filter);
		if(!FPlayList.isEmpty())
		{
			Options::node(OPV_MMPLAYER_FILTER).setValue(filter);
			FPlayList.sort();
			newTrackPlay(PLAY_WITH_DELETE);
		}
    }
}

void PlayerWindow::onDropClicked(QStringList APlayList)
{
	LOG_DEBUG("onDropClicked()");
    if(FMediaStreamer) {
        FPlayList.append(APlayList);
        FPlayList.removeDuplicates();
        FPlayList.sort();
        ui->lcdNbr->display(FPlayList.size());
    }
    else{
        QString filter = Options::node(OPV_MMPLAYER_FILTER).value().toString();
		FPlayList.clear();
		FPlayList << APlayList;
        if(!FPlayList.isEmpty())
        {
            Options::node(OPV_MMPLAYER_FILTER).setValue(filter);
            FPlayList.sort();
            newTrackPlay(PLAY_WITH_DELETE);
        }
    }
	LOG_DEBUG("onDropClicked(): finished");
}

void PlayerWindow::newTrackPlay(int ANumber)
{
	LOG_DEBUG(QString("newTrackPlay(%1)").arg(ANumber));
	if(FPlayList.isEmpty())
		return;
	QString fileName;
    if(ANumber==PLAY_WITH_DELETE){
        fileName=FPlayList.takeAt(0);
        ui->lcdNbr->display(FPlayList.size());
    }
	else if(ANumber>=0 || ANumber<FPlayList.size())
		fileName=FPlayList[ANumber];
	else
		return;

	if (!fileName.isEmpty())
	{
		if (FFile)
		{
			FFile->deleteLater(); FFile=NULL;
		}
		FFile = new QFile(fileName);
			Options::node(OPV_MMPLAYER_DIRECTORY).setValue(QFileInfo(*FFile).filePath());
			start();
	}
	LOG_DEBUG("newTrackPlay(): finished");
}

void PlayerWindow::onPlayClicked()
{
	LOG_DEBUG("onPlayClicked()");
	ui->btnPlay->setDisabled(true);
	if (FMediaStreamer)
		switch (FMediaStreamer->status())
		{
			case MediaPlayer::Running:
				LOG_DEBUG("Runing");
				FMediaStreamer->setStatus(MediaPlayer::Paused);
				break;
	
			case MediaPlayer::Paused:
				LOG_DEBUG("Paused");
				FMediaStreamer->setStatus(MediaPlayer::Running);
				break;
	
			default:
				break;
		}
	else
		if (FFile)
			start();
	LOG_DEBUG("onPlayClicked(): finished!");
}

void PlayerWindow::onMuteToggled(bool AMute)
{
    Options::node(OPV_MMPLAYER_MUTE).setValue(AMute);
}

void PlayerWindow::onVolumeSliderValueChanged(int AVolume)
{
    Options::node(OPV_MMPLAYER_VOLUME).setValue(AVolume);
}

void PlayerWindow::onPlayTimeChanged(qint64 APlayTime)
{
    QTime   time;
    if (ui->sldPosition->isVisible())
    {
        ui->sldPosition->setValue(APlayTime/1000000);
        time.addSecs(ui->sldPosition->maximum());
    }
    else
        time.addSecs(APlayTime/1000000);
    ui->lblCurrentTime->setText(MicroTime(APlayTime).toString(time.hour()?"h:mm:ss":"m:ss"));
}

void PlayerWindow::onVolumeShortcut(const QString &AShortcutId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AShortcutId == SCT_MMPLAYER_VOLUMEUP)
        ui->sldVolume->setValue(ui->sldVolume->value() + 3);
    else if (AShortcutId == SCT_MMPLAYER_VOLUMEDOWN)
        ui->sldVolume->setValue(ui->sldVolume->value() - 3);
}

void PlayerWindow::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path() == OPV_MMPLAYER_VOLUME)
    {
        int volume = ANode.value().toInt();
        ui->lcdVolume->display(volume);
        FMediaStreamer->setVolume(volume);
    }
    else if (ANode.path() == OPV_MMPLAYER_MUTE)
    {
        if(ANode.value().toBool())
        {
            ui->btnMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
            ui->btnMute->setToolTip(tr("Unmute"));
            ui->sldVolume->blockSignals(true);
            ui->sldVolume->setValue(0);
            ui->sldVolume->setDisabled(true);
            ui->sldVolume->blockSignals(false);
            ui->lcdVolume->display(0);
			if(FMediaStreamer)
				FMediaStreamer->setVolume(0);
        }
        else
        {
            int volume = Options::node(OPV_MMPLAYER_VOLUME).value().toInt();
            ui->btnMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
            ui->btnMute->setToolTip(tr("Mute"));
            ui->sldVolume->blockSignals(true);
            ui->sldVolume->setValue(volume);
            ui->sldVolume->setEnabled(true);
            ui->sldVolume->blockSignals(false);
            ui->lcdVolume->display(volume);
			if(FMediaStreamer)
				FMediaStreamer->setVolume(volume);
        }
    }
    else if (ANode.path() == OPV_MMPLAYER_SMOOTHRESIZE)
        FMediaStreamer->setSmoothResize(ANode.value().toBool());
    else if (ANode.path() == OPV_MMPLAYER_ASPECTRATIOMODE)
		FMediaStreamer->setAspectRatioMode((Qt::AspectRatioMode)ANode.value().toInt());
}

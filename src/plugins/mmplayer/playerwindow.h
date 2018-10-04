#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QFile>
#include <QCloseEvent>
#include <QWheelEvent>
#include <QHideEvent>
#include <MediaPlayer>

#include <interfaces/ifilestreamsmanager.h>
#include <interfaces/itune.h>
#include <utils/microtime.h>

namespace Ui {
class PlayerWindow;
}

class PlayerWindow: public QWidget
{
    Q_OBJECT

public:
	PlayerWindow(QObject *APlugin, MediaPlayer *AMediaStreamer, QString AFileName, qint64 ALenth, QWidget *AParent = 0);
	PlayerWindow(QObject *APlugin, QWidget *AParent = 0);
    //qint64 duration
    ~PlayerWindow();
	const TuneData &currentTune() const;

public slots:
    void onOptionsChanged(const OptionsNode &ANode);

protected:
	void init(QObject *AParent);
	void calculate(const QString &AFileName, int AFileLength);
	void start();
	void setupControls(int AState);
	void newTrackPlay(int ANumber);

	// QWidget interface
	void hideEvent(QHideEvent *AEvent);
    void closeEvent(QCloseEvent *AEvent);
    void wheelEvent(QWheelEvent *AEvent);
    void resizeEvent(QResizeEvent *AEvent);
    // drag & drop
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *AEvent);

protected slots:
	void onPlayClicked();
	void onOpenClicked();
    void onDropClicked(QStringList APlayList);
    void onMuteToggled(bool AMute);
    void onVolumeSliderValueChanged(int AVolume);
    void onPlayTimeChanged(qint64 APlayTime);
    void onVolumeShortcut(const QString &AShortcutId, QWidget *AWidget);
	void onStreamerStatusChanged(int AStatusNew, int AStatusOld);

signals:
    void volume(int AVolume);
	void showOptions() const;
	void playing(const TuneData &ATuneData);
	void stopped();

private:
	MediaPlayer		*FMediaStreamer;
	QFile			*FFile;
	QStringList		FPlayList;
	QStringList		FMidiFormats;
	TuneData		FCurrentTune;
	const QString	FSupportedAudioFormats;
	const QString	FSupportedVideoFormats;
	Ui::PlayerWindow *ui;
};

#endif // PLAYERWINDOW_H

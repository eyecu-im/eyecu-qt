#ifndef AUDIOOPTIONS_H
#define AUDIOOPTIONS_H

#include <QWidget>
#include <QAVCodec>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QpAudioLevel>

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

#include "ui_audiooptions.h"

namespace Ui {
class AudioOptions;
}

class AudioOptions :
    public QWidget,
	public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	explicit AudioOptions(QWidget *parent = nullptr);
	~AudioOptions();
    virtual QWidget* instance() { return this; }
	Ui::AudioOptions *ui;

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected:
	void initializeAudio();
	void createAudioInput();

	void changeEvent(QEvent *e);

protected slots:
    void modify(int);
	void onInputVolumeChanged(int value);
	void onInputDeviceChanged(int index);

private:
	QAudioInput		 *FAudioInput;
	QpAudioLevel	 *FAudioLevel;
	QAudioDeviceInfo FInputDevice;
	QAudioFormat	 FInputFormat;
};

#endif // AUDIOOPTIONS_H

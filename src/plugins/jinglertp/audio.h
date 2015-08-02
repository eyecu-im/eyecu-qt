#ifndef AUDIO_H
#define AUDIO_H

#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <QAudioInput>
#include <utils/iconstorage.h>

#include "audioinfo.h"
#include "renderarea.h"
#include "ui_audio.h"

QAudioFormat GetStreamAudioFormat(void);

namespace Ui {
class Audio;
}

class Audio : public QWidget
{
    Q_OBJECT
public:
    Audio(QWidget *parent = 0);
    ~Audio();
    Ui::Audio *ui;
    int NotifyIntervalMs;

private:
    void initializeWindow();
    void initializeAudio(QAudioDeviceInfo &device);

private slots:
    void onPbSpeaker(bool ch);
    void onPbTest(bool ch);
    void onPbMicrophone(bool ch);

    void deviceChanged(int index);
    void onInputReadyRead();
    void onOutputReady();

private:
    RenderArea  *m_canvas;
    AudioInfo   *audioInfo;
    QAudioInput *progressBar;
    QAudioOutput *audioOutput;
    QAudioInput  *audioInput;

    QIODevice   *audioInputDevice;
    QIODevice   *audioOutputDevice;
    QAudioDeviceInfo m_device;
    IconStorage *FIconStorage;
    QAudioFormat audioFormat;
    QStyle *style;

//    QFile pcmFile;
    bool forTest;

    void stopInputAudio();
    void stopOutputAudio();
    void startInputAudio();
    void startOutputAudio();


};

#endif // AUDIO_H

#include <QDebug>
#include <QStyle>

#include <definitions/resources.h>
#include <definitions/jingleicons.h>

#include "audio.h"

//------Constants-------------------------------------------------------------
const qint64 BufferDurationUs       = 10 * 1000000;
//----------------------------------------------------------------------------

Audio::Audio(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Audio)
    , m_canvas(0)
    , audioInfo(NULL)
    , progressBar(NULL)
    , audioOutput(NULL)
    , audioInput(NULL)
    , audioInputDevice(NULL)
    , audioOutputDevice(NULL)
    , m_device(QAudioDeviceInfo::defaultInputDevice())
//    , NotifyIntervalMs(100)
    , forTest(false)
{
    ui->setupUi(this);
    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_JINGLE);

    style = QApplication::style();
	ui->pbMicrophone->setIcon(FIconStorage->getIcon(JNI_RTP_MIC_OFF));
    //ui->pbSpeaker->setIcon(FIconStorage->getIcon(MNI_JINGLERTP_SPEAKER_ON));
    ui->pbSpeaker->setIcon(style->standardIcon(QStyle::SP_MediaVolumeMuted));
	ui->pbTest->setIcon(FIconStorage->getIcon(JNI_RTP_TEST_OFF));

    ui->pbMicrophone->setCheckable(true);
    ui->pbTest->setCheckable(true);
    ui->pbSpeaker->setCheckable(true);

    connect(ui->pbSpeaker,SIGNAL(clicked(bool)),this,SLOT(onPbSpeaker(bool)));
    connect(ui->pbTest,SIGNAL(clicked(bool)),this,SLOT(onPbTest(bool)));
    connect(ui->pbMicrophone,SIGNAL(clicked(bool)),this,SLOT(onPbMicrophone(bool)));

    audioFormat = GetStreamAudioFormat();
    initializeAudio(m_device);

}

Audio::~Audio()
{
qDebug() << "~Audio()" ;

    audioInput->stop();
    audioOutput->stop();
    disconnect(audioInputDevice,0,this,0);
    disconnect(audioOutputDevice,0,this,0);
    audioInputDevice->close();
    audioOutputDevice->close();

    delete audioInput;
    delete audioOutput;
    delete ui;
}

void Audio::initializeWindow()
{

}

void Audio::initializeAudio(QAudioDeviceInfo &device)
{
    m_canvas = new RenderArea(this);    //---
    m_canvas->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    QHBoxLayout *layout= new QHBoxLayout;
    layout->addWidget(m_canvas);
    ui->LayoutProgress->addLayout(layout,0);

    progressBar = new QAudioInput(device,audioFormat);
    audioInfo   = new AudioInfo(audioFormat);
    connect(audioInfo,SIGNAL(update(qreal)),m_canvas,SLOT(changeLevel(qreal)));
    audioInfo->start();
//------
    audioInput  = new QAudioInput(device,audioFormat,this);  //from microphone
    audioInput->setNotifyInterval(NotifyIntervalMs);

    audioOutput = new QAudioOutput(device,audioFormat,this);  //to speaker
    audioOutput->setNotifyInterval(NotifyIntervalMs);

}

void Audio::onPbSpeaker(bool ch)
{
    if(ch){
        //ui->pbSpeaker->setIcon(FIconStorage->getIcon(MNI_JINGLERTP_SPEAKER_OFF));
        ui->pbSpeaker->setIcon(style->standardIcon(QStyle::SP_MediaVolume));
    }
    else{
         //ui->pbSpeaker->setIcon(FIconStorage->getIcon(MNI_JINGLERTP_SPEAKER_ON));
         ui->pbSpeaker->setIcon(style->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
}

void Audio::onPbTest(bool ch)
{
    forTest=ch;
    if(ch){
		ui->pbTest->setIcon(FIconStorage->getIcon(JNI_RTP_TEST_ON));
    }
    else{
		ui->pbTest->setIcon(FIconStorage->getIcon(JNI_RTP_TEST_OFF));
    }
}

void Audio::onPbMicrophone(bool ch)
{
    if(ch){
        progressBar->start(audioInfo);
        startInputAudio();
        startOutputAudio();
		ui->pbMicrophone->setIcon(FIconStorage->getIcon(JNI_RTP_MIC_ON));
    }
    else{
        progressBar->stop();
//        disconnect(progressBar, 0, this, 0);//--???---
        stopInputAudio();
        stopOutputAudio();
		ui->pbMicrophone->setIcon(FIconStorage->getIcon(JNI_RTP_MIC_OFF));
    }
}

void Audio::deviceChanged(int index)
{
	Q_UNUSED(index)

    stopInputAudio();
    stopOutputAudio();
//    m_device = m_deviceBox->itemData(index).value<QAudioDeviceInfo>();

    audioInput  = new QAudioInput(m_device,audioFormat,this);  //from microphone
    audioInput->setNotifyInterval(NotifyIntervalMs);

    audioOutput  = new QAudioOutput(m_device,audioFormat,this);  //to speaker
    audioOutput->setNotifyInterval(NotifyIntervalMs);
}

void Audio::startInputAudio()
{
    audioInputDevice = audioInput->start();
    connect(audioInputDevice,SIGNAL(readyRead()),this,SLOT(onInputReadyRead()));
}

void Audio::startOutputAudio()
{
    audioOutputDevice = audioOutput->start();
    //connect(audioOutputDevice,SIGNAL(),this,SLOT());
}

//-- if temporare stop microphone and speaker ---
void Audio::stopInputAudio()
{
    if (audioInput) {
        audioInput->stop();
        disconnect(audioInputDevice,0,this,0);
    }
}

void Audio::stopOutputAudio()
{
    if (audioOutput) {
        audioOutput->stop();
        //disconnect(audioOutputDevice,0,this,0);
    }
}

void Audio::onInputReadyRead()
{
    QByteArray buffer;//--this data from microphone to StreamOut--
    buffer.fill(0);
    const qint64 bytesReady = audioInput->bytesReady();
    buffer.resize(bytesReady);
    const qint64 bytesRead = audioInputDevice->read(buffer.data(),bytesReady);

//--to speaker--if test mode--
    if(forTest){
        QByteArray tmp=buffer;
		const qint64 bytesWritten =audioOutputDevice->write(tmp, bytesRead); //--data to speaker
    }
}

void Audio::onOutputReady()
{}

QAudioFormat GetStreamAudioFormat(void)
{
    QAudioFormat format;
#if QT_VERSION >= 0x040700
    format.setSampleRate(8000);
#endif
#if QT_VERSION >= 0x050000
	format.setChannelCount(1);
#else
    format.setChannels(1);
#endif
    format.setSampleSize(16);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);//UnSignedInt
    format.setCodec("audio/pcm");
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format))
    {
        qDebug() << "codecs= " << info.supportedCodecs();
        qDebug() << "default format not supported try to use nearest";
        format = info.nearestFormat(format);
    }
    return format;
}


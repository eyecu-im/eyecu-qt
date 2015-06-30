#include "jinglertpoptions.h"
#include <definitions/optionvalues.h>

JingleRtpOptions::JingleRtpOptions(QWidget *parent):
    QWidget(parent),ui(new Ui::JingleRtpOptions),m_device(QAudioDeviceInfo::defaultInputDevice())
{
    ui->setupUi(this);
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for(int i = 0; i < devices.size(); ++i)
        ui->cbCodec->addItem(devices.at(i).deviceName(), qVariantFromValue(devices.at(i)));

    getSuppSampleRates();

    connect(ui->cbCodec, SIGNAL(activated(int)),SLOT(deviceChanged(int)));
    connect(ui->cbVideoCodec,SIGNAL(activated(int)),this,SLOT(videoDeviceChanged(int)));
    connect(ui->cbInterval,SIGNAL(activated(int)),this,SLOT(modify(int)));

    reset();
}

JingleRtpOptions::~JingleRtpOptions()
{
    delete ui;
}


void JingleRtpOptions::modify(int s)
{
	Q_UNUSED(s)
    emit modified();
}

void JingleRtpOptions::deviceChanged(int index)
{
    m_device = ui->cbCodec->itemData(index).value<QAudioDeviceInfo>();
    getSuppSampleRates();
    emit modified();
}

void JingleRtpOptions::videoDeviceChanged(int index)
{
	Q_UNUSED(index)
    emit modified();
}

void JingleRtpOptions::getSuppSampleRates()
{
    ui->cbSmRate->clear();
    //!-0--setSampleRate,Hz --
	#if QT_VERSION >= 0x040700
        QList<int> suppSampleRates = m_device.supportedSampleRates();
    #else
        QList<int> suppSampleRates = m_device.supportedFrequencies();
    #endif
        qSort(suppSampleRates.begin(),suppSampleRates.end());
    QStringList sampleRate;
    for(int i = 0; i <suppSampleRates.size(); i++ ){
        sampleRate << QString().setNum(suppSampleRates.at(i));
    }
    ui->cbSmRate->addItems(sampleRate);
    //int prefSmRate=m_device.preferredFormat().sampleRate();
    //! save to STORAGE ---
}

void JingleRtpOptions::apply()
{
    Options::node(OPV_JINGLERTP_USERTCP).setValue(ui->cbUseRtcp->isChecked());
    emit childApply();
}

void JingleRtpOptions::reset()
{
    ui->cbUseRtcp->setChecked(Options::node(OPV_JINGLERTP_USERTCP).value().toBool());
    emit childReset();
}

void JingleRtpOptions::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

#include "jinglertpoptions.h"
#include <definitions/optionvalues.h>

JingleRtpOptions::JingleRtpOptions(QWidget *parent):
	QWidget(parent),ui(new Ui::JingleRtpOptions)
{
    ui->setupUi(this);
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		ui->cmbAudioDeviceInput->addItem((*it).deviceName(), qVariantFromValue(*it));

	devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		ui->cmbAudioDeviceOutput->addItem((*it).deviceName(), qVariantFromValue(*it));

	// getSuppSampleRates();

//    connect(ui->cbCodec, SIGNAL(activated(int)),SLOT(deviceChanged(int)));
//    connect(ui->cbVideoCodec,SIGNAL(activated(int)),this,SLOT(videoDeviceChanged(int)));
//    connect(ui->cbInterval,SIGNAL(activated(int)),this,SLOT(modify(int)));

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

void JingleRtpOptions::apply()
{
//	Options::node(OPV_JINGLERTP_USERTCP).setValue(ui->cbUseRtcp->isChecked());
	Options::node(OPV_JINGLE_RTP_AUDIO_INPUT).setValue(QVariant::fromValue(ui->cmbAudioDeviceInput->itemData(ui->cmbAudioDeviceInput->currentIndex())));
	Options::node(OPV_JINGLE_RTP_AUDIO_OUTPUT).setValue(QVariant::fromValue(ui->cmbAudioDeviceOutput->itemData(ui->cmbAudioDeviceOutput->currentIndex())));
    emit childApply();
}

void JingleRtpOptions::reset()
{
	if (ui->cmbAudioDeviceInput->count())
	{
		QVariant name = Options::node(OPV_JINGLE_RTP_AUDIO_INPUT).value();
		int index = ui->cmbAudioDeviceInput->findData(info);
		if (index==-1)
			index = ui->cmbAudioDeviceInput->findData(QVariant::fromValue(QAudioDeviceInfo::defaultInputDevice()));
		ui->cmbAudioDeviceInput->setCurrentIndex(index);
	}

	if (ui->cmbAudioDeviceInput->count())
	{
		QVariant info = Options::node(OPV_JINGLE_RTP_AUDIO_OUTPUT).value();
		int index = ui->cmbAudioDeviceOutput->findData(info);
		if (index==-1)
			index = ui->cmbAudioDeviceOutput->findData(QVariant::fromValue(QAudioDeviceInfo::defaultOutputDevice()));
		ui->cmbAudioDeviceOutput->setCurrentIndex(index);
	}

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

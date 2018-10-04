#include "audiooptions.h"
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>

AudioOptions::AudioOptions(QWidget *parent):
	QWidget(parent),
	ui(new Ui::AudioOptions),
	m_audioInput(nullptr),
	m_audioInfo(nullptr),
	m_device(QAudioDeviceInfo::defaultInputDevice())
{
    ui->setupUi(this);

	ui->audioLevel->setSegments(20);

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		ui->cmbAudioDeviceInput->addItem((*it).deviceName(), qVariantFromValue(*it));

	devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		ui->cmbAudioDeviceOutput->addItem((*it).deviceName(), qVariantFromValue(*it));

	connect(ui->cmbAudioDeviceInput, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->cmbAudioDeviceOutput, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->spbBitrate, SIGNAL(valueChanged(int)), SIGNAL(modified()));

	connect(ui->vcInput, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	connect(ui->vcOutput, SIGNAL(valueChanged(int)), SIGNAL(modified()));

    reset();

	initializeAudio();
}

AudioOptions::~AudioOptions()
{
    delete ui;
}

void AudioOptions::modify(int s)
{
	Q_UNUSED(s)
	emit modified();
}

void AudioOptions::onInputVolumeChanged(int value)
{
	if (m_audioInput)
		m_audioInput->setVolume(value/100.0);
}

void AudioOptions::apply()
{
	if (ui->cmbAudioDeviceInput->count())
	{
		QString deviceName = ui->cmbAudioDeviceInput->currentText();
		if (deviceName == QAudioDeviceInfo::defaultInputDevice().deviceName())
			deviceName.clear();
		Options::node(OPV_JINGLE_RTP_AUDIO_INPUT).setValue(deviceName.isEmpty()?QVariant():QVariant(deviceName));
	}

	if (ui->cmbAudioDeviceOutput->count())
	{
		QString deviceName = ui->cmbAudioDeviceOutput->currentText();
		if (deviceName == QAudioDeviceInfo::defaultOutputDevice().deviceName())
			deviceName.clear();
		Options::node(OPV_JINGLE_RTP_AUDIO_OUTPUT).setValue(deviceName.isEmpty()?QVariant():QVariant(deviceName));
	}

	Options::node(OPV_JINGLE_RTP_AUDIO_BITRATE).setValue(ui->spbBitrate->value());
	Options::node(OPV_JINGLE_RTP_AUDIO_INPUT_VOLUME).setValue(ui->vcInput->value());
	Options::node(OPV_JINGLE_RTP_AUDIO_OUTPUT_VOLUME).setValue(ui->vcOutput->value());

    emit childApply();
}

void AudioOptions::reset()
{
	if (ui->cmbAudioDeviceInput->count())
	{
		QVariant value = Options::node(OPV_JINGLE_RTP_AUDIO_INPUT).value();
		QString name = value.isNull()?QAudioDeviceInfo::defaultInputDevice().deviceName():value.toString();
		int index = ui->cmbAudioDeviceInput->findText(name);
		if (index==-1)
			index = ui->cmbAudioDeviceInput->findText(QAudioDeviceInfo::defaultInputDevice().deviceName());
		ui->cmbAudioDeviceInput->setCurrentIndex(index);
	}

	if (ui->cmbAudioDeviceOutput->count())
	{
		QVariant value = Options::node(OPV_JINGLE_RTP_AUDIO_OUTPUT).value();
		QString name = value.isNull()?QAudioDeviceInfo::defaultOutputDevice().deviceName():value.toString();
		int index = ui->cmbAudioDeviceOutput->findText(name);
		if (index==-1)
			index = ui->cmbAudioDeviceOutput->findText(QAudioDeviceInfo::defaultOutputDevice().deviceName());
		ui->cmbAudioDeviceOutput->setCurrentIndex(index);
	}

	ui->spbBitrate->setValue(Options::node(OPV_JINGLE_RTP_AUDIO_BITRATE).value().toInt());
	ui->vcInput->setValue(Options::node(OPV_JINGLE_RTP_AUDIO_INPUT_VOLUME).value().toInt());
	ui->vcOutput->setValue(Options::node(OPV_JINGLE_RTP_AUDIO_OUTPUT_VOLUME).value().toInt());
	emit childReset();
}

void AudioOptions::initializeAudio()
{
	m_format.setSampleRate(8000);
	m_format.setChannelCount(1);
	m_format.setSampleSize(16);
	m_format.setSampleType(QAudioFormat::SignedInt);
	m_format.setByteOrder(QAudioFormat::LittleEndian);
	m_format.setCodec("audio/pcm");

	if (!m_device.isFormatSupported(m_format))
		m_format = m_device.nearestFormat(m_format);

	m_audioInfo  = new QpAudioLevel(m_format, this);
	connect(m_audioInfo, SIGNAL(levelChanged(qreal)), ui->audioLevel, SLOT(setLevel(qreal)));

	createAudioInput();
}

void AudioOptions::createAudioInput()
{
	m_audioInput = new QAudioInput(m_device, m_format, this);
	m_audioInfo->open(QIODevice::WriteOnly);
	m_audioInput->start(m_audioInfo);
	onInputVolumeChanged(ui->vcInput->value());
}

void AudioOptions::changeEvent(QEvent *e)
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

#include "audiooptions.h"
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>

AudioOptions::AudioOptions(QWidget *parent):
	QWidget(parent),
	ui(new Ui::AudioOptions),
	FAudioInput(nullptr),
	FAudioLevel(nullptr),
	FInputDevice(QAudioDeviceInfo::defaultInputDevice())
{
    ui->setupUi(this);

	ui->audioLevel->setSegments(20);

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	ui->cmbAudioDeviceInput->blockSignals(true);
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
	ui->cmbAudioDeviceInput->blockSignals(false);

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
	FAudioLevel->setVolume(quint8(value));
}

void AudioOptions::onInputDeviceChanged(int index)
{
	FAudioLevel->close();
	FAudioInput->stop();
	delete FAudioInput;

	FInputDevice = ui->cmbAudioDeviceInput->itemData(index).value<QAudioDeviceInfo>();
	createAudioInput();
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
	FInputFormat.setSampleRate(8000);
	FInputFormat.setChannelCount(1);
	FInputFormat.setSampleSize(16);
	FInputFormat.setSampleType(QAudioFormat::SignedInt);
	FInputFormat.setByteOrder(QAudioFormat::LittleEndian);
	FInputFormat.setCodec("audio/pcm");

	if (!FInputDevice.isFormatSupported(FInputFormat))
		FInputFormat = FInputDevice.nearestFormat(FInputFormat);

	FAudioLevel  = new QpAudioLevel(FInputFormat, this);
	connect(FAudioLevel, SIGNAL(levelChanged(qreal)), ui->audioLevel, SLOT(setLevel(qreal)));
	connect(ui->vcInput, SIGNAL(valueChanged(int)), SLOT(onInputVolumeChanged(int)));

	createAudioInput();
}

void AudioOptions::createAudioInput()
{
	FAudioInput = new QAudioInput(FInputDevice, FInputFormat, this);
	FAudioLevel->open(QIODevice::WriteOnly);
	FAudioInput->start(FAudioLevel);
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

#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include "payloadtypeeditdialog.h"
#include "ui_payloadtypeeditdialog.h"

PayloadTypeEditDialog::PayloadTypeEditDialog(const QAVP &APayloadType, const QList<QAVP> &APayloadTypes, QWidget *AParent) :
	QDialog(AParent),
	ui(new Ui::PayloadTypeEditDialog),
	FPayloadType(APayloadType),
	FPayloadTypes(APayloadTypes)
{
	ui->setupUi(this);
	ui->cmbChannels->addItem(tr("Mono"), 1);
	ui->cmbChannels->addItem(tr("Stereo"), 2);
	ui->cmbChannels->addItem(tr("Undefined"), 0);

	ui->cmbMediaType->addItem(tr("Audio"), QAVP::Audio);
//	Video is not implemented yet
//	ui->cmbMediaType->addItem(tr("Video"), QAVP::Video);
	if (APayloadType.isValid())
	{
		ui->cmbMediaType->setCurrentIndex(ui->cmbMediaType->findData(APayloadType.mediaType));
		ui->cmbCodecName->setCurrentIndex(ui->cmbCodecName->findData(QAVCodec::idByName(APayloadType.codecName)));
		if (APayloadType.clockRate)
		{
			if (ui->cmbSampleRate->isEditable())
				ui->cmbSampleRate->setEditText(QString::number(APayloadType.clockRate));
			else
				ui->cmbSampleRate->setCurrentIndex(ui->cmbSampleRate->findData(APayloadType.clockRate));
		}
		ui->cmbChannels->setCurrentIndex(ui->cmbChannels->findData(APayloadType.channels));
	}
}


PayloadTypeEditDialog::~PayloadTypeEditDialog()
{
	delete ui;
}

QAVP PayloadTypeEditDialog::payloadType() const
{
	return QAVP(-1,
				QAVCodec::nameById(ui->cmbCodecName->itemData(ui->cmbCodecName->currentIndex()).toInt()),
				(QAVP::MediaType)ui->cmbMediaType->itemData(ui->cmbMediaType->currentIndex()).toInt(),
				ui->cmbSampleRate->currentText().toInt(),
				ui->cmbChannels->itemData(ui->cmbChannels->currentIndex()).toInt());
}

void PayloadTypeEditDialog::accept()
{
	if (FPayloadTypes.contains(payloadType()))
		QMessageBox::critical(this, tr("Error"), tr("Payload type exists alredy"), QMessageBox::Ok);
	else
		QDialog::accept();
}

void PayloadTypeEditDialog::onMediaTypeSelected(int AIndex)
{
	qDebug() << "PayloadTypeEditDialog::onMediaTypeSelected(" << AIndex << ")";
	QAVCodec::MediaType mediaType;
	switch ((QAVP::MediaType)ui->cmbMediaType->itemData(AIndex).toInt())
	{
		case QAVP::Audio:
			mediaType = QAVCodec::MT_Audio;
			break;
		case QAVP::Video:
			mediaType = QAVCodec::MT_Video;
			break;
		default:
			mediaType = QAVCodec::MT_Unknown;
	}

	ui->cmbCodecName->clear();
	const QStringList codecNames = QAVCodec::codecNames(true);
	for (QStringList::ConstIterator it = codecNames.constBegin(); it != codecNames.constEnd(); ++it)
	{
		int id = QAVCodec::idByName(*it);
		if (QAVCodec::findDecoder(id).type()==mediaType)
		ui->cmbCodecName->addItem(*it, id);
	}
}

void PayloadTypeEditDialog::onCodecSelected(int AIndex)
{
	qDebug() << "PayloadTypeEditDialog::onCodecSelected(" << AIndex << ")";
	QAVCodec codec = QAVCodec::findEncoder(ui->cmbCodecName->itemData(AIndex).toInt());
	QList<int> sampleRates = codec.supportedSampleRates();
	ui->cmbSampleRate->clear();
	for (QList<int>::ConstIterator it = sampleRates.constBegin(); it!=sampleRates.constEnd(); ++it)
		ui->cmbSampleRate->addItem(QString::number(*it), *it);
	ui->cmbSampleRate->setEditable(sampleRates.isEmpty());
}

void PayloadTypeEditDialog::onSettingsChanged()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(payloadType()==FPayloadType);
}

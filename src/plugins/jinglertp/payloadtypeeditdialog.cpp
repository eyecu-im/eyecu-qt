#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include "payloadtypeeditdialog.h"
#include "ui_payloadtypeeditdialog.h"

PayloadTypeEditDialog::PayloadTypeEditDialog(const PayloadType &APayloadType, const QList<PayloadType> &APayloadTypes, QWidget *AParent) :
	QDialog(AParent),
	ui(new Ui::PayloadTypeEditDialog),
	FPayloadType(APayloadType),
	FPayloadTypes(APayloadTypes)
{
	ui->setupUi(this);
	ui->cmbChannels->addItem(tr("Mono"), 1);
	ui->cmbChannels->addItem(tr("Stereo"), 2);
	ui->cmbChannels->addItem(tr("Undefined"), 0);

	ui->cmbMedia->addItem(tr("Audio"), PayloadType::Audio);
//	Video is not implemented yet
//	ui->cmbMediaType->addItem(tr("Video"), QAVP::Video);
	if (APayloadType.isValid())
	{
		ui->cmbMedia->setCurrentIndex(ui->cmbMedia->findData(APayloadType.media));
		ui->cmbName->setCurrentIndex(ui->cmbName->findData(QAVCodec::idByName(APayloadType.name)));
		if (APayloadType.clockrate)
		{
			if (ui->cmbSampleRate->isEditable())
				ui->cmbSampleRate->setEditText(QString::number(APayloadType.clockrate));
			else
				ui->cmbSampleRate->setCurrentIndex(ui->cmbSampleRate->findData(APayloadType.clockrate));
		}
		ui->cmbChannels->setCurrentIndex(ui->cmbChannels->findData(APayloadType.channels));
	}
}


PayloadTypeEditDialog::~PayloadTypeEditDialog()
{
	delete ui;
}

PayloadType PayloadTypeEditDialog::payloadType() const
{
	return PayloadType(-1,
				QAVCodec::nameById(ui->cmbName->itemData(ui->cmbName->currentIndex()).toInt()),
				(PayloadType::MediaType)ui->cmbMedia->itemData(ui->cmbMedia->currentIndex()).toInt(),
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
	switch ((PayloadType::MediaType)ui->cmbMedia->itemData(AIndex).toInt())
	{
		case PayloadType::Audio:
			mediaType = QAVCodec::MT_Audio;
			break;
		case PayloadType::Video:
			mediaType = QAVCodec::MT_Video;
			break;
		default:
			mediaType = QAVCodec::MT_Unknown;
	}

	ui->cmbName->clear();
	const QStringList codecNames = QAVCodec::codecNames(true);
	for (QStringList::ConstIterator it = codecNames.constBegin(); it != codecNames.constEnd(); ++it)
	{
		int id = QAVCodec::idByName(*it);
		QAVCodec decoder(QAVCodec::findDecoder(id));
		if (decoder.type()==mediaType)
			ui->cmbName->addItem(QString("%1: %2").arg(*it).arg(decoder.longName()), id);
	}
}

void PayloadTypeEditDialog::onCodecSelected(int AIndex)
{
	qDebug() << "PayloadTypeEditDialog::onCodecSelected(" << AIndex << ")";
	QAVCodec codec = QAVCodec::findEncoder(ui->cmbName->itemData(AIndex).toInt());
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

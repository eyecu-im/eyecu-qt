#include "jinglertp.h"
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>

QTreeWidgetItem *JingleRtpOptions::itemFromAvp(const QAVP &AAvp)
{
	QTreeWidgetItem *item = new QTreeWidgetItem();
	if (AAvp.payloadType>=0)
		item->setText(0, QString::number(AAvp.payloadType));
	item->setText(1, AAvp.codecName);
	item->setText(2, QString::number(AAvp.clockRate));
	item->setText(3, QString::number(AAvp.channels));
	item->setText(4, tr(AAvp.mediaType==QAVP::Audio?"Audio":
						AAvp.mediaType==QAVP::Video?"Video":
						AAvp.mediaType==QAVP::Both?"Both":"Unknown"));
	return item;
}

JingleRtpOptions::JingleRtpOptions(QWidget *parent):
	QWidget(parent),ui(new Ui::JingleRtpOptions)
{
    ui->setupUi(this);

	ui->pbPayloadTypeUsedUp->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
	ui->pbPayloadTypeUsedDown->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
	ui->pbPayloadTypeUse->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));
	ui->pbPayloadTypeUnuse->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));

    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		ui->cmbAudioDeviceInput->addItem((*it).deviceName(), qVariantFromValue(*it));

	devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
	for(QList<QAudioDeviceInfo>::ConstIterator it=devices.constBegin(); it!=devices.constEnd(); ++it)
		ui->cmbAudioDeviceOutput->addItem((*it).deviceName(), qVariantFromValue(*it));

	QHash<int, QAVP> payloadTypes = QAVCodec::payloadTypes(true);
	for (QHash<int, QAVP>::ConstIterator it=payloadTypes.constBegin(); it!=payloadTypes.constEnd(); ++it)
	{
		int codecId = QAVCodec::idByName((*it).codecName);
		if (QAVCodec::findDecoder(codecId) && QAVCodec::findEncoder(codecId))
			FAvailableStaticPayloadTypes.append(*it);
	}

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

void JingleRtpOptions::onAvailablePayloadTypeSelectionChanged()
{
	QTreeWidgetItem *item = ui->twPayloadTypesAvailable->currentItem();
	if (item)
	{
		if (item->text(0).isEmpty())
		{
			ui->pbPayloadTypeEdit->setDisabled(true);
			ui->pbPayloadTypeRemove->setDisabled(true);
		}
		else
		{
			ui->pbPayloadTypeEdit->setEnabled(true);
			ui->pbPayloadTypeRemove->setEnabled(true);
		}
		ui->pbPayloadTypeUse->setEnabled(true);
	}
	else
	{
		ui->pbPayloadTypeUse->setDisabled(true);
		ui->pbPayloadTypeEdit->setDisabled(true);
		ui->pbPayloadTypeRemove->setDisabled(true);
	}
}

void JingleRtpOptions::onUsedPayloadTypeSelectionChanged()
{
	QTreeWidgetItem *item = ui->twPayloadTypesUsed->currentItem();
	if (item)
	{
		int index = ui->twPayloadTypesUsed->indexOfTopLevelItem(item);
		ui->pbPayloadTypeUsedUp->setEnabled(index>0);
		ui->pbPayloadTypeUsedDown->setEnabled(index < ui->twPayloadTypesUsed->topLevelItemCount()-1);
		ui->pbPayloadTypeUnuse->setEnabled(true);
	}
	else
	{
		ui->pbPayloadTypeUnuse->setDisabled(true);
		ui->pbPayloadTypeUsedUp->setDisabled(true);
		ui->pbPayloadTypeUsedDown->setDisabled(true);
	}
}

void JingleRtpOptions::onUsedPayloadTypePriorityUp()
{
	qDebug() << "JingleRtpOptions::onUsedPayloadTypePriorityUp()";
	QTreeWidgetItem *item = ui->twPayloadTypesUsed->currentItem();
	int index = ui->twPayloadTypesUsed->indexOfTopLevelItem(item);
	if (index > 0)
	{
		ui->twPayloadTypesUsed->takeTopLevelItem(index);
		index--;
		ui->twPayloadTypesUsed->insertTopLevelItem(index, item);
		ui->twPayloadTypesUsed->setCurrentItem(item);
	}
}

void JingleRtpOptions::onUsedPayloadTypePriorityDown()
{
	qDebug() << "JingleRtpOptions::onUsedPayloadTypePriorityDown()";
	QTreeWidgetItem *item = ui->twPayloadTypesUsed->currentItem();
	int index = ui->twPayloadTypesUsed->indexOfTopLevelItem(item);
	if (index < ui->twPayloadTypesUsed->topLevelItemCount()-1)
	{
		ui->twPayloadTypesUsed->takeTopLevelItem(index);
		index++;
		ui->twPayloadTypesUsed->insertTopLevelItem(index, item);
		ui->twPayloadTypesUsed->setCurrentItem(item);
	}
}

void JingleRtpOptions::onPayloadTypeUse()
{
	qDebug() << "JingleRtpOptions::onPayloadTypeUse()";
	QTreeWidgetItem *item = ui->twPayloadTypesAvailable->currentItem();
	int index = ui->twPayloadTypesAvailable->indexOfTopLevelItem(item);
	ui->twPayloadTypesAvailable->takeTopLevelItem(index);
	ui->twPayloadTypesUsed->addTopLevelItem(item);
	ui->twPayloadTypesUsed->setCurrentItem(item);
}

void JingleRtpOptions::onPayloadTypeUnuse()
{
	qDebug() << "JingleRtpOptions::onPayloadTypeUnuse()";
	QTreeWidgetItem *item = ui->twPayloadTypesUsed->currentItem();
	int index = ui->twPayloadTypesUsed->indexOfTopLevelItem(item);
	ui->twPayloadTypesUsed->takeTopLevelItem(index);
	ui->twPayloadTypesAvailable->addTopLevelItem(item);
	ui->twPayloadTypesAvailable->setCurrentItem(item);
}

void JingleRtpOptions::apply()
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

    emit childApply();
}

void JingleRtpOptions::reset()
{
	QList<QAVP> dynamicPayloadTypes = JingleRtp::avpsFromStrings(Options::node(OPV_JINGLE_RTP_PT_DYNAMIC).value().toStringList());
	QList<QAVP> usedPayloadTypes = JingleRtp::avpsFromStrings(Options::node(OPV_JINGLE_RTP_PT_USED).value().toStringList());

	ui->twPayloadTypesAvailable->clear();
	for (QList<QAVP>::ConstIterator it=FAvailableStaticPayloadTypes.constBegin(); it!=FAvailableStaticPayloadTypes.constEnd(); ++it)
		if (!usedPayloadTypes.contains(*it))
			ui->twPayloadTypesAvailable->addTopLevelItem(itemFromAvp(*it));

	for (QList<QAVP>::ConstIterator it=dynamicPayloadTypes.constBegin(); it!=dynamicPayloadTypes.constEnd(); ++it)
		if (!usedPayloadTypes.contains(*it))
			ui->twPayloadTypesAvailable->addTopLevelItem(itemFromAvp(*it));

	onAvailablePayloadTypeSelectionChanged();

	ui->twPayloadTypesUsed->clear();
	for (QList<QAVP>::ConstIterator it=usedPayloadTypes.constBegin(); it!=usedPayloadTypes.constEnd(); ++it)
		ui->twPayloadTypesUsed->addTopLevelItem(itemFromAvp(*it));

	onUsedPayloadTypeSelectionChanged();

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

#include "jinglertp.h"
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>

JingleRtpOptions::JingleRtpOptions(QWidget *parent):
	QWidget(parent),ui(new Ui::JingleRtpOptions)
{
    ui->setupUi(this);

	connect(ui->rptAvailable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onAvailablePayloadTypeSelectionChanged()));
	connect(ui->rptUsed->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onUsedPayloadTypeSelectionChanged()));

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
	QModelIndex index = ui->rptAvailable->currentIndex();
	if (index.isValid())
	{
		QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->rptAvailable->model());
		QStandardItem *item = model->itemFromIndex(index);
		if (model->item(item->row(), 0)->text().isEmpty())
		{
			ui->pbPayloadTypeEdit->setEnabled(true);
			ui->pbPayloadTypeRemove->setEnabled(true);
		}
		else
		{
			ui->pbPayloadTypeEdit->setDisabled(true);
			ui->pbPayloadTypeRemove->setDisabled(true);
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
	QModelIndex index = ui->rptUsed->currentIndex();
	if (index.isValid())
	{
		QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->rptUsed->model());
		QStandardItem *item = model->itemFromIndex(index);
		int row = item->row();
		ui->pbPayloadTypeUsedUp->setEnabled(row>0);
		ui->pbPayloadTypeUsedDown->setEnabled(row < ui->rptUsed->model()->rowCount()-1);
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
	QModelIndex index = ui->rptUsed->currentIndex();
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->rptUsed->model());
	QStandardItem *item = model->itemFromIndex(index);
	int row = item->row();
	if (row > 0)
	{
		QList<QStandardItem *> r = model->takeRow(row);
		row--;
		model->insertRow(row, r);
		ui->rptUsed->selectionModel()->setCurrentIndex(model->index(row, 0), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
	}
}

void JingleRtpOptions::onUsedPayloadTypePriorityDown()
{
	QModelIndex index = ui->rptUsed->currentIndex();
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->rptUsed->model());
	QStandardItem *item = model->itemFromIndex(index);
	int row = item->row();
	if (row < model->rowCount()-1)
	{
		QList<QStandardItem *> r = model->takeRow(row);
		row++;
		model->insertRow(row, r);
		ui->rptUsed->selectionModel()->setCurrentIndex(model->index(row, 0), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
	}
}

void JingleRtpOptions::onPayloadTypeUnuse()
{
	ui->rptAvailable->setCurrentRow(ui->rptAvailable->appendAvp(ui->rptUsed->takeAvp(ui->rptUsed->currentRow())));
	ui->rptAvailable->setSortingEnabled(true);
}

void JingleRtpOptions::onPayloadTypeUse()
{
	ui->rptUsed->setCurrentRow(ui->rptUsed->appendAvp(ui->rptAvailable->takeAvp(ui->rptAvailable->currentRow())));
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

	ui->rptAvailable->clear();
	ui->rptAvailable->setSortingEnabled(false);
	for (QList<QAVP>::ConstIterator it=FAvailableStaticPayloadTypes.constBegin(); it!=FAvailableStaticPayloadTypes.constEnd(); ++it)
		if (!usedPayloadTypes.contains(*it))
			ui->rptAvailable->appendAvp(*it);

	for (QList<QAVP>::ConstIterator it=dynamicPayloadTypes.constBegin(); it!=dynamicPayloadTypes.constEnd(); ++it)
		if (!usedPayloadTypes.contains(*it))
			ui->rptAvailable->appendAvp(*it);

	ui->rptAvailable->sortByColumn(0, Qt::AscendingOrder);
	ui->rptAvailable->setSortingEnabled(true);

	onAvailablePayloadTypeSelectionChanged();

	ui->rptUsed->clear();
	for (QList<QAVP>::ConstIterator it=usedPayloadTypes.constBegin(); it!=usedPayloadTypes.constEnd(); ++it)
		ui->rptUsed->appendAvp(*it);

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


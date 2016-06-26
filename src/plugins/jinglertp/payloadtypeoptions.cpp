#include <QDebug>
#include <QAVOutputFormat>
#include "jinglertp.h"
#include "payloadtypeeditdialog.h"
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>

PayloadTypeOptions::PayloadTypeOptions(QWidget *parent):
	QWidget(parent),ui(new Ui::PayloadTypeOptions)
{
    ui->setupUi(this);

	connect(ui->rptAvailable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onAvailablePayloadTypeSelectionChanged()));
	connect(ui->rptUsed->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onUsedPayloadTypeSelectionChanged()));

	connect(ui->rptAvailable->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(modified()));
	connect(ui->rptUsed->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(modified()));

	connect(ui->rptAvailable->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(modified()));
	connect(ui->rptUsed->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(modified()));

	connect(ui->rptAvailable->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(modified()));
	connect(ui->rptUsed->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(modified()));

	ui->pbUsedUp->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
	ui->pbUsedDown->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
	ui->pbUse->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));
	ui->pbUnuse->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));

	// Build list of supported payload types
	QAVOutputFormat rtp(NULL);
	for (QAVOutputFormat format = QAVOutputFormat::first(); format; ++format)
		if (format.name()=="rtp")
		{
			rtp = format;
			break;
		}

	if (rtp)
	{
		QHash<int, PayloadType> payloadTypes = QAVCodec::payloadTypes(true);
		for (QHash<int, PayloadType>::ConstIterator it=payloadTypes.constBegin(); it!=payloadTypes.constEnd(); ++it)
			if ((*it).media==PayloadType::Audio) // Only Audio implemented so far
			{
				int codecId = QAVCodec::idByName((*it).name);
				if (rtp.queryCodec(codecId) && QAVCodec::findDecoder(codecId) && QAVCodec::findEncoder(codecId))
					FAvailableStaticPayloadTypes.append(*it);
			}
	}
    reset();
}

PayloadTypeOptions::~PayloadTypeOptions()
{
    delete ui;
}

void PayloadTypeOptions::onAvailablePayloadTypeSelectionChanged()
{
	QModelIndex index = ui->rptAvailable->currentIndex();
	if (index.isValid())
	{
		QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->rptAvailable->model());
		QStandardItem *item = model->itemFromIndex(index);
		if (model->item(item->row(), 0)->text().isEmpty())
		{
			ui->pbEdit->setEnabled(true);
			ui->pbRemove->setEnabled(true);
		}
		else
		{
			ui->pbEdit->setDisabled(true);
			ui->pbRemove->setDisabled(true);
		}
		ui->pbUse->setEnabled(true);
	}
	else
	{
		ui->pbUse->setDisabled(true);
		ui->pbEdit->setDisabled(true);
		ui->pbRemove->setDisabled(true);
	}
}

void PayloadTypeOptions::onUsedPayloadTypeSelectionChanged()
{
	QModelIndex index = ui->rptUsed->currentIndex();
	if (index.isValid())
	{
		QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->rptUsed->model());
		QStandardItem *item = model->itemFromIndex(index);
		int row = item->row();
		ui->pbUsedUp->setEnabled(row>0);
		ui->pbUsedDown->setEnabled(row < ui->rptUsed->model()->rowCount()-1);
		ui->pbUnuse->setEnabled(true);
	}
	else
	{
		ui->pbUnuse->setDisabled(true);
		ui->pbUsedUp->setDisabled(true);
		ui->pbUsedDown->setDisabled(true);
	}
}

void PayloadTypeOptions::onUsedPayloadTypePriorityUp()
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

void PayloadTypeOptions::onUsedPayloadTypePriorityDown()
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

void PayloadTypeOptions::onPayloadTypeUnuse()
{
	ui->rptAvailable->setCurrentRow(ui->rptAvailable->appendAvp(ui->rptUsed->takeAvp(ui->rptUsed->currentRow())));
	ui->rptAvailable->setSortingEnabled(true);
}

void PayloadTypeOptions::onPayloadTypeAdd()
{
	QList<PayloadType> payloadTypes = availablePayloadTypes();
	PayloadTypeEditDialog *dialog = new PayloadTypeEditDialog(PayloadType(), payloadTypes, this);
	if (dialog->exec()==QDialog::Accepted)
		ui->rptAvailable->setCurrentRow(ui->rptAvailable->appendAvp(dialog->payloadType()));
	dialog->deleteLater();
}

void PayloadTypeOptions::onPayloadTypeEdit()
{
	int row = ui->rptAvailable->currentRow();
	PayloadType avp = ui->rptAvailable->getAvp(row);
	if (avp.isValid())
	{
		QList<PayloadType> payloadTypes = availablePayloadTypes();
		PayloadTypeEditDialog *dialog = new PayloadTypeEditDialog(avp, payloadTypes, this);
		if (dialog->exec()==QDialog::Accepted)
		{
			ui->rptAvailable->updateRow(row, dialog->payloadType());
			ui->rptAvailable->setCurrentRow(row);
		}
		dialog->deleteLater();
	}
}

void PayloadTypeOptions::onPayloadTypeRemove()
{
	ui->rptAvailable->takeAvp(ui->rptAvailable->currentRow());
}

void PayloadTypeOptions::onPayloadTypeUse()
{
	ui->rptUsed->setCurrentRow(ui->rptUsed->appendAvp(ui->rptAvailable->takeAvp(ui->rptAvailable->currentRow())));
}

void PayloadTypeOptions::apply()
{
	QList<PayloadType> dynamicPayloadTypes, usedPayloadTypes;
	for (int i=0; i<ui->rptAvailable->rowCount(); ++i)
	{
		PayloadType payloadType = ui->rptAvailable->getAvp(i);
		if (payloadType.id<0)	// Dynamic
			dynamicPayloadTypes.append(payloadType);
	}

	for (int i=0; i<ui->rptUsed->rowCount(); ++i)
	{
		PayloadType payloadType = ui->rptUsed->getAvp(i);
		if (payloadType.id<0)	// Dynamic
			dynamicPayloadTypes.append(payloadType);
		usedPayloadTypes.append(payloadType);
	}

	Options::node(OPV_JINGLE_RTP_PT_DYNAMIC).setValue(JingleRtp::stringsFromAvps(dynamicPayloadTypes));
	Options::node(OPV_JINGLE_RTP_PT_USED).setValue(JingleRtp::stringsFromAvps(usedPayloadTypes));

    emit childApply();
}

void PayloadTypeOptions::reset()
{
	QList<PayloadType> dynamicPayloadTypes = JingleRtp::avpsFromStrings(Options::node(OPV_JINGLE_RTP_PT_DYNAMIC).value().toStringList());
	QList<PayloadType> usedPayloadTypes = JingleRtp::avpsFromStrings(Options::node(OPV_JINGLE_RTP_PT_USED).value().toStringList());

	ui->rptAvailable->clear();
	ui->rptAvailable->setSortingEnabled(false);
	for (QList<PayloadType>::ConstIterator it=FAvailableStaticPayloadTypes.constBegin(); it!=FAvailableStaticPayloadTypes.constEnd(); ++it)
		if (!usedPayloadTypes.contains(*it))
			ui->rptAvailable->appendAvp(*it);

	for (QList<PayloadType>::ConstIterator it=dynamicPayloadTypes.constBegin(); it!=dynamicPayloadTypes.constEnd(); ++it)
		if (!usedPayloadTypes.contains(*it))
			ui->rptAvailable->appendAvp(*it);

	ui->rptAvailable->sortByColumn(0, Qt::AscendingOrder);
	ui->rptAvailable->setSortingEnabled(true);

	onAvailablePayloadTypeSelectionChanged();

	ui->rptUsed->clear();
	for (QList<PayloadType>::ConstIterator it=usedPayloadTypes.constBegin(); it!=usedPayloadTypes.constEnd(); ++it)
		ui->rptUsed->appendAvp(*it);

	onUsedPayloadTypeSelectionChanged();

    emit childReset();
}

void PayloadTypeOptions::changeEvent(QEvent *e)
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

QList<PayloadType> PayloadTypeOptions::availablePayloadTypes() const
{
	QList<PayloadType> payloadTypes;
	for (int i=0; i<ui->rptAvailable->rowCount(); ++i)
		payloadTypes.append(ui->rptAvailable->getAvp(i));

	for (int i=0; i<ui->rptUsed->rowCount(); ++i)
		payloadTypes.append(ui->rptUsed->getAvp(i));
	return payloadTypes;
}

#include "rtppayloadtypeselector.h"

RtpPayloadTypeSelector::RtpPayloadTypeSelector(QWidget *AParent):
	QTreeView(AParent),
	FModel(new QStandardItemModel(this))
{
	setModel(FModel);
}

void RtpPayloadTypeSelector::clear()
{
	FModel->removeRows(0, FModel->rowCount());
}

int RtpPayloadTypeSelector::appendAvp(const QAVP &AAvp)
{
	QList<QStandardItem *> items;

	items.append(new QStandardItem(AAvp.payloadType>=0?QString::number(AAvp.payloadType):QString()));
	items.append(new QStandardItem(AAvp.codecName));
	items.append(new QStandardItem(QString::number(AAvp.clockRate)));
	items.append(new QStandardItem(QString::number(AAvp.channels)));
	items.append(new QStandardItem(tr(AAvp.mediaType==QAVP::Audio?"Audio":
									  AAvp.mediaType==QAVP::Video?"Video":
									  AAvp.mediaType==QAVP::Both?"Both":"Unknown")));

	FModel->appendRow(items);
	return FModel->rowCount()-1;
}

QAVP RtpPayloadTypeSelector::getAvp(int ARow) const
{
	QAVP avp;
	QString payloadType = FModel->item(ARow, 0)->text();
	avp.payloadType = payloadType.isEmpty()?-1:payloadType.toInt();
	avp.codecName = FModel->item(ARow, 1)->text();
	avp.clockRate = FModel->item(ARow, 2)->text().toInt();
	avp.channels = FModel->item(ARow, 3)->text().toInt();
	QString mediaType = FModel->item(ARow, 4)->text();
	avp.mediaType = mediaType=="Audio"?QAVP::Audio:
					mediaType=="Video"?QAVP::Video:
					mediaType=="Both"?QAVP::Both:QAVP::Unknown;
	return avp;
}

QAVP RtpPayloadTypeSelector::takeAvp(int ARow)
{
	QList<QStandardItem*> row = FModel->takeRow(ARow);
	QAVP avp;
	QString payloadType = row.at(0)->text();
	avp.payloadType = payloadType.isEmpty()?-1:payloadType.toInt();
	avp.codecName = row.at(1)->text();
	avp.clockRate = row.at(2)->text().toInt();
	avp.channels = row.at(3)->text().toInt();
	QString mediaType = row.at(4)->text();
	avp.mediaType = mediaType=="Audio"?QAVP::Audio:
					mediaType=="Video"?QAVP::Video:
					mediaType=="Both"?QAVP::Both:QAVP::Unknown;
	return avp;
}

int RtpPayloadTypeSelector::currentRow() const
{
	return currentIndex().row();
}

void RtpPayloadTypeSelector::setCurrentRow(int ARow)
{
	selectionModel()->setCurrentIndex(FModel->index(ARow, 0), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

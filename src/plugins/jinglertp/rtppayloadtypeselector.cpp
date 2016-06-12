#include <QDebug>
#include "rtppayloadtypeselector.h"

RtpPayloadTypeSelector::RtpPayloadTypeSelector(QWidget *AParent):
	QTreeView(AParent),
	FModel(new QStandardItemModel(this))
{
	setModel(FModel);
	setSortRole(Qt::UserRole+1);
	QStringList headers;
	headers << tr("Id")<< tr("Name") << tr("Clockrate") << tr("Channels")
			<< tr("Media type");

	FModel->setHorizontalHeaderLabels(headers);

	setColumnWidth(0, 20); // Id
	setColumnWidth(1, 48); // Codec name
	setColumnWidth(2, 56); // Clock rate
	setColumnWidth(3, 56); // Number of channels
//	FServerList->setColumnWidth(4, 30); // Media type
}

void RtpPayloadTypeSelector::clear()
{
	FModel->removeRows(0, FModel->rowCount());
}

int RtpPayloadTypeSelector::appendAvp(const QAVP &AAvp)
{
	QList<QStandardItem *> items;

	QStandardItem *item = new QStandardItem(AAvp.payloadType<0?QString():QString::number(AAvp.payloadType));
	item->setData(AAvp.payloadType<0?1000:AAvp.payloadType);
	items.append(item);
	item = new QStandardItem(AAvp.codecName);
	item->setData(AAvp.codecName);
	items.append(item);
	item = new QStandardItem(QString::number(AAvp.clockRate));
	item->setData(AAvp.clockRate);
	items.append(item);
	item = new QStandardItem(QString::number(AAvp.channels));
	item->setData(AAvp.channels);
	items.append(item);
	item = new QStandardItem(tr(AAvp.mediaType==QAVP::Audio?"Audio":
								AAvp.mediaType==QAVP::Video?"Video":
								AAvp.mediaType==QAVP::Both?"Both":"Unknown"));
	item->setData(AAvp.mediaType);
	items.append(item);

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

int RtpPayloadTypeSelector::rowCount() const
{
	return FModel->rowCount();
}

int RtpPayloadTypeSelector::currentRow() const
{
	return currentIndex().row();
}

void RtpPayloadTypeSelector::setCurrentRow(int ARow)
{
	selectionModel()->setCurrentIndex(FModel->index(ARow, 0), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

void RtpPayloadTypeSelector::setSortRole(int ARole)
{
	FModel->setSortRole(ARole);
}

int RtpPayloadTypeSelector::sortRole() const
{
	return FModel->sortRole();
}

bool RtpPayloadTypeSelector::setItemData(int ARow, const QVariant &AData, int AColumn, int ARole)
{
	return FModel->setData(FModel->index(ARow, AColumn), AData, ARole);
}

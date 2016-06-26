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

int RtpPayloadTypeSelector::appendAvp(const PayloadType &AAvp)
{
	QList<QStandardItem *> items;

	QStandardItem *item = new QStandardItem(AAvp.id<0?QString():QString::number(AAvp.id));
	item->setData(AAvp.id<0?1000:AAvp.id);
	items.append(item);
	item = new QStandardItem(AAvp.name);
	item->setData(AAvp.name);
	items.append(item);
	item = new QStandardItem(QString::number(AAvp.clockrate));
	item->setData(AAvp.clockrate);
	items.append(item);
	item = new QStandardItem(QString::number(AAvp.channels));
	item->setData(AAvp.channels);
	items.append(item);
	item = new QStandardItem(tr(AAvp.media == PayloadType::Audio ? "Audio":
								AAvp.media == PayloadType::Video ? "Video":
								AAvp.media == PayloadType::Both ? "Both" : "Unknown"));
	item->setData(AAvp.media);
	items.append(item);

	FModel->appendRow(items);
	return FModel->rowCount()-1;
}

void RtpPayloadTypeSelector::updateRow(int ARow, const PayloadType &AAvp)
{
	QStandardItem *item = new QStandardItem(AAvp.id<0?QString():QString::number(AAvp.id));
	item->setData(AAvp.id<0?1000:AAvp.id);
	FModel->setItem(ARow, 0, item);
	item = new QStandardItem(AAvp.name);
	item->setData(AAvp.name);
	FModel->setItem(ARow, 1, item);
	item = new QStandardItem(QString::number(AAvp.clockrate));
	item->setData(AAvp.clockrate);
	FModel->setItem(ARow, 2, item);
	item = new QStandardItem(QString::number(AAvp.channels));
	item->setData(AAvp.channels);
	FModel->setItem(ARow, 3, item);
	item = new QStandardItem(tr(AAvp.media == PayloadType::Audio? "Audio":
								AAvp.media == PayloadType::Video? "Video":
								AAvp.media == PayloadType::Both? "Both" : "Unknown"));
	item->setData(AAvp.media);
	FModel->setItem(ARow, 4, item);
}

PayloadType RtpPayloadTypeSelector::getAvp(int ARow) const
{
	PayloadType avp;
	QString id = FModel->item(ARow, 0)->text();
	avp.id = id.isEmpty()?-1:id.toInt();
	avp.name = FModel->item(ARow, 1)->text();
	avp.clockrate = FModel->item(ARow, 2)->text().toInt();
	avp.channels = FModel->item(ARow, 3)->text().toInt();
	QString media = FModel->item(ARow, 4)->text();
	avp.media = media=="Audio" ? PayloadType::Audio:
				media=="Video" ? PayloadType::Video:
				media=="Both" ? PayloadType::Both: PayloadType::Unknown;
	return avp;
}

PayloadType RtpPayloadTypeSelector::takeAvp(int ARow)
{
	QList<QStandardItem*> row = FModel->takeRow(ARow);
	PayloadType avp;
	QString id = row.at(0)->text();
	avp.id = id.isEmpty()?-1:id.toInt();
	avp.name = row.at(1)->text();
	avp.clockrate = row.at(2)->text().toInt();
	avp.channels = row.at(3)->text().toInt();
	QString media = row.at(4)->text();
	avp.media = media == "Audio" ? PayloadType::Audio:
				media == "Video" ? PayloadType::Video:
				media == "Both" ? PayloadType::Both : PayloadType::Unknown;
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


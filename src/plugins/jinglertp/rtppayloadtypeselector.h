#ifndef RTPPAYLOADTYPESELECTOR_H
#define RTPPAYLOADTYPESELECTOR_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QAVCodec>

class RtpPayloadTypeSelector : public QTreeView
{
	Q_OBJECT
public:
	explicit RtpPayloadTypeSelector(QWidget *AParent=NULL);
	void clear();
	int appendAvp(const QAVP &AAvp);
	QAVP getAvp(int ARow) const;
	QAVP takeAvp(int ARow);
	int	 rowCount() const;
	int currentRow() const;
	void setCurrentRow(int ARow);
	void setSortRole(int ARole);
	int  sortRole() const;
	void updateRow(int ARow, const QAVP &AAvp);
	bool setItemData(int ARow, const QVariant &AData, int AColumn = 0, int ARole = Qt::UserRole);
private:
	QStandardItemModel *FModel;
};


#endif // RTPPAYLOADTYPESELECTOR_H

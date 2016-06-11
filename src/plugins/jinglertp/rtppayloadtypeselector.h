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
	int currentRow() const;
	void setCurrentRow(int ARow);
private:
	QStandardItemModel *FModel;
};


#endif // RTPPAYLOADTYPESELECTOR_H

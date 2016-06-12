#ifndef PAYLOADTYPEEDITDIALOG_H
#define PAYLOADTYPEEDITDIALOG_H

#include <QDialog>
#include <QAVCodec>

namespace Ui {
class PayloadTypeEditDialog;
}

class PayloadTypeEditDialog : public QDialog
{
	Q_OBJECT

public:
	PayloadTypeEditDialog(const QAVP &APayloadType, QWidget *AParent = 0);
	~PayloadTypeEditDialog();

private:
	Ui::PayloadTypeEditDialog *ui;
	QAVP FPayloadType;
};

#endif // PAYLOADTYPEEDITDIALOG_H

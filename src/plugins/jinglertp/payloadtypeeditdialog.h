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
	PayloadTypeEditDialog(const PayloadType &APayloadType, const QList<PayloadType> &APayloadTypes, QWidget *AParent = 0);
	~PayloadTypeEditDialog();
	PayloadType payloadType() const;
	// QDialog interface
public slots:
	void accept();
protected slots:
	void onMediaTypeSelected(int AIndex);
	void onCodecSelected(int AIndex);
	void onSettingsChanged();
private:
	Ui::PayloadTypeEditDialog *ui;
	PayloadType FPayloadType;
	QList<PayloadType> FPayloadTypes;
};

#endif // PAYLOADTYPEEDITDIALOG_H

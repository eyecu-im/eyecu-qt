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
	PayloadTypeEditDialog(const QAVP &APayloadType, const QList<QAVP> &APayloadTypes, QWidget *AParent = 0);
	~PayloadTypeEditDialog();
	QAVP payloadType() const;
	// QDialog interface
public slots:
	void accept();
protected slots:
	void onMediaTypeSelected(int AIndex);
	void onCodecSelected(int AIndex);
	void onSettingsChanged();
private:
	Ui::PayloadTypeEditDialog *ui;
	QAVP FPayloadType;
	QList<QAVP> FPayloadTypes;
};

#endif // PAYLOADTYPEEDITDIALOG_H

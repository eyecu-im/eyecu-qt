#include "payloadtypeeditdialog.h"
#include "ui_payloadtypeeditdialog.h"

PayloadTypeEditDialog::PayloadTypeEditDialog(const QAVP &APayloadType, QWidget *AParent) :
	QDialog(AParent),
	ui(new Ui::PayloadTypeEditDialog),
	FPayloadType(APayloadType)
{
	ui->setupUi(this);
	const QStringList codecNames = QAVCodec::codecNames(true);
	for (QStringList::ConstIterator it = codecNames.constBegin(); it != codecNames.constEnd(); ++it)
		ui->cmbCodecName->addItem(*it, QAVCodec::idByName((*it)));
}


PayloadTypeEditDialog::~PayloadTypeEditDialog()
{
	delete ui;
}

#include "confirmationdialog.h"
#include "ui_confirmationdialog.h"

ConfirmationDialog::ConfirmationDialog(const QString &ATitle, const QString &ALabel, QString &AReason, bool &AStore, bool &AAsk, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ConfirmationDialog),
	FReason(AReason), FStore(AStore), FAsk(AAsk)
{
	setWindowTitle(ATitle);
	ui->setupUi(this);
	ui->lblLabel->setText(ALabel);
	ui->tedText->setText(FReason);
	ui->chkStore->setChecked(FStore);
	ui->chkAsk->setChecked(FAsk);
}

ConfirmationDialog::~ConfirmationDialog()
{
	delete ui;
}

void ConfirmationDialog::accept()
{
	FReason = ui->tedText->toPlainText();
	FAsk = ui->chkAsk->isChecked();
	FStore = ui->chkStore->isChecked();
	QDialog::accept();
}

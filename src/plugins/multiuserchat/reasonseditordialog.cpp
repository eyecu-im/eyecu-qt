#include "reasonseditordialog.h"
#include <definitions/optionvalues.h>
#include "ui_reasonseditordialog.h"

ReasonsEditorDialog::ReasonsEditorDialog(const QString &ATitle, const QString &ALabel, QString &AReason, bool &ABad, bool &AStore, bool &AAsk, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ReasonsEditorDialog),
	FReason(AReason), FStore(AStore), FAsk(AAsk), FBad(ABad)
{
	setWindowTitle(ATitle);
	ui->setupUi(this);
	ui->lblLabel->setText(ALabel);
	ui->tedText->setText(FReason);
	ui->chkStore->setChecked(FStore);
	ui->chkAsk->setChecked(FAsk);
	if (ABad)
	{
		FList = OPV_MUC_BADREASONS;
	}
	else
	{
		FList = OPV_MUC_REASONS;
	}
	ui->lstReasons->addItems(Options::node(FList).value().toStringList());

	connect(ui->btnAdd, SIGNAL(clicked()), SLOT(onItemAdd()));
	connect(ui->btnRemove, SIGNAL(clicked()), SLOT(onItemDelete()));
	connect(ui->lstReasons, SIGNAL(currentTextChanged(QString)), SLOT(currentChanged(QString)));
}

ReasonsEditorDialog::~ReasonsEditorDialog()
{
	delete ui;
}

void ReasonsEditorDialog::currentChanged(const QString & FReason)
{
	ui->tedText->setText(FReason);
}

void ReasonsEditorDialog::onItemAdd()
{
	//res = ui->tedText->text().trimmed();
	FReason = ui->tedText->toPlainText();
	if (FReason.isEmpty())
		return;
	ui->lstReasons->addItem(FReason);
}

void ReasonsEditorDialog::onItemDelete()
{
	int idx = ui->lstReasons->currentRow();
	if (idx >= 0) {
		QListWidgetItem *item =ui->lstReasons->takeItem(idx);
		if (item)
			delete item;
	}
}

void ReasonsEditorDialog::accept()
{
	FReason = ui->tedText->toPlainText();
	FAsk = ui->chkAsk->isChecked();
	FStore = ui->chkStore->isChecked();
	save();
	QDialog::accept();
}

void ReasonsEditorDialog::save()
{
	QStringList reasonsList;
	int cnt = ui->lstReasons->count();
	for (int i = 0; i < cnt; ++i)
		reasonsList.append(ui->lstReasons->item(i)->text());
	Options::node(FList).setValue(reasonsList);
}
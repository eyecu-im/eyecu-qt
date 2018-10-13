#include <QClipboard>
#include <QPushButton>
#include <utils/shortcuts.h>
#include <definitions/shortcuts.h>
#include "addlink.h"

AddLink::AddLink(const QIcon &AIcon, const QUrl &AHref, const QString &ADescription, QWidget *parent) :
    QDialog(parent),
	FSchemeMasks(QStringList() << "http" << "https" << "ftp" << "xmpp" << "mailto" << "tel" << "native"),
    FOriginalHref(AHref),
    FOriginalDescription(ADescription),
    ui(new Ui::AddLink)
{
    setWindowIcon(AIcon);
    ui->setupUi(this);

	ui->buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Add"));
	ui->buttonBox->button(QDialogButtonBox::No)->setText(tr("Remove"));

    if (!ADescription.isEmpty())
        ui->tedDesc->setEnabled(false);
    if (AHref.isEmpty())
	{
		ui->buttonBox->button(QDialogButtonBox::No)->setEnabled(false);
	}

    if (FOriginalHref.isValid())
    {
        ui->ledPath->setText(FOriginalHref.toString());
		ui->buttonBox->button(QDialogButtonBox::Yes)->setText(tr("Change"));
    }
    else
    {
        QUrl url = QUrl::fromUserInput(QApplication::clipboard()->text());
        if (url.isValid() && FSchemeMasks.contains(url.scheme()))
            ui->ledPath->setText(url.toString());
    }  

    ui->tedDesc->setText(FOriginalDescription.isEmpty()?ui->ledPath->text().isEmpty()?tr("Link"):ui->ledPath->text():FOriginalDescription);

    ui->ledPath->selectAll();
    ui->ledPath->setFocus();

	Shortcuts::bindObjectShortcut(SCT_MESSAGEWINDOWS_LINKDIALOG_OK, ui->buttonBox->button(QDialogButtonBox::Yes));
}

AddLink::~AddLink()
{
	delete ui;
}

void AddLink::onButtonClicked(QAbstractButton *AButton)
{
	switch (ui->buttonBox->buttonRole(AButton))
	{
		case QDialogButtonBox::YesRole:
			FDescription = !ui->tedDesc->toPlainText().isEmpty() ? ui->tedDesc->toPlainText() : "";
			FHref = QUrl::fromUserInput(ui->ledPath->text());
			done(Add);
			break;
		case QDialogButtonBox::NoRole:
			done(Remove);
		default:
			break;
	}
}

void AddLink::onTextChanged()
{
    QUrl url = QUrl::fromUserInput(ui->ledPath->text());
	ui->buttonBox->button(QDialogButtonBox::Yes)->setEnabled((url.isValid()||
                            FOriginalDescription != ui->tedDesc->toPlainText()) &&
                            FOriginalHref != ui->ledPath->text());
    ui->cmbScheme->blockSignals(true);
    ui->cmbScheme->setCurrentIndex(FSchemeMasks.indexOf(url.scheme())+1);
    ui->cmbScheme->blockSignals(false);
}

void AddLink::onSchemeChanged(int index)
{
    QUrl url = QUrl::fromUserInput(ui->ledPath->text());
    url.setScheme(index?FSchemeMasks[index-1]:"");
    ui->ledPath->setText(url.toString());
}

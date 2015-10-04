#include <QClipboard>
#include <utils/shortcuts.h>
#include <definitions/shortcuts.h>
#include "addlink.h"

AddLink::AddLink(const QIcon &AIcon, const QUrl &AHref, const QString &ADescription, QWidget *parent) :
    QDialog(parent),
    FSchemeMasks(QStringList() << "http" << "https" << "ftp" << "xmpp" << "mail" << "tel" << "native"),
    FOriginalHref(AHref),
    FOriginalDescription(ADescription),
    ui(new Ui::AddLink)
{
//    setWindowTitle(ATitle);
    setWindowIcon(AIcon);
    ui->setupUi(this);

    if (!ADescription.isEmpty())
        ui->tedDesc->setEnabled(false);
    if (AHref.isEmpty())
        ui->pbtRemove->setEnabled(false);

    if (FOriginalHref.isValid())
    {
        ui->ledPath->setText(FOriginalHref.toString());
        ui->pbtAdd->setText(tr("Change"));
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

    Shortcuts::bindObjectShortcut(SCT_MESSAGEWINDOWS_LINKDIALOG_OK, ui->pbtAdd);
}

AddLink::~AddLink()
{
    delete ui;
}

void AddLink::onAdd()
{
    FDescription = !ui->tedDesc->toPlainText().isEmpty() ? ui->tedDesc->toPlainText() : "";
    FHref = ui->ledPath->text();
    done(Add);
}

void AddLink::onRemove()
{
    done(Remove);
}

void AddLink::onTextChanged()
{
    QUrl url = QUrl::fromUserInput(ui->ledPath->text());
    ui->pbtAdd->setEnabled((url.isValid()||
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

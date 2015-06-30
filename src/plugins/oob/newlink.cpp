#include "newlink.h"


#include <QDebug>
#include <QApplication>
#include <QClipboard>

#include <utils/shortcuts.h>
#include <definitions/shortcuts.h>

NewLink::NewLink(const QString &ATitle, const QIcon &AIcon, QUrl AUrl, QString ADescription, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewLink),
    FUrl(AUrl),
    FDescription(ADescription),
    FSchemeMasks(QStringList() << "http" << "https" << "ftp" << "xmpp" << "mailto" << "tel" << "native")
{
    setWindowIcon(AIcon);
    setWindowTitle(ATitle);
    ui->setupUi(this);

    if (FUrl.isValid())
        ui->ledUrl->setText(FUrl.toString());
    else
    {
        QUrl url = QUrl::fromUserInput(QApplication::clipboard()->text());
        if (url.isValid() && FSchemeMasks.contains(url.scheme()))
            ui->ledUrl->setText(url.toString());
    }

    ui->tedDescription->setText(FDescription);

    ui->ledUrl->selectAll();
    ui->ledUrl->setFocus();
    Shortcuts::bindObjectShortcut(SCT_MESSAGEWINDOWS_LINKDIALOG_OK, ui->pbOk);
    enableButtons();
}

NewLink::~NewLink()
{
    delete ui;
}

void NewLink::enableButtons()
{
    QUrl url = QUrl::fromUserInput(ui->ledUrl->text());
    ui->pbOk->setEnabled(url.isValid() && (url != FUrl ||
                         FDescription!=ui->tedDescription->toPlainText()));
}

void NewLink::onUrlChanged(QString AText)
{
    QUrl url = QUrl::fromUserInput(AText);
    ui->cmbScheme->blockSignals(true);
    ui->cmbScheme->setCurrentIndex(FSchemeMasks.indexOf(url.scheme())+1);
    ui->cmbScheme->blockSignals(false);
    enableButtons();
}

void NewLink::onDescriptionChanged()
{
    enableButtons();
}

void NewLink::onSchemeChanged(int index)
{
    QUrl url = QUrl::fromUserInput(ui->ledUrl->text());
    url.setScheme(index?FSchemeMasks[index-1]:"");
    ui->ledUrl->setText(url.toString());
}

#ifndef NEWLINK_H
#define NEWLINK_H

#include <QUrl>

#include "ui_newlink.h"

namespace Ui {
    class NewLink;
}

class NewLink : public QDialog
{
    Q_OBJECT

public:
    NewLink(const QString &ATitle, const QIcon &AIcon, QUrl AUrl=QUrl(), QString ADescription=QString(), QWidget *parent = 0);
    ~NewLink();
    QUrl getUrl() const {return QUrl::fromUserInput(ui->ledUrl->text());}
    QString getDescription() const { return ui->tedDescription->toPlainText();}
    Ui::NewLink *ui;

protected:
    void enableButtons();

protected slots:
    void onUrlChanged(QString AText);
    void onDescriptionChanged();
    void onSchemeChanged(int index);

private:
    QUrl    FUrl;
    QString FDescription;
    const QStringList FSchemeMasks;
};

#endif // NEWLINK_H

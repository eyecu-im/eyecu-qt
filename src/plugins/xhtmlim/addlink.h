#ifndef ADDLINK_H
#define ADDLINK_H

#include <QUrl>

#include "ui_addlink.h"

namespace Ui {
    class AddLink;
}

class AddLink : public QDialog
{
    Q_OBJECT

public:
    enum LinkResult
    {
        Cancel,
        Add,
        Remove
    };

	AddLink(const QIcon &AIcon, const QUrl &AHref, const QString &ADescription, QWidget *parent = 0);
    ~AddLink();

    const QString &description() const {return FDescription;}
    const QUrl    &url() const {return FHref;}

private:
    const QStringList FSchemeMasks;
    QUrl    FHref;
    QUrl    FOriginalHref;
    QString FDescription;
    QString FOriginalDescription;

    Ui::AddLink *ui;

protected slots:
	void onButtonClicked(QAbstractButton *AButton);
    void onTextChanged();
    void onSchemeChanged(int index);
};

#endif // ADDLINK_H

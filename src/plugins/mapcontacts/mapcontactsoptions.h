#ifndef MAPCONTACTSOPTIONS_H
#define MAPCONTACTSOPTIONS_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>

#define MCO_VIEW_FULL       0
#define MCO_VIEW_FULLACTIVE 1
#define MCO_VIEW_COMPACT    2

namespace Ui {
    class MapContactsOptions;
}

class MapContactsOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    virtual QWidget* instance() {return this; }
    ~MapContactsOptions();
    explicit MapContactsOptions(QWidget *parent = 0);

// IOptionsWidget
public slots:
        virtual void apply();
        virtual void reset();

signals:
        void modified();
        void childApply();
        void childReset();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MapContactsOptions *ui;
};

#endif // MAPCONTACTSOPTIONS_H

#ifndef POSITIONINGMETHODLOCATIONOPTIONS_H
#define POSITIONINGMETHODLOCATIONOPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include "definitions/optionvalues.h"

#include "ui_positioningmethodlocationoptions.h"

namespace Ui {
class PositioningMethodLocationOptions;
}

class PositioningMethodLocationOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit PositioningMethodLocationOptions(QWidget *parent = 0);
    ~PositioningMethodLocationOptions();
    virtual QWidget* instance() { return this; }
    Ui::PositioningMethodLocationOptions *ui;
    
public slots:
    virtual void apply();
    virtual void reset();
signals:
    void modified();
    void childApply();
    void childReset();

protected slots:

protected:
    void changeEvent(QEvent *e);
};

#endif // POSITIONINGMETHODLOCATIONOPTIONS_H

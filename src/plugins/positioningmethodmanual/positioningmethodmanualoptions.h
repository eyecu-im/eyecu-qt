#ifndef POSITIONINGMETHODMANUALOPTIONS_H
#define POSITIONINGMETHODMANUALOPTIONS_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>
#include "definitions/optionvalues.h"

#include "ui_positioningmethodmanualoptions.h"

namespace Ui {
class PositioningMethodManualOptions;
}

class PositioningMethodManualOptions:
        public QWidget,
		public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit PositioningMethodManualOptions(QWidget *parent = 0);
    ~PositioningMethodManualOptions();
    virtual QWidget* instance() { return this; }
    Ui::PositioningMethodManualOptions *ui;
    
public slots:
    virtual void apply();
    virtual void reset();

protected:
    void changeEvent(QEvent *e);

protected slots:
	void onEditTextChanged(const QString &AText);

signals:
    void modified();
    void childApply();
    void childReset();
};

#endif // POSITIONINGMETHODMANUALOPTIONS_H

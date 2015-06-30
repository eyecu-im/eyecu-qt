#ifndef POSITIONINGMETHODIPOPTIONS_H
#define POSITIONINGMETHODIPOPTIONS_H

#include <QUuid>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioningmethodipprovider.h>
#include "definitions/optionvalues.h"

#include "ui_positioningmethodipoptions.h"

namespace Ui {
class PositioningMethodIpOptions;
}

class PositioningMethodIpOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
	explicit PositioningMethodIpOptions(QHash<QUuid, IPositioningMethodIpProvider *> AProviders, QWidget *parent = 0);
	~PositioningMethodIpOptions();
    virtual QWidget* instance() { return this; }
	Ui::PositioningMethodIpOptions *ui;
    
public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected slots:
	void onValueChanged(int AValue);

protected:
    void changeEvent(QEvent *e);
};

#endif // POSITIONINGMETHODIPOPTIONS_H

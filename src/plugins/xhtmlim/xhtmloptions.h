#ifndef XHTMLOPTIONS_H
#define XHTMLOPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>

#include "ui_xhtmloptions.h"

namespace Ui {
class XhtmlOptions;
}

class XhtmlIm;

class XhtmlOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
	XhtmlOptions(XhtmlIm *AXhtmlIm, QWidget *AParent = 0);
	~XhtmlOptions();
    virtual QWidget* instance() { return this; }

public slots:
    virtual void apply();
    virtual void reset();

protected slots:
	void onMaxEmbedChanged(int AValue);
	void onMaxAgeChanged(int AValue);

signals:
    void modified();
    void childApply();
    void childReset();

private:
	Ui::XhtmlOptions *ui;
	XhtmlIm*	FXhtmlIm;
};

#endif // XHTMLOPTIONS_H

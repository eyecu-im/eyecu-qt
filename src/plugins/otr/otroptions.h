#ifndef OTROPTIONS_H
#define OTROPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>

#include "otr.h"

namespace Ui {
class OtrOptions;
}

class OtrOptions:
		public QWidget,
		public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
public:
	OtrOptions(Otr* AOtr, QWidget *AParent = nullptr);
	~OtrOptions();

	// IOptionsDialogWidget interface
	virtual QWidget *instance() override;

public slots:
	virtual void apply() override;
	virtual void reset() override;

protected slots:
	void onKeysClicked();

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::OtrOptions*		ui;
	Otr*				FOtr;
	IPresenceManager*	FPresenceManager;
	IAccountManager*	FAccountManager;
	IOptionsManager*	FOptionsManager;
};

#endif // OTROPTIONS_H

#ifndef OMEMOOPTIONS_H
#define OMEMOOPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>
class Omemo;

namespace Ui {
class OmemoOptions;
}

class OmemoOptions:
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT

public:
	OmemoOptions(Omemo *AOmemo, QWidget *AParent = nullptr);
	~OmemoOptions();

	// IOptionsDialogWidget interface
	QWidget *instance();

public slots:
	void apply();
	void reset();

protected slots:
	void onKeysClicked();

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::OmemoOptions	*ui;
	Omemo				*FOmemo;
//	IPresenceManager	*FPresenceManager;
//	IAccountManager		*FAccountManager;
	IOptionsManager		*FOptionsManager;
};

#endif // OMEMOOPTIONS_H

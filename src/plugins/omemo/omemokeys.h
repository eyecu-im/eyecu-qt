#ifndef OMEMOKEYS_H
#define OMEMOKEYS_H

#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>

class Omemo;
class QStandardItemModel;

namespace Ui {
class OmemoKeys;
}

class OmemoKeys:
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT

public:
	OmemoKeys(Omemo *AOmemo, QWidget *AParent = nullptr);
	~OmemoKeys();

	// IOptionsDialogWidget
	QWidget *instance();

public slots:
	void apply();
	void reset();

protected slots:
	void onAccountIndexChanged(int AIndex);
	void onIdentityKeyCopy();

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::OmemoKeys		*ui;
	Omemo				*FOmemo;
	IPresenceManager	*FPresenceManager;
	IAccountManager		*FAccountManager;
	IOptionsManager		*FOptionsManager;

	QStandardItemModel	*FPreKeysModel;
	QStandardItemModel	*FIdentityKeysModel;
};

#endif // OMEMOKEYS_H

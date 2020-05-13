#ifndef OMEMOKEYS_H
#define OMEMOKEYS_H

#include <QSet>
#include <QItemSelection>

#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>

class Omemo;
class QStandardItemModel;
class SignalProtocol;
class Action;

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

protected:
	void updateIdentityKeys();
	void setSelectedIdentityKeysTrusted(bool ATrusted);

protected slots:
	void onAccountIndexChanged(int AIndex);
	void onIdentityKeyCopy();
	void onRetractOtherClicked(bool AChecked);
	void onIdentityKeysSelectionChanged(const QItemSelection &ASelected,
										const QItemSelection &ADeselected);
	void onIdentityTrustClicked();
	void onIdentityUntrustClicked();
	void onIdentityContextMenu(const QPoint& APos);

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

	Action				*FIkTrust;
	Action				*FIkUntrust;

	SignalProtocol		*FSignalProtocol;

	QStandardItemModel	*FPreKeysModel;
	QStandardItemModel	*FIdentityKeysModel;
	QSet<QUuid>			FRetractDevices;
};

#endif // OMEMOKEYS_H

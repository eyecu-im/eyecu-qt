#ifndef OTRKEYS_H
#define OTRKEYS_H

#include <QStandardItemModel>
#include <QItemSelection>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>

#include "otr.h"

namespace Ui {
class OtrKeys;
}

class OtrKeys:
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	OtrKeys(Otr* AOtr, QWidget *AParent = nullptr);
	~OtrKeys();

	// IOptionsDialogWidget interface
	QWidget *instance();

public slots:
	void apply();
	void reset();

protected:
	void copyFingerprint(const QItemSelectionModel *AModel, int AColumn);
	void updatePrivKeys();
	void updatePrivKeyGenerateButton(int AIndex);

protected slots:
	void updateFingerprints();
	void onFingerprintDelete();
	void onFingerprintVerify();
	void onFingerprintCopy();
	void onFingerprintSelectionChanged(const QItemSelection &ASelected,
									   const QItemSelection &ADeselected);
	void onFingerprintContextMenu(const QPoint& APos);
	void onPrivKeyDelete();
	void onPrivKeyGenerate();
	void onPrivKeyCopyFingerprint();
	void onPrivKeySelectionChanged(const QItemSelection &ASelected,
								   const QItemSelection &ADeselected);
	void onPrivKeyContextMenu(const QPoint& APos);
	void onAccountIndexChanged(int AIndex);

	void onPrivKeyGenerated(const Jid &AStreamJid, const QString &AFingerprint);
//	void onPrivKeyGenerationFailed(const Jid &AStreamJid);

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::OtrKeys			*ui;
	Otr					*FOtr;
	IPresenceManager	*FPresenceManager;
	IAccountManager		*FAccountManager;
	QStandardItemModel	*FFingerprintsModel;
	QStandardItemModel	*FPrivKeyModel;
};

#endif // OTRKEYS_H

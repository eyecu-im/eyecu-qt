#ifndef OTROPTIONS_H
#define OTROPTIONS_H

#include <QStandardItemModel>
#include <QItemSelection>
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

protected:
	void copyFingerprint(const QItemSelectionModel *AModel, int AColumn);
	void updatePrivKeys();
	void updatePrivKeyGenerateButton(int AIndex);

protected slots:
	void updateFingerprints();
	void onFingerprintDelete();
	void onFingerprintVerify();
	void onFingerprintCopyFingerprint();
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
	Ui::OtrOptions*		ui;
	Otr*				FOtr;
	IPresenceManager*	FPresenceManager;
	IAccountManager*	FAccountManager;
	QStandardItemModel* FFingerprintsModel;
	QStandardItemModel* FPrivKeyModel;
};

#endif // OTROPTIONS_H

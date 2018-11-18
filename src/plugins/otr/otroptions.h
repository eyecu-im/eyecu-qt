#ifndef OTROPTIONS_H
#define OTROPTIONS_H

#include <QStandardItemModel>
#include <QItemSelection>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>
#include "otrmessaging.h"

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
	OtrOptions(OtrMessaging* AOtrMessaging, QWidget *AParent = 0);
	~OtrOptions();

	// IOptionsDialogWidget interface
	virtual QWidget *instance() override;

public slots:
	virtual void apply() override;
	virtual void reset() override;

protected:
	void copyFingerprint(const QItemSelectionModel *AModel, int AColumn);

protected slots:
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
signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::OtrOptions *ui;
	OtrMessaging*       FOtrMessaging;
	IPresenceManager*	FPresenceManager;
	IAccountManager*	FAccountManager;
	QStandardItemModel* FFingerprintsModel;
	QStandardItemModel* FPrivKeyModel;
	QList<OtrFingerprint>  FFingerprints;
	QHash<QString, QString>	FKeys;
};

#endif // OTROPTIONS_H

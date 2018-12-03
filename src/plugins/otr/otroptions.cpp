#include <QMessageBox>
#include <QModelIndex>
#include <QMenu>
#include <QClipboard>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include <utils/pluginhelper.h>
#include "otroptions.h"
#include "ui_otroptions.h"

OtrOptions::OtrOptions(Otr *AOtr, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OtrOptions),
	FOtr(AOtr),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>()),
	FFingerprintsModel(new QStandardItemModel(this)),
	FPrivKeyModel(new QStandardItemModel(this))
{
	ui->setupUi(this);
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	ui->pbFingerprintDelete->setIcon(menuicons->getIcon(MNI_EDIT_DELETE));
	ui->pbFingerprintVerify->setIcon(menuicons->getIcon(MNI_OTR_UNVERFIFIED));
	ui->pbFingerprintCopy->setIcon(menuicons->getIcon(MNI_EDIT_COPY));
	ui->pbPrivKeyGenerate->setIcon(menuicons->getIcon(MNI_EDIT_ADD));
	ui->pbPrivKeyDelete->setIcon(menuicons->getIcon(MNI_EDIT_DELETE));
	ui->pbPrivKeyCopy->setIcon(menuicons->getIcon(MNI_EDIT_COPY));

	// Fingerprints
	ui->tvFingerprints->setShowGrid(true);
	ui->tvFingerprints->setEditTriggers(nullptr);
	ui->tvFingerprints->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tvFingerprints->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->tvFingerprints->setSortingEnabled(true);

	FFingerprintsModel->setColumnCount(4);
	FFingerprintsModel->setHorizontalHeaderLabels(QStringList() << tr("Account") <<
												  tr("User") << tr("Fingerprint") <<
												  tr("Verified"));
	ui->tvFingerprints->setModel(FFingerprintsModel);

	ui->tvPrivateKeys->setShowGrid(true);
	ui->tvPrivateKeys->setEditTriggers(nullptr);
	ui->tvPrivateKeys->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tvPrivateKeys->setSortingEnabled(true);

	ui->tvPrivateKeys->setContextMenuPolicy(Qt::CustomContextMenu);

	FPrivKeyModel->setColumnCount(2);
	FPrivKeyModel->setHorizontalHeaderLabels(QStringList() << tr("Account")
														   << tr("Fingerprint"));
	ui->tvPrivateKeys->setModel(FPrivKeyModel);
	reset();

	connect(ui->cmbPolicy, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->cbEndWhenOffline, SIGNAL(toggled(bool)), SIGNAL(modified()));
	connect(ui->tvFingerprints->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
												  SLOT(onFingerprintSelectionChanged(QItemSelection,QItemSelection)));
	connect(ui->tvPrivateKeys->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
												  SLOT(onPrivKeySelectionChanged(QItemSelection,QItemSelection)));
	connect(FOtr, SIGNAL(privKeyGenerated(QString, QString)),
				  SLOT(onPrivKeyGenerated(QString, QString)));
	connect(FOtr, SIGNAL(fingerprintsUpdated()),
				  SLOT(updateFingerprints()));
}

OtrOptions::~OtrOptions()
{
	delete ui;
}

QWidget *OtrOptions::instance()
{
	return this;
}

void OtrOptions::apply()
{
	Options::node(OPV_OTR_POLICY).setValue(ui->cmbPolicy->currentIndex());
	Options::node(OPV_OTR_ENDWHENOFFLINE).setValue(ui->cbEndWhenOffline->isChecked());
	emit childApply();
}

void OtrOptions::reset()
{
	ui->cmbPolicy->setCurrentIndex(Options::node(OPV_OTR_POLICY).value().toInt());
	ui->cbEndWhenOffline->setChecked(Options::node(OPV_OTR_ENDWHENOFFLINE).value().toBool());

	// Fingerprints
	updateFingerprints();

	// Update account list
	ui->cmbAccount->clear();
	QList<IPresence*> presences = FPresenceManager->presences();
	for (QList<IPresence*>::ConstIterator it=presences.constBegin(); it!=presences.constEnd(); ++it)
	{
		QString id =  FAccountManager->findAccountByStream((*it)->streamJid())->accountId().toString();
		IAccount *account = FAccountManager->findAccountByStream((*it)->streamJid());
		ui->cmbAccount->addItem(account->name(), id);
	}

	// Private keys
	updatePrivKeys();

	emit childReset();
}

void OtrOptions::copyFingerprint(const QItemSelectionModel *AModel, int AColumn)
{
	if (!AModel->hasSelection())
		return;

	QString text;
	foreach(QModelIndex selectIndex, AModel->selectedRows(1))
	{
		if (!text.isEmpty())
			text += "\n";
		text += FPrivKeyModel->item(selectIndex.row(), AColumn)->text();
	}
	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(text);
}

void OtrOptions::updatePrivKeys()
{
	FPrivKeyModel->setRowCount(0);
	QHash<QString, QString>	FKeys = FOtr->getPrivateKeys();
	for (QHash<QString, QString>::iterator keyIt = FKeys.begin(); keyIt != FKeys.end(); ++keyIt)
	{
		QList<QStandardItem*> row;

		QStandardItem* accItem = new QStandardItem(FOtr->humanAccount(keyIt.key()));
		accItem->setData(keyIt.key());

		row.append(accItem);
		row.append(new QStandardItem(keyIt.value()));

		FPrivKeyModel->appendRow(row);
	}

//TODO: Check, if we really need it
	int pkSortSection         = ui->tvPrivateKeys->horizontalHeader()->sortIndicatorSection();
	Qt::SortOrder pkSortOrder = ui->tvPrivateKeys->horizontalHeader()->sortIndicatorOrder();

	ui->tvPrivateKeys->sortByColumn(pkSortSection, pkSortOrder);
	ui->tvPrivateKeys->resizeColumnsToContents();

	updatePrivKeyGenerateButton(ui->cmbAccount->currentIndex());
}

void OtrOptions::updatePrivKeyGenerateButton(int AIndex)
{
	if (AIndex > -1)
	{
		QHash<QString, QString>	FKeys = FOtr->getPrivateKeys();
		QString accountId(ui->cmbAccount->itemData(AIndex).toString());
		if (FKeys.contains(accountId))
			ui->pbPrivKeyGenerate->setIcon(QApplication::style()->standardPixmap(QStyle::SP_BrowserReload));
		else
			ui->pbPrivKeyGenerate->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_EDIT_ADD));
	}
}

void OtrOptions::updateFingerprints()
{
	FFingerprintsModel->setRowCount(0);
	QList<OtrFingerprint> fingerprints = FOtr->getFingerprints();
	QListIterator<OtrFingerprint> fingerprintIt(fingerprints);
	int fpIndex = 0;
	while(fingerprintIt.hasNext())
	{
		QList<QStandardItem*> row;
		OtrFingerprint fp = fingerprintIt.next();

		QStandardItem* item = new QStandardItem(FOtr->humanAccount(fp.account));
		item->setData(fpIndex);

		row.append(item);
		row.append(new QStandardItem(fp.username));
		row.append(new QStandardItem(fp.fingerprintHuman));
		row.append(new QStandardItem(fp.trust));
		FFingerprintsModel->appendRow(row);

		fpIndex++;
	}

//TODO: Check, if we really need it
	int sortSection         = ui->tvFingerprints->horizontalHeader()->sortIndicatorSection();
	Qt::SortOrder sortOrder = ui->tvFingerprints->horizontalHeader()->sortIndicatorOrder();
	ui->tvFingerprints->sortByColumn(sortSection, sortOrder);
	ui->tvFingerprints->resizeColumnsToContents();
}

void OtrOptions::onFingerprintDelete()
{
	if (!ui->tvFingerprints->selectionModel()->hasSelection())
		return;

	QList<OtrFingerprint> fingerprints = FOtr->getFingerprints();

	QModelIndexList selectedRows = ui->tvFingerprints->selectionModel()->selectedRows();
	for (QModelIndexList::ConstIterator it = selectedRows.constBegin();
		 it != selectedRows.constEnd(); ++it)
		FOtr->deleteFingerprint(fingerprints[it->data(Qt::UserRole+1).toInt()]);
	updateFingerprints();
}

void OtrOptions::onFingerprintVerify()
{
	if (!ui->tvFingerprints->selectionModel()->hasSelection())
		return;

	foreach(QModelIndex selectIndex, ui->tvFingerprints->selectionModel()->selectedRows())
	{
		int fpIndex = FFingerprintsModel->item(selectIndex.row(), 0)->data().toInt();

		QList<OtrFingerprint> fingerprints = FOtr->getFingerprints();

		QString msg(tr("Have you verified that this is in fact the correct fingerprint?") + "\n\n" +
					tr("Account: ") + FOtr->humanAccount(fingerprints[fpIndex].account) + "\n" +
					tr("User: ") + fingerprints[fpIndex].username + "\n" +
					tr("Fingerprint: ") + fingerprints[fpIndex].fingerprintHuman);

		QMessageBox mb(QMessageBox::Question, tr("Off-the-Record Messaging"), msg,
					   QMessageBox::Yes | QMessageBox::No, this,
					   Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

		FOtr->verifyFingerprint(fingerprints[fpIndex],
								 (mb.exec() == QMessageBox::Yes));
	}
	//	updateData();
}

void OtrOptions::onFingerprintCopyFingerprint()
{
	copyFingerprint(ui->tvFingerprints->selectionModel(), 0);
}

void OtrOptions::onFingerprintSelectionChanged(const QItemSelection &ASelected, const QItemSelection &ADeselected)
{
	Q_UNUSED(ASelected)
	Q_UNUSED(ADeselected)

	bool disabled = qobject_cast<QItemSelectionModel *>(sender())->selection().isEmpty();
	ui->pbFingerprintDelete->setDisabled(disabled);
	ui->pbFingerprintVerify->setDisabled(disabled);
	ui->pbFingerprintCopy->setDisabled(disabled);
}

void OtrOptions::onFingerprintContextMenu(const QPoint &APos)
{
	QModelIndex index = ui->tvFingerprints->indexAt(APos);
	if (!index.isValid())
		return;
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
//TODO: Use Menu instead of QMenu here
	QMenu* menu = new QMenu(this);

	menu->addAction(menuicons->getIcon(MNI_EDIT_DELETE), tr("Delete"), this, SLOT(onFingerprintDelete()));
	menu->addAction(menuicons->getIcon(MNI_OTR_UNVERFIFIED), tr("Verify fingerprint"), this, SLOT(onFingerprintVerify()));
	menu->addAction(menuicons->getIcon(MNI_EDIT_COPY), tr("Copy fingerprint"), this, SLOT(onFingerprintCopyFingerprint()));

	menu->exec(QCursor::pos());
}

void OtrOptions::onPrivKeyContextMenu(const QPoint &APos)
{
	QModelIndex index = ui->tvPrivateKeys->indexAt(APos);
	if (!index.isValid())
		return;
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
//TODO: Use Menu instead of QMenu here
	QMenu* menu = new QMenu(this);

	menu->addAction(menuicons->getIcon(MNI_EDIT_DELETE), tr("Delete"), this, SLOT(onPrivKeyDelete()));
	menu->addAction(menuicons->getIcon(MNI_EDIT_COPY), tr("Copy fingerprint"), this, SLOT(onPrivKeyCopyFingerprint()));

	menu->exec(QCursor::pos());
}

void OtrOptions::onAccountIndexChanged(int AIndex)
{
	ui->pbPrivKeyGenerate->setDisabled(AIndex==-1);
	updatePrivKeyGenerateButton(AIndex);
}

void OtrOptions::onPrivKeyGenerated(const QString &AAccount, const QString &AFingerprint)
{
	QList<QStandardItem *> items = FPrivKeyModel->findItems(AAccount);

	int rowCount = FPrivKeyModel->rowCount();
	int row;
	for (row = 0; row < rowCount; ++row)
		if (FPrivKeyModel->item(row)->data() == AAccount)
		{
			FPrivKeyModel->item(row, 1)->setText(AFingerprint);
			break;
		}

	if (row==rowCount) // Not found
		updatePrivKeys();
}

void OtrOptions::onPrivKeyGenerationFailed(const QString &AAccount)
{
	Q_UNUSED(AAccount)
}

void OtrOptions::onPrivKeyDelete()
{
	if (!ui->tvPrivateKeys->selectionModel()->hasSelection())
		return;
	QModelIndexList selectedRows = ui->tvPrivateKeys->selectionModel()->selectedRows();
	for (QModelIndexList::ConstIterator it = selectedRows.constBegin();
		 it != selectedRows.constEnd(); ++it)
		FOtr->deleteKey(it->data(Qt::UserRole+1).toString());
	updatePrivKeys();
}

void OtrOptions::onPrivKeyGenerate()
{
	int accountIndex = ui->cmbAccount->currentIndex();
	if (accountIndex == -1)
		return;

	QString accountName(ui->cmbAccount->currentText());
	QString accountId(ui->cmbAccount->itemData(accountIndex).toString());

	QHash<QString, QString>	FKeys = FOtr->getPrivateKeys();
	if (FKeys.contains(accountId))
	{
		QString msg(tr("Are you sure you want to overwrite the following key?") + "\n\n" +
					tr("Account: ") + accountName + "\n" +
					tr("Fingerprint: ") + FKeys.value(accountId));

		QMessageBox mb(QMessageBox::Question, tr("Off-the-Record Messaging"), msg,
					   QMessageBox::Yes | QMessageBox::No, this,
					   Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

		if (mb.exec() == QMessageBox::No)
			return;
	}

	FOtr->generateKey(accountId);
}

void OtrOptions::onPrivKeyCopyFingerprint()
{
	copyFingerprint(ui->tvPrivateKeys->selectionModel(), 1);
}

void OtrOptions::onPrivKeySelectionChanged(const QItemSelection &ASelected, const QItemSelection &ADeselected)
{
	Q_UNUSED(ASelected)
	Q_UNUSED(ADeselected)

	bool disable = qobject_cast<QItemSelectionModel *>(sender())->selection().isEmpty();
	ui->pbPrivKeyDelete->setDisabled(disable);
	ui->pbPrivKeyCopy->setDisabled(disable);
}

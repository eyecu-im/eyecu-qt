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

OtrOptions::OtrOptions(OtrMessaging *AOtrMessaging, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OtrOptions),
	FOtrMessaging(AOtrMessaging),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>()),
	FFingerprintsModel(new QStandardItemModel(this)),
	FPrivKeyModel(new QStandardItemModel(this))
{
	ui->setupUi(this);
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	ui->pbFingerprintDelete->setIcon(menuicons->getIcon(MNI_EDIT_DELETE));
	ui->pbFingerprintVerify->setIcon(menuicons->getIcon(MNI_PEPMANAGER));
	ui->pbFingerprintCopy->setIcon(menuicons->getIcon(MNI_EDIT_COPY));
	ui->pbPrivKeyGenerate->setIcon(menuicons->getIcon(MNI_EDIT_ADD));
	ui->pbPrivKeyDelete->setIcon(menuicons->getIcon(MNI_EDIT_DELETE));
	ui->pbPrivKeyCopy->setIcon(menuicons->getIcon(MNI_EDIT_COPY));

	// Fingerprints
	ui->tvFingerprints->setShowGrid(true);
	ui->tvFingerprints->setEditTriggers(0);
	ui->tvFingerprints->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tvFingerprints->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->tvFingerprints->setSortingEnabled(true);

	FFingerprintsModel->setColumnCount(5);
	FFingerprintsModel->setHorizontalHeaderLabels(QStringList() << tr("Account") <<
												  tr("User") << tr("Fingerprint") <<
												  tr("Verified") << tr("Status"));
	ui->tvFingerprints->setModel(FFingerprintsModel);

	ui->tvPrivateKeys->setShowGrid(true);
	ui->tvPrivateKeys->setEditTriggers(0);
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

	// Cleanup deleted indexes
	QSet<int> indexes;
	int count = FFingerprintsModel->rowCount();

	// List present indexes
	for (int i=0; i<count; ++i)
		indexes.insert(FFingerprintsModel->index(i,0).data(Qt::UserRole+1).toInt());

	int index = 0;
	for (QList<OtrFingerprint>::ConstIterator it = FFingerprints.constBegin();
		 it != FFingerprints.constEnd(); ++it, ++index)
		if (!indexes.contains(index)) // Deleted
			FOtrMessaging->deleteFingerprint(*it);

	// Cleanup deleted indexes
	QSet<QString> ids;
	count = FPrivKeyModel->rowCount();

	// List present indexes
	for (int i=0; i<count; ++i)
		ids.insert(FPrivKeyModel->index(i,0).data(Qt::UserRole+1).toString());

	for (QHash<QString,QString>::ConstIterator it = FKeys.constBegin();
		 it != FKeys.constEnd(); ++it)
		if (!ids.contains(it.key())) // Deleted
			FOtrMessaging->deleteKey(it.key());

	emit childApply();
}

void OtrOptions::reset()
{
	ui->cmbPolicy->setCurrentIndex(Options::node(OPV_OTR_POLICY).value().toInt());
	ui->cbEndWhenOffline->setChecked(Options::node(OPV_OTR_ENDWHENOFFLINE).value().toBool());

	// Fingerprints
	FFingerprints = FOtrMessaging->getFingerprints();
	QListIterator<OtrFingerprint> fingerprintIt(FFingerprints);
	int fpIndex = 0;
	while(fingerprintIt.hasNext())
	{
		QList<QStandardItem*> row;
		OtrFingerprint fp = fingerprintIt.next();

		QStandardItem* item = new QStandardItem(FOtrMessaging->humanAccount(fp.account));
		item->setData(fpIndex);

		row.append(item);
		row.append(new QStandardItem(fp.username));
		row.append(new QStandardItem(fp.fingerprintHuman));
		row.append(new QStandardItem(fp.trust));
		row.append(new QStandardItem(FOtrMessaging->getMessageStateString(fp.account,
																		  fp.username)));
		FFingerprintsModel->appendRow(row);

		fpIndex++;
	}

//	FTable->setModel(FTableModel);
//TODO: Check, if we really need it
	int sortSection         = ui->tvFingerprints->horizontalHeader()->sortIndicatorSection();
	Qt::SortOrder sortOrder = ui->tvFingerprints->horizontalHeader()->sortIndicatorOrder();
	ui->tvFingerprints->sortByColumn(sortSection, sortOrder);
	ui->tvFingerprints->resizeColumnsToContents();

	// Private keys
	ui->cmbAccount->clear();
	FKeys = FOtrMessaging->getPrivateKeys();
	QHash<QString, QString>::iterator keyIt;
	for (keyIt = FKeys.begin(); keyIt != FKeys.end(); ++keyIt)
	{
		QList<QStandardItem*> row;

		QStandardItem* accItem = new QStandardItem(FOtrMessaging->humanAccount(keyIt.key()));
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

	QList<IPresence*> presences = FPresenceManager->presences();
	for (QList<IPresence*>::ConstIterator it=presences.constBegin(); it!=presences.constEnd(); ++it)
	{
		QString id =  FAccountManager->findAccountByStream((*it)->streamJid())->accountId().toString();
		IAccount *account = FAccountManager->findAccountByStream((*it)->streamJid());
		ui->cmbAccount->addItem(account->name(), id);
	}

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

void OtrOptions::onFingerprintDelete()
{
	if (!ui->tvFingerprints->selectionModel()->hasSelection())
		return;

	foreach(QModelIndex selectIndex, ui->tvFingerprints->selectionModel()->selectedRows())
		FFingerprintsModel->removeRow(selectIndex.row());
}

void OtrOptions::onFingerprintVerify()
{
	if (!ui->tvFingerprints->selectionModel()->hasSelection())
	{
		return;
	}
	foreach(QModelIndex selectIndex, ui->tvFingerprints->selectionModel()->selectedRows())
	{
		int fpIndex = FFingerprintsModel->item(selectIndex.row(), 0)->data().toInt();

		QString msg(tr("Have you verified that this is in fact the correct fingerprint?") + "\n\n" +
					tr("Account: ") + FOtrMessaging->humanAccount(FFingerprints[fpIndex].account) + "\n" +
					tr("User: ") + FFingerprints[fpIndex].username + "\n" +
					tr("Fingerprint: ") + FFingerprints[fpIndex].fingerprintHuman);

		QMessageBox mb(QMessageBox::Question, tr("Off-the-Record Messaging"), msg,
					   QMessageBox::Yes | QMessageBox::No, this,
					   Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

		FOtrMessaging->verifyFingerprint(FFingerprints[fpIndex],
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
	menu->addAction(QIcon(":/otrplugin/otr_unverified.png"), tr("Verify fingerprint"), this, SLOT(onFingerprintVerify()));
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

	QString accountId(ui->cmbAccount->itemData(AIndex).toString());
	if (FKeys.contains(accountId))
		ui->pbPrivKeyGenerate->setIcon(QApplication::style()->standardPixmap(QStyle::SP_BrowserReload));
	else
		ui->pbPrivKeyGenerate->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_EDIT_ADD));
}

void OtrOptions::onPrivKeyDelete()
{
	if (!ui->tvPrivateKeys->selectionModel()->hasSelection())
		return;
	foreach(QModelIndex selectIndex, ui->tvPrivateKeys->selectionModel()->selectedRows())
		FPrivKeyModel->removeRow(selectIndex.row());
}

void OtrOptions::onPrivKeyGenerate()
{
	int accountIndex = ui->cmbAccount->currentIndex();
	if (accountIndex == -1)
		return;

	QString accountName(ui->cmbAccount->currentText());
	QString accountId(ui->cmbAccount->itemData(accountIndex).toString());

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

	FOtrMessaging->generateKey(accountId);
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

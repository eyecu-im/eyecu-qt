/*
 * psiotrconfig.cpp - Configuration dialogs
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *                    2011  Florian Fieber
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "otrconfig.h"
#include <utils/pluginhelper.h> // xnamed!
#include <utils/options.h>

#include <QGroupBox>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItem>
#include <QMessageBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QPoint>

ConfigDialog::ConfigDialog(OtrMessaging* AOtrMessaging, QWidget* AParent)
	: QWidget(AParent),
	  FOtrMessaging(AOtrMessaging),
      FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>()),
      FAccountManager(PluginHelper::pluginInstance<IAccountManager>())
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QTabWidget* tabWidget = new QTabWidget(this);

	tabWidget->addTab(new FingerprintWidget(FOtrMessaging, tabWidget),
                      tr("Known fingerprints"));

	tabWidget->addTab(new PrivKeyWidget(FOtrMessaging, tabWidget),
                      tr("My private keys"));

	tabWidget->addTab(new ConfigOtrWidget(FOtrMessaging, tabWidget),
                      tr("Configuration"));

    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    QVBoxLayout *vltLayout = new QVBoxLayout(this);
    vltLayout->setSpacing(0);
    vltLayout->setMargin(0);
    vltLayout->addWidget(this);
}

void ConfigDialog::apply()
{
    emit childApply();
}

void ConfigDialog::reset()
{
    emit childReset();
}

//=============================================================================

//ConfigOtrWidget::ConfigOtrWidget(IOptionsManager* optionHost,
ConfigOtrWidget::ConfigOtrWidget(OtrMessaging* AOtrMessaging,
								 QWidget* AParent)
	: QWidget(AParent),
	  FOtrMessaging(AOtrMessaging),
      FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>())
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox* policyGroup = new QGroupBox(tr("OTR Policy"), this);
    QVBoxLayout* policyLayout = new QVBoxLayout(policyGroup);

	FPolicy = new QButtonGroup(policyGroup);

    QRadioButton* polDisable = new QRadioButton(tr("Disable private messaging"), policyGroup);
    QRadioButton* polEnable  = new QRadioButton(tr("Manually start private messaging"), policyGroup);
    QRadioButton* polAuto    = new QRadioButton(tr("Automatically start private messaging"), policyGroup);
    QRadioButton* polRequire = new QRadioButton(tr("Require private messaging"), policyGroup);

	FEndWhenOffline = new QCheckBox(tr("End session when contact goes offline"), this);


	FPolicy->addButton(polDisable, IOtr::PolocyOff);
	FPolicy->addButton(polEnable,  IOtr::PolicyEnabled);
	FPolicy->addButton(polAuto,    IOtr::PolicyAuto);
	FPolicy->addButton(polRequire, IOtr::PolicyRequire);

    policyLayout->addWidget(polDisable);
    policyLayout->addWidget(polEnable);
    policyLayout->addWidget(polAuto);
    policyLayout->addWidget(polRequire);
    policyGroup->setLayout(policyLayout);

    layout->addWidget(policyGroup);
	layout->addWidget(FEndWhenOffline);
    layout->addStretch();

    setLayout(layout);

    int policyOption = Options::node(OPTION_POLICY).value().toInt();
	if ((policyOption < IOtr::PolocyOff) || (policyOption > IOtr::PolicyRequire))
    {
		policyOption = static_cast<int>(IOtr::PolicyEnabled);
    }

    bool endWhenOfflineOption = Options::node(OPTION_END_WHEN_OFFLINE).value().toBool();

	FPolicy->button(policyOption)->setChecked(true);

	FEndWhenOffline->setChecked(endWhenOfflineOption);

    updateOptions();

	connect(FPolicy, SIGNAL(buttonClicked(int)),
            SLOT(updateOptions()));

	connect(FEndWhenOffline, SIGNAL(stateChanged(int)),
            SLOT(updateOptions()));
}

// ---------------------------------------------------------------------------

void ConfigOtrWidget::updateOptions()
{
	IOtr::Policy policy = static_cast<IOtr::Policy>(FPolicy->checkedId());

    Options::node(OPTION_POLICY).setValue(static_cast<int>(policy));
    Options::node(OPTION_END_WHEN_OFFLINE).setValue(
									FEndWhenOffline->checkState() == Qt::Checked);
	FOtrMessaging->setPolicy(policy);
}

//=============================================================================

FingerprintWidget::FingerprintWidget(OtrMessaging* otr, QWidget* parent)
    : QWidget(parent),
	  FOtrMessaging(otr),
	  FTable(new QTableView(this)),
	  FTableModel(new QStandardItemModel(this)),
	  FFingerprints()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

	FTable->setShowGrid(true);
	FTable->setEditTriggers(0);
	FTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	FTable->setContextMenuPolicy(Qt::CustomContextMenu);
	FTable->setSortingEnabled(true);

	connect(FTable, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(contextMenu(const QPoint&)));

	mainLayout->addWidget(FTable);

    QPushButton* deleteButton = new QPushButton(tr("Delete fingerprint"), this);
    QPushButton* verifyButton = new QPushButton(tr("Verify fingerprint"), this);
    connect(deleteButton,SIGNAL(clicked()),SLOT(deleteFingerprint()));
    connect(verifyButton,SIGNAL(clicked()),SLOT(verifyFingerprint()));
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(verifyButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    updateData();
}

//-----------------------------------------------------------------------------

void FingerprintWidget::updateData()
{
	int sortSection         = FTable->horizontalHeader()->sortIndicatorSection();
	Qt::SortOrder sortOrder = FTable->horizontalHeader()->sortIndicatorOrder();

	FTableModel->clear();
	FTableModel->setColumnCount(5);
	FTableModel->setHorizontalHeaderLabels(QStringList() << tr("Account")
                                            << tr("User") << tr("Fingerprint")
                                            << tr("Verified") << tr("Status"));

	FFingerprints = FOtrMessaging->getFingerprints();
	QListIterator<OtrFingerprint> fingerprintIt(FFingerprints);
    int fpIndex = 0;
    while(fingerprintIt.hasNext())
    {
        QList<QStandardItem*> row;
        OtrFingerprint fp = fingerprintIt.next();

		QStandardItem* item = new QStandardItem(FOtrMessaging->humanAccount(fp.account));
        item->setData(QVariant(fpIndex));

        row.append(item);
        row.append(new QStandardItem(fp.username));
        row.append(new QStandardItem(fp.fingerprintHuman));
        row.append(new QStandardItem(fp.trust));
		row.append(new QStandardItem(FOtrMessaging->getMessageStateString(fp.account,
                                                                  fp.username)));

		FTableModel->appendRow(row);

        fpIndex++;
    }

	FTable->setModel(FTableModel);

	FTable->sortByColumn(sortSection, sortOrder);
	FTable->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------
//** slots **

void FingerprintWidget::deleteFingerprint()
{
	if (!FTable->selectionModel()->hasSelection())
    {
        return;
    }
	foreach(QModelIndex selectIndex, FTable->selectionModel()->selectedRows())
    {
		int fpIndex = FTableModel->item(selectIndex.row(), 0)->data().toInt();

        QString msg(tr("Are you sure you want to delete the following fingerprint?") + "\n\n" +
					tr("Account: ") + FOtrMessaging->humanAccount(FFingerprints[fpIndex].account) + "\n" +
					tr("User: ") + FFingerprints[fpIndex].username + "\n" +
					tr("Fingerprint: ") + FFingerprints[fpIndex].fingerprintHuman);

		QMessageBox mb(QMessageBox::Question, tr("Off-the-Record Messaging"), msg,
                       QMessageBox::Yes | QMessageBox::No, this,
                       Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

        if (mb.exec() == QMessageBox::Yes)
        {
			FOtrMessaging->deleteFingerprint(FFingerprints[fpIndex]);
        }
    }
    updateData();
}

//-----------------------------------------------------------------------------

void FingerprintWidget::verifyFingerprint()
{
	if (!FTable->selectionModel()->hasSelection())
    {
        return;
    }
	foreach(QModelIndex selectIndex, FTable->selectionModel()->selectedRows())
    {
		int fpIndex = FTableModel->item(selectIndex.row(), 0)->data().toInt();

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
    updateData();
}

//-----------------------------------------------------------------------------

void FingerprintWidget::copyFingerprint()
{
	if (!FTable->selectionModel()->hasSelection())
    {
        return;
    }
    QString text;
	foreach(QModelIndex selectIndex, FTable->selectionModel()->selectedRows(1))
    {
		int fpIndex = FTableModel->item(selectIndex.row(), 0)->data().toInt();

        if (!text.isEmpty())
        {
            text += "\n";
        }
		text += FFingerprints[fpIndex].fingerprintHuman;
    }
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

//-----------------------------------------------------------------------------

void FingerprintWidget::contextMenu(const QPoint& APos)
{
	QModelIndex index = FTable->indexAt(APos);
    if (!index.isValid())
    {
        return;
    }

    QMenu* menu = new QMenu(this);

    menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this, SLOT(deleteFingerprint()));
    menu->addAction(QIcon(":/otrplugin/otr_unverified.png"), tr("Verify fingerprint"), this, SLOT(verifyFingerprint()));
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy fingerprint"), this, SLOT(copyFingerprint()));

    menu->exec(QCursor::pos());
}

//=============================================================================

//PrivKeyWidget::PrivKeyWidget(AccountInfoAccessingHost* accountInfo,
//                             OtrMessaging* otr, QWidget* parent)
PrivKeyWidget::PrivKeyWidget(OtrMessaging* FOtrMessaging, QWidget* parent)
    : QWidget(parent),
	  FOtrMessaging(FOtrMessaging),
	  FTable(new QTableView(this)),
	  FTableModel(new QStandardItemModel(this)),
	  FKeys(),
      FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>()),
      FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
      FAccountManager(PluginHelper::pluginInstance<IAccountManager>())
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

	FAccountBox = new QComboBox(this);

    QList<IPresence*> presences = FPresenceManager->presences();
    for (QList<IPresence*>::ConstIterator it=presences.constBegin(); it!=presences.constEnd(); ++it)
    {
        QString id =  FAccountManager->findAccountByStream((*it)->streamJid())->accountId().toString();
        IAccount *account = FAccountManager->findAccountByStream((*it)->streamJid());
		FAccountBox->addItem(account->name(), QVariant(id));
    }

    QPushButton* generateButton = new QPushButton(tr("Generate new key"), this);
    connect(generateButton,SIGNAL(clicked()),SLOT(generateKey()));

    QHBoxLayout* generateLayout = new QHBoxLayout();
	generateLayout->addWidget(FAccountBox);
    generateLayout->addWidget(generateButton);

    mainLayout->addLayout(generateLayout);
	mainLayout->addWidget(FTable);

    QPushButton* deleteButton = new QPushButton(tr("Delete key"), this);
    connect(deleteButton,SIGNAL(clicked()),SLOT(deleteKey()));

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(deleteButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

	FTable->setShowGrid(true);
	FTable->setEditTriggers(0);
	FTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	FTable->setSortingEnabled(true);

	FTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(FTable, SIGNAL(customContextMenuRequested(const QPoint&)),
					SLOT(contextMenu(const QPoint&)));
    updateData();
}

//-----------------------------------------------------------------------------

void PrivKeyWidget::updateData()
{
	int sortSection         = FTable->horizontalHeader()->sortIndicatorSection();
	Qt::SortOrder sortOrder = FTable->horizontalHeader()->sortIndicatorOrder();

	FTableModel->clear();
	FTableModel->setColumnCount(2);
	FTableModel->setHorizontalHeaderLabels(QStringList() << tr("Account")
                                                          << tr("Fingerprint"));

	FKeys = FOtrMessaging->getPrivateKeys();
    QHash<QString, QString>::iterator keyIt;
	for (keyIt = FKeys.begin(); keyIt != FKeys.end(); ++keyIt)
    {
        QList<QStandardItem*> row;

		QStandardItem* accItem = new QStandardItem(FOtrMessaging->humanAccount(keyIt.key()));
        accItem->setData(QVariant(keyIt.key()));

        row.append(accItem);
        row.append(new QStandardItem(keyIt.value()));

		FTableModel->appendRow(row);
    }

	FTable->setModel(FTableModel);

	FTable->sortByColumn(sortSection, sortOrder);
	FTable->resizeColumnsToContents();
}

//-----------------------------------------------------------------------------

void PrivKeyWidget::deleteKey()
{
	if (!FTable->selectionModel()->hasSelection())
    {
        return;
    }
	foreach(QModelIndex selectIndex, FTable->selectionModel()->selectedRows(1))
    {
		QString fpr(FTableModel->item(selectIndex.row(), 1)->text());
		QString account(FTableModel->item(selectIndex.row(), 0)->data().toString());

        QString msg(tr("Are you sure you want to delete the following key?") + "\n\n" +
					tr("Account: ") + FOtrMessaging->humanAccount(account) + "\n" +
                    tr("Fingerprint: ") + fpr);

		QMessageBox mb(QMessageBox::Question, tr("Off-the-Record Messaging"), msg,
                       QMessageBox::Yes | QMessageBox::No, this,
                       Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

        if (mb.exec() == QMessageBox::Yes)
        {
			FOtrMessaging->deleteKey(account);
        }
    }
    updateData();
}

//-----------------------------------------------------------------------------

void PrivKeyWidget::generateKey()
{
	int accountIndex = FAccountBox->currentIndex();

    if (accountIndex == -1)
    {
        return;
    }

	QString accountName(FAccountBox->currentText());
	QString accountId(FAccountBox->itemData(accountIndex).toString());

	if (FKeys.contains(accountId))
    {
        QString msg(tr("Are you sure you want to overwrite the following key?") + "\n\n" +
                    tr("Account: ") + accountName + "\n" +
					tr("Fingerprint: ") + FKeys.value(accountId));

		QMessageBox mb(QMessageBox::Question, tr("Off-the-Record Messaging"), msg,
                       QMessageBox::Yes | QMessageBox::No, this,
                       Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

        if (mb.exec() == QMessageBox::No)
        {
            return;
        }
    }

	FOtrMessaging->generateKey(accountId);

    updateData();
}

//-----------------------------------------------------------------------------

void PrivKeyWidget::copyFingerprint()
{
	if (!FTable->selectionModel()->hasSelection())
    {
        return;
    }
    QString text;
	foreach(QModelIndex selectIndex, FTable->selectionModel()->selectedRows(1))
    {
        if (!text.isEmpty())
        {
            text += "\n";
        }
		text += FTableModel->item(selectIndex.row(), 1)->text();
    }
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

//-----------------------------------------------------------------------------

void PrivKeyWidget::contextMenu(const QPoint& APos)
{
	QModelIndex index = FTable->indexAt(APos);
    if (!index.isValid())
    {
        return;
    }

    QMenu* menu = new QMenu(this);

    menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"), this, SLOT(deleteKey()));
    menu->addAction(QIcon::fromTheme("edit-copy"), tr("Copy fingerprint"), this, SLOT(copyFingerprint()));

    menu->exec(QCursor::pos());
}

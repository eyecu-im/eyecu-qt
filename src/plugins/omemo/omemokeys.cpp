#include <QClipboard>

#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include <utils/pluginhelper.h>

#include "omemo.h"
#include "omemokeys.h"
#include "ui_omemokeys.h"

#define TAG_ACCOUNT "account"

#define COL_NAME		0
#define COL_ID			1
#define COL_FINGERPRIT	2
#define COL_TRUSTED		3

#define IDR_KEY_DATA	Qt::UserRole+1

OmemoKeys::OmemoKeys(Omemo *AOmemo, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OmemoKeys),
	FOmemo(AOmemo),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>()),
	FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>()),
	FSignalProtocol(nullptr),
	FPreKeysModel(new QStandardItemModel(this)),
	FIdentityKeysModel(new QStandardItemModel(this))
{
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

	ui->setupUi(this);

	ui->pbPublicIdentityKeyCopy->setIcon(menuicons->getIcon(MNI_EDIT_COPY));
	ui->pbTrust->setIcon(menuicons->getIcon(MNI_RCHANGER_SUBSCRIBE));
	ui->pbUntrust->setIcon(menuicons->getIcon(MNI_RCHANGER_UNSUBSCRIBE));

	FPreKeysModel->setColumnCount(2);
	FPreKeysModel->setHorizontalHeaderLabels(QStringList() << tr("Id")
														   << tr("Key data"));
	ui->tvPreKeys->setModel(FPreKeysModel);
	ui->tvPreKeys->setColumnWidth(0, 31);
	ui->tvPreKeys->setColumnWidth(1, 360);

	FIdentityKeysModel->setColumnCount(3);
	FIdentityKeysModel->setHorizontalHeaderLabels(QStringList() << tr("Name")
																<< tr("Device ID")
																<< tr("Fingerprint")
																<< tr("Trusted"));
	ui->tvIdentityKeys->setModel(FIdentityKeysModel);
	ui->tvIdentityKeys->setColumnWidth(0, 128);
	ui->tvIdentityKeys->setColumnWidth(1, 96);
	ui->tvIdentityKeys->setColumnWidth(2, 400);
	ui->tvIdentityKeys->setColumnWidth(3, 48);
	ui->tvIdentityKeys->setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(ui->tvIdentityKeys->selectionModel(),
			SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			SLOT(onIdentityKeysSelectionChanged(QItemSelection,QItemSelection)));

	reset();
}

OmemoKeys::~OmemoKeys()
{
	delete ui;
}

QWidget *OmemoKeys::instance()
{
	return this;
}

void OmemoKeys::apply()
{
	OptionsNode retract = Options::node(OPV_OMEMO_RETRACT);
	QStringList ns = retract.childNSpaces(TAG_ACCOUNT);

	for (QStringList::ConstIterator it = ns.constBegin();
		 it != ns.constEnd(); ++it)
		if (!FRetractDevices.contains(QUuid(*it)))
			retract.removeNode(TAG_ACCOUNT, *it);

	for (QSet<QUuid>::ConstIterator it = FRetractDevices.constBegin();
		 it != FRetractDevices.constEnd(); ++it)
		if (ui->cbRetractOther->isChecked())
			retract.node(TAG_ACCOUNT, it->toString()).setValue(true);

	emit childApply();
}

void OmemoKeys::reset()
{
	FRetractDevices.clear();
	ui->cmbAccount->clear();

	QList<IAccount*> accounts = FAccountManager->accounts();

	OptionsNode retract = Options::node(OPV_OMEMO_RETRACT);
	QStringList ns = retract.childNSpaces(TAG_ACCOUNT);

	for (QList<IAccount*>::ConstIterator it = accounts.constBegin();
		 it != accounts.constEnd(); ++it)
	{
		QUuid uuid = (*it)->accountId();
		if (ns.contains(uuid.toString()) &&
			retract.value(TAG_ACCOUNT, uuid.toString()).toBool())
			FRetractDevices.insert(uuid);
		ui->cmbAccount->addItem((*it)->name(), uuid.toByteArray());
	}

	if (ui->cmbAccount->count())
		onAccountIndexChanged(ui->cmbAccount->currentIndex());

	emit childReset();
}

void OmemoKeys::updateIdentityKeys()
{
	FIdentityKeysModel->removeRows(0, FIdentityKeysModel->rowCount());
	QList<OmemoStore::IdentityKey> identityKeys = FSignalProtocol->getIdentityKeys();

	for (QList<OmemoStore::IdentityKey>::ConstIterator it=identityKeys.constBegin();
		 it != identityKeys.constEnd(); ++it)
	{
		QList<QStandardItem*> row;

		QStandardItem* dataItem = new QStandardItem(SignalProtocol::calcFingerprint(
													FSignalProtocol->curveFromEd(it->keyData)));
		dataItem->setData(it->keyData, IDR_KEY_DATA);

		row.append(new QStandardItem(it->name));
		row.append(new QStandardItem(QString::number(it->deviceId)));
		row.append(dataItem);
		row.append(new QStandardItem(QString::number(it->trusted)));
		FIdentityKeysModel->appendRow(row);
	}
}

void OmemoKeys::setSelectedIdentityKeysTrusted(bool ATrusted)
{
	if (FSignalProtocol)
	{
		QModelIndexList list = ui->tvIdentityKeys->selectionModel()->selection().indexes();

		QByteArray keyData;
		QString bareJid;
		quint32	deviceId;

		for (QModelIndexList::ConstIterator it = list.constBegin();
			 it != list.constEnd(); ++it)
			if (it->column()==COL_NAME)
				bareJid = it->data().toString();
			else if (it->column()==COL_ID)
				deviceId = it->data().toUInt();
			else if (it->column()==COL_FINGERPRIT)
				keyData = it->data(IDR_KEY_DATA).toByteArray();
			else if (it->column()==COL_TRUSTED)
			{
				int trusted = it->data().toInt();
				if (trusted != ATrusted)
					FSignalProtocol->setIdentityTrusted(bareJid, deviceId, keyData, ATrusted);
			}
		updateIdentityKeys();
	}
}

void OmemoKeys::onAccountIndexChanged(int AIndex)
{
	ui->pbPublicIdentityKeyCopy->setDisabled(true);
	QUuid uuid = ui->cmbAccount->itemData(AIndex).toString();
	IAccount *account = FAccountManager->findAccountById(uuid);
	if (account)
	{
		FSignalProtocol = FOmemo->signalProtocol(account->streamJid());
		ui->cbRetractOther->setChecked(FRetractDevices.contains(uuid));		
		if (FSignalProtocol)
		{
			quint32 deviceId = FSignalProtocol->getDeviceId();
			ui->lblDeviceId->setNum(int(deviceId));

			QString fingerprint = SignalProtocol::calcFingerprint(FSignalProtocol->getIdentityKeyPublic(true));
			ui->lblPublicIdentityKey->setText(fingerprint);
			if (!fingerprint.isEmpty())
				ui->pbPublicIdentityKeyCopy->setEnabled(true);

			FPreKeysModel->removeRows(0, FPreKeysModel->rowCount());
			QMap<quint32, QByteArray> preKeys = FSignalProtocol->getPreKeys();

			for (QMap<quint32, QByteArray>::ConstIterator it=preKeys.constBegin();
				 it != preKeys.constEnd(); ++it)
			{
				QList<QStandardItem*> row;

				QStandardItem* dataItem = new QStandardItem(SignalProtocol::calcFingerprint(
															FSignalProtocol->curveFromEd(*it)));
				dataItem->setData(*it);

				row.append(new QStandardItem(QString::number(it.key())));
				row.append(dataItem);
				FPreKeysModel->appendRow(row);
			}

			updateIdentityKeys();

			onIdentityKeysSelectionChanged(QItemSelection(), QItemSelection());
		}
	}
}

void OmemoKeys::onIdentityKeyCopy()
{
	QApplication::clipboard()->setText(ui->lblPublicIdentityKey->text());
}

void OmemoKeys::onRetractOtherClicked(bool AChecked)
{
	QString uuid = ui->cmbAccount->itemData(ui->cmbAccount->currentIndex()).toString();
	if (AChecked)
		FRetractDevices.insert(uuid);
	else
		FRetractDevices.remove(uuid);

	emit modified();
}

void OmemoKeys::onIdentityKeysSelectionChanged(const QItemSelection &ASelected, const QItemSelection &ADeselected)
{
	QModelIndexList list = ASelected.indexes();
	if (list.isEmpty())
	{
		ui->pbTrust->setDisabled(true);
		ui->pbUntrust->setDisabled(true);
	}
	else
	{
		bool haveTrusted(false);
		bool haveUntrusted(false);

		for (QModelIndexList::ConstIterator it = list.constBegin();
			 it != list.constEnd(); ++it)
			if (it->column()==COL_TRUSTED)
			{
				int trusted = it->data().toInt();
				if (trusted==1)
					haveTrusted = true;
				else
					haveUntrusted = true;
				if (haveTrusted && haveUntrusted)
					break;
			}

		ui->pbTrust->setEnabled(haveUntrusted);
		ui->pbUntrust->setEnabled(haveTrusted);
	}
}

void OmemoKeys::onIdentityTrustClicked()
{
	setSelectedIdentityKeysTrusted(true);
}

void OmemoKeys::onIdentityUntrustClicked()
{
	setSelectedIdentityKeysTrusted(false);
}

void OmemoKeys::onIdentityContextMenu(const QPoint &APos)
{
	QModelIndex index = ui->tvIdentityKeys->indexAt(APos);
	if (!index.isValid())
		return;

	QModelIndexList list = ui->tvIdentityKeys->selectionModel()->selectedIndexes();
	if (!list.isEmpty())
	{
		bool haveTrusted(false);
		bool haveUntrusted(false);

		for (QModelIndexList::ConstIterator it = list.constBegin();
			 it != list.constEnd(); ++it)
			if (it->column()==COL_TRUSTED)
			{
				int trusted = it->data().toInt();
				if (trusted==1)
					haveTrusted = true;
				else
					haveUntrusted = true;
				if (haveTrusted && haveUntrusted)
					break;
			}

		IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
//TODO: Use Menu instead of QMenu here
		QMenu* menu = new QMenu(this);

		menu->addAction(menuicons->getIcon(MNI_RCHANGER_SUBSCRIBE), tr("Trust key"),
						this, SLOT(onIdentityTrustClicked()))->setEnabled(haveUntrusted);
		menu->addAction(menuicons->getIcon(MNI_RCHANGER_UNSUBSCRIBE), tr("Untrust key"),
						this, SLOT(onIdentityUntrustClicked()))->setEnabled(haveTrusted);
		menu->exec(QCursor::pos());
	}
}

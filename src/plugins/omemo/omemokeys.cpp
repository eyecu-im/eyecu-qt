#include <QDebug>
#include <QClipboard>

#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include <utils/pluginhelper.h>

#include "signalprotocol.h"
#include "omemo.h"
#include "omemokeys.h"
#include "ui_omemokeys.h"

#define TAG_ACCOUNT "account"

OmemoKeys::OmemoKeys(Omemo *AOmemo, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::OmemoKeys),
	FOmemo(AOmemo),
	FPresenceManager(PluginHelper::pluginInstance<IPresenceManager>()),
	FAccountManager(PluginHelper::pluginInstance<IAccountManager>()),
	FOptionsManager(PluginHelper::pluginInstance<IOptionsManager>()),
	FPreKeysModel(new QStandardItemModel(this)),
	FIdentityKeysModel(new QStandardItemModel(this))
{
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

	ui->setupUi(this);

	ui->pbPublicIdentityKeyCopy->setIcon(menuicons->getIcon(MNI_EDIT_COPY));

	FPreKeysModel->setColumnCount(2);
	FPreKeysModel->setHorizontalHeaderLabels(QStringList() << tr("Id")
														   << tr("Key data"));
	ui->tvPreKeys->setModel(FPreKeysModel);
	ui->tvPreKeys->setColumnWidth(0, 31);
	ui->tvPreKeys->setColumnWidth(1, 360);

	FIdentityKeysModel->setColumnCount(3);
	FIdentityKeysModel->setHorizontalHeaderLabels(QStringList() << tr("Name")
																<< tr("Device ID")
																<< tr("Key data")
																<< tr("Trusted"));
	ui->tvIdentityKeys->setModel(FIdentityKeysModel);
	ui->tvIdentityKeys->setColumnWidth(0, 128);
	ui->tvIdentityKeys->setColumnWidth(1, 96);
	ui->tvIdentityKeys->setColumnWidth(2, 400);
	ui->tvIdentityKeys->setColumnWidth(3, 48);

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

void OmemoKeys::onAccountIndexChanged(int AIndex)
{
	ui->pbPublicIdentityKeyCopy->setDisabled(true);
	QUuid uuid = ui->cmbAccount->itemData(AIndex).toString();
	IAccount *account = FAccountManager->findAccountById(uuid);
	if (account)
	{
		ui->cbRetractOther->setChecked(FRetractDevices.contains(uuid));
		SignalProtocol *signalProtocol = FOmemo->signalProtocol(account->streamJid());
		if (signalProtocol)
		{
			quint32 deviceId = signalProtocol->getDeviceId();
			ui->lblDeviceId->setNum(int(deviceId));

			QString fingerprint = SignalProtocol::calcFingerprint(signalProtocol->getIdentityKeyPublic(true));
			ui->lblPublicIdentityKey->setText(fingerprint);
			if (!fingerprint.isEmpty())
				ui->pbPublicIdentityKeyCopy->setEnabled(true);

			FPreKeysModel->removeRows(0, FPreKeysModel->rowCount());
			QMap<quint32, QByteArray> preKeys = signalProtocol->getPreKeys();

			for (QMap<quint32, QByteArray>::ConstIterator it=preKeys.constBegin();
				 it != preKeys.constEnd(); ++it)
			{
				QList<QStandardItem*> row;

				QStandardItem* dataItem = new QStandardItem(SignalProtocol::calcFingerprint(
																signalProtocol->curveFromEd(*it)));
				dataItem->setData(*it);

				row.append(new QStandardItem(QString::number(it.key())));
				row.append(dataItem);
				FPreKeysModel->appendRow(row);
			}

			FIdentityKeysModel->removeRows(0, FIdentityKeysModel->rowCount());
			QList<OmemoStore::IdentityKey> identityKeys = signalProtocol->getIdentityKeys();

			for (QList<OmemoStore::IdentityKey>::ConstIterator it=identityKeys.constBegin();
				 it != identityKeys.constEnd(); ++it)
			{
				QList<QStandardItem*> row;

				QStandardItem* dataItem = new QStandardItem(SignalProtocol::calcFingerprint(
								signalProtocol->curveFromEd(it->keyData)));
				dataItem->setData(it->keyData);

				row.append(new QStandardItem(it->name));
				row.append(new QStandardItem(QString::number(it->deviceId)));
				row.append(dataItem);
				row.append(new QStandardItem(QString::number(it->trusted)));
				FIdentityKeysModel->appendRow(row);
			}
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

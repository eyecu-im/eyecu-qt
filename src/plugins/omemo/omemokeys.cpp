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
	qDebug() << "OmemoKeys(" << AOmemo << "," << AParent << ")";

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
																<< tr("Key data")
																<< tr("Trusted"));
	ui->tvIdentityKeys->setModel(FIdentityKeysModel);
	ui->tvIdentityKeys->setColumnWidth(0, 128);
	ui->tvIdentityKeys->setColumnWidth(1, 360);
	ui->tvIdentityKeys->setColumnWidth(2, 48);

	reset();
}

OmemoKeys::~OmemoKeys()
{
	qDebug() << "OmemoKeys::~OmemoKeys()";
	delete ui;
}

QWidget *OmemoKeys::instance()
{
	return this;
}

void OmemoKeys::apply()
{
	emit childApply();
}

void OmemoKeys::reset()
{
	qDebug() << "OmemoKeys::reset()";

	ui->cmbAccount->clear();

	QList<IAccount*> accounts = FAccountManager->accounts();

	for (QList<IAccount*>::ConstIterator it = accounts.constBegin();
		 it != accounts.constEnd(); ++it)
		ui->cmbAccount->addItem((*it)->name(), (*it)->accountId());

	emit childReset();
}

void OmemoKeys::onAccountIndexChanged(int AIndex)
{
	ui->pbPublicIdentityKeyCopy->setDisabled(true);
	QUuid uuid = ui->cmbAccount->itemData(AIndex).toUuid();
	IAccount *account = FAccountManager->findAccountById(uuid);
	if (account)
	{
		SignalProtocol *signalProtocol = FOmemo->signalProtocol(account->streamJid());
		if (signalProtocol)
		{
			quint32 deviceId = signalProtocol->getDeviceId();
			ui->lblDeviceId->setNum(int(deviceId));

			QString fingerprint = SignalProtocol::calcFingerprint(signalProtocol->getIdentityKeyPublic());
			ui->lblPublicIdentityKey->setText(fingerprint);
			if (!fingerprint.isEmpty())
				ui->pbPublicIdentityKeyCopy->setEnabled(true);

			FPreKeysModel->removeRows(0, FPreKeysModel->rowCount());
			QMap<quint32, QByteArray> preKeys = signalProtocol->getPreKeys();

			for (QMap<quint32, QByteArray>::ConstIterator it=preKeys.constBegin();
				 it != preKeys.constEnd(); ++it)
			{
				QList<QStandardItem*> row;

				QStandardItem* dataItem = new QStandardItem(SignalProtocol::calcFingerprint(*it));
				dataItem->setData(*it);

				row.append(new QStandardItem(QString::number(it.key())));
				row.append(dataItem);
				FPreKeysModel->appendRow(row);
			}

			FIdentityKeysModel->removeRows(0, FIdentityKeysModel->rowCount());
			QHash<QString, QPair<QByteArray,uint> > identityKeys = signalProtocol->getIdentityKeys();

			for (QHash<QString, QPair<QByteArray,uint> >::ConstIterator it=identityKeys.constBegin();
				 it != identityKeys.constEnd(); ++it)
			{
				QList<QStandardItem*> row;

				QStandardItem* dataItem = new QStandardItem(SignalProtocol::calcFingerprint(it->first));
				dataItem->setData(it->first);

				row.append(new QStandardItem(it.key()));
				row.append(dataItem);
				row.append(new QStandardItem(QString::number(it->second)));
				FIdentityKeysModel->appendRow(row);
			}
		}
	}
}

void OmemoKeys::onIdentityKeyCopy()
{
	QApplication::clipboard()->setText(ui->lblPublicIdentityKey->text());
}

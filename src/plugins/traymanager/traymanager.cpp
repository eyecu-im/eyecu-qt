#include "traymanager.h"

#include <QApplication>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <utils/versionparser.h>
#include <utils/logger.h>

#define BLINK_VISIBLE_TIME      750
#define BLINK_INVISIBLE_TIME    250

TrayManager::TrayManager()
{
	FPluginManager = NULL;

	FActiveNotify = -1;

	QPixmap empty(16,16);
	empty.fill(Qt::transparent);
	FEmptyIcon.addPixmap(empty);

	FContextMenu = new Menu;
	FSystemIcon.setContextMenu(FContextMenu);

	FBlinkVisible = true;
	FBlinkTimer.setSingleShot(true);
	connect(&FBlinkTimer,SIGNAL(timeout()),SLOT(onBlinkTimerTimeout()));

	connect(&FSystemIcon,SIGNAL(messageClicked()), SIGNAL(messageClicked()));
	connect(&FSystemIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(onTrayIconActivated(QSystemTrayIcon::ActivationReason)));
}

TrayManager::~TrayManager()
{
	while (FNotifyOrder.count() > 0)
		removeNotify(FNotifyOrder.first());
	delete FContextMenu;
}

void TrayManager::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Tray Icon");
	APluginInfo->description = tr("Allows other modules to access the icon and context menu in the tray");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool TrayManager::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	FPluginManager = APluginManager;

	return true;
}

bool TrayManager::initObjects()
{
	Action *action = new Action(FContextMenu);
	action->setIcon(RSR_STORAGE_MENUICONS,MNI_MAINWINDOW_QUIT);
	action->setText(tr("Quit"));
	connect(action,SIGNAL(triggered()),FPluginManager->instance(),SLOT(quit()));
	FContextMenu->addAction(action,AG_TMTM_TRAYMANAGER_QUIT);
	return true;
}

bool TrayManager::startPlugin()
{
	setTrayIconVisible(true);
	return true;
}

QRect TrayManager::geometry() const
{
	return FSystemIcon.geometry();
}

Menu *TrayManager::contextMenu() const
{
	return FContextMenu;
}

QIcon TrayManager::icon() const
{
	return FIcon;
}

void TrayManager::setIcon(const QIcon &AIcon)
{
	FIcon = AIcon;
	if (FActiveNotify <= 0)
		FSystemIcon.setIcon(AIcon);
	else
		updateTray();
}

QString TrayManager::toolTip() const
{
	return FToolTip;
}

void TrayManager::setToolTip(const QString &AToolTip)
{
	FToolTip = AToolTip;
	if (FActiveNotify <= 0)
		FSystemIcon.setToolTip(AToolTip);
	else
		updateTray();
}

bool TrayManager::isTrayIconVisible() const
{
	return FSystemIcon.isVisible();
}

void TrayManager::setTrayIconVisible(bool AVisible)
{
	LOG_INFO(QString("Tray icon visibitity changed to=%1").arg(AVisible));
	FSystemIcon.setVisible(AVisible);
}

int TrayManager::activeNotify() const
{
	return FActiveNotify;
}

QList<int> TrayManager::notifies() const
{
	return FNotifyOrder;
}

ITrayNotify TrayManager::notifyById(int ANotifyId) const
{
	return FNotifyItems.value(ANotifyId);
}

int TrayManager::appendNotify(const ITrayNotify &ANotify)
{
	int notifyId = qrand();
	while (notifyId<=0 || FNotifyItems.contains(notifyId))
		notifyId = qrand();

	FNotifyOrder.append(notifyId);
	FNotifyItems.insert(notifyId,ANotify);
	updateTray();

	LOG_DEBUG(QString("Tray notification inserted, id=%1, blink=%2").arg(notifyId).arg(ANotify.blink));
	emit notifyAppended(notifyId);

	return notifyId;
}

void TrayManager::removeNotify(int ANotifyId)
{
	if (FNotifyItems.contains(ANotifyId))
	{
		FNotifyItems.remove(ANotifyId);
		FNotifyOrder.removeAll(ANotifyId);
		updateTray();

		LOG_DEBUG(QString("Tray notification removed, id=%1").arg(ANotifyId));
		emit notifyRemoved(ANotifyId);
	}
}

bool TrayManager::isMessagesSupported() const
{
	return FSystemIcon.supportsMessages();
}

void TrayManager::showMessage(const QString &ATitle, const QString &AMessage, QSystemTrayIcon::MessageIcon AIcon, int ATimeout)
{
	FSystemIcon.showMessage(ATitle,AMessage,AIcon,ATimeout);
	emit messageShown(ATitle,AMessage,AIcon,ATimeout);
}

void TrayManager::updateTray()
{
	int notifyId = !FNotifyOrder.isEmpty() ? FNotifyOrder.last() : -1;
	if (notifyId != FActiveNotify)
	{
		FBlinkVisible = true;
		FBlinkTimer.stop();
		FActiveNotify = notifyId;

		if (FActiveNotify > 0)
		{
			const ITrayNotify &notify = FNotifyItems.value(notifyId);
			if (notify.blink)
				FBlinkTimer.start(BLINK_VISIBLE_TIME);
			if (!notify.iconKey.isEmpty() && !notify.iconStorage.isEmpty())
				IconStorage::staticStorage(notify.iconStorage)->insertAutoIcon(&FSystemIcon,notify.iconKey);
			else
				FSystemIcon.setIcon(notify.icon);
			FSystemIcon.setToolTip(notify.toolTip);
		}
		else
		{
			FSystemIcon.setIcon(FIcon);
			FSystemIcon.setToolTip(FToolTip);
		}

		emit activeNotifyChanged(notifyId);
	}
}

void TrayManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason AReason)
{
	if (FActiveNotify > 0)
		LOG_DEBUG(QString("Tray notification activated, id=%1").arg(FActiveNotify));
	emit notifyActivated(FActiveNotify,AReason);
}

void TrayManager::onBlinkTimerTimeout()
{
	const ITrayNotify &notify = FNotifyItems.value(FActiveNotify);
	if (!FBlinkVisible)
	{
		if (!notify.iconStorage.isEmpty() && !notify.iconKey.isEmpty())
			IconStorage::staticStorage(notify.iconStorage)->insertAutoIcon(&FSystemIcon,notify.iconKey);
		else
			FSystemIcon.setIcon(notify.icon);
		FBlinkVisible = true;
		FBlinkTimer.start(BLINK_VISIBLE_TIME);
	}
	else
	{
		IconStorage::staticStorage(notify.iconStorage)->removeAutoIcon(&FSystemIcon);
		FSystemIcon.setIcon(FEmptyIcon);
		FBlinkVisible = false;
		FBlinkTimer.start(BLINK_INVISIBLE_TIME);
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_traymanager, TrayManager)
#endif

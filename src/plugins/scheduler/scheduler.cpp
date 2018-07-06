#include "scheduler.h"
#include "scheduleroptions.h"
#include "definitions/optionvalues.h"
#include "definitions/optionnodes.h"
#include "definitions/optionnodeorders.h"
#include "definitions/optionwidgetorders.h"
#include "definitions/menuicons.h"
#include "utils/qt4qt5compat.h"

SchedulerItem::operator QString() const
{
	return QString("%1;\t%2;\t%3;\t%4").arg(streamJid.full()).arg(contactJid.full()).arg(timeout).arg(message);
}

SchedulerItem::SchedulerItem(): timeout(0)
{}

SchedulerItem::SchedulerItem(const SchedulerItem &other)
{
	streamJid	= other.streamJid;
	contactJid	= other.contactJid;
	timeout		= other.timeout;
	message		= other.message;
}

SchedulerItem::SchedulerItem(const QString &string)
{
	QStringList parts=string.split(";\t");
	streamJid=parts.value(0);
	contactJid=parts.value(1);
	timeout=parts.value(2).toInt();
	message=parts.value(3);
}

bool SchedulerItem::operator ==(const SchedulerItem &other) const
{
	return	streamJid == other.streamJid &&
			contactJid == other.streamJid &&
			timeout == other.timeout &&
			message == other.message;
}

bool SchedulerItem::operator !=(const SchedulerItem &other) const
{
	return !operator ==(other);
}

//------------------------------------------------

Scheduler::Scheduler():
	FOptionsManager(NULL),
	FPresenceManager(NULL),
	FMessageProcessor(NULL)
{}

Scheduler::~Scheduler()
{}

void Scheduler::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Scheduler");
	APluginInfo->description = tr("Allows periodically send messages to specified nodes");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
}

bool Scheduler::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
	else
		return false;

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	else
		return false;

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

    //AInitOrder = 200;   //
    return true;
}

bool Scheduler::initObjects()
{
    return true;
}

bool Scheduler::initSettings()
{
	Options::setDefaultValue(OPV_SCHEDULER_ACTIVE, true);
	Options::setDefaultValue(OPV_SCHEDULER_ITEMS, QStringList());
	if (FOptionsManager)
	{
		IOptionsDialogNode dnode = {ONO_SCHEDULER, OPN_SCHEDULER, MNI_CLIENTINFO_TIME, tr("Scheduler")};
		FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
	}
    return true;
}

void Scheduler::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_SCHEDULER_ITEMS));
}

void Scheduler::onOptionsClosed()
{
}

void Scheduler::onOptionsChanged(const OptionsNode &ANode)
{	
	Q_UNUSED(ANode)

	if (ANode.path()==OPV_SCHEDULER_ITEMS)
	{
		bool active = Options::node(OPV_SCHEDULER_ACTIVE).value().toBool();
		for (QHash<QTimer*,SchedulerItem>::ConstIterator it=FSchedule.constBegin(); it!=FSchedule.constEnd(); ++it)
		{
			it.key()->stop();
			it.key()->deleteLater();
		}
		FSchedule.clear();

		QStringList strings = ANode.value().toStringList();
		for (QStringList::ConstIterator it=strings.constBegin(); it!=strings.constEnd(); ++it)
		{
			SchedulerItem item(*it);
			QTimer *timer = new QTimer(this);
			connect(timer, SIGNAL(timeout()), SLOT(onTimeout()));
			timer->setInterval(item.timeout*1000);
			FSchedule.insert(timer, item);
			if (active)
				timer->start();
		}
	}
	else if (ANode.path()==OPV_SCHEDULER_ACTIVE)
	{
		bool active = ANode.value().toBool();
		for (QHash<QTimer*,SchedulerItem>::ConstIterator it=FSchedule.constBegin(); it!=FSchedule.constEnd(); ++it)
			if (active)
				it.key()->start();
			else
				it.key()->stop();
	}
}

void Scheduler::onTimeout()
{
	QTimer *timer = qobject_cast<QTimer *>(sender());
	if (timer)
	{
		SchedulerItem item(FSchedule.value(timer));
		IPresence *presence = FPresenceManager->findPresence(item.streamJid);
		if (presence && !presence->findItem(item.contactJid).isNull())
		{
			Message message;
			message.setType(Message::Chat).setTo(item.contactJid.full()).setBody(item.message);
			FMessageProcessor->sendMessage(item.streamJid, message, IMessageProcessor::DirectionOut);
		}
	}
}

QMultiMap<int, IOptionsDialogWidget *> Scheduler::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_SCHEDULER)
		widgets.insertMulti(OWO_SCHEDULER, new SchedulerOptions(AParent));
	return widgets;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_Scheduler, Scheduler)
#endif

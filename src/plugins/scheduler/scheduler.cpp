#include "scheduler.h"
#include "scheduleroptions.h"
#include "definitions/optionnodes.h"
#include "definitions/optionnodeorders.h"
#include "definitions/optionwidgetorders.h"
#include "definitions/menuicons.h"
#include "utils/qt4qt5compat.h"
Scheduler::Scheduler():
	FOptionsManager(NULL)
{

}

Scheduler::~Scheduler()
{
}

//-----------------------------
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


    //AInitOrder = 200;   //
    return true;
}

bool Scheduler::initObjects()
{
    return true;
}

bool Scheduler::initSettings()
{
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
}

void Scheduler::onOptionsClosed()
{
}

void Scheduler::onOptionsChanged(const OptionsNode &ANode)
{
	Q_UNUSED(ANode)
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

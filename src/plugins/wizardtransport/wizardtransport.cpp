#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/serviceicons.h>
#include <definitions/optionvalues.h>
#include <definitions/actiongroups.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>

#include "wizardtransport.h"

#define ADR_STREAM_JID      Action::DR_StreamJid

WizardTransport::WizardTransport(QObject *parent) :
    QObject(parent)
    ,FXmppStreamManager(NULL)
    ,FRostersViewPlugin(NULL)
{}

void WizardTransport::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Transport Wizard");
	APluginInfo->description = tr("Wizard, which helps to connect to legacy networks via XMPP transports (gateways)");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
}

bool WizardTransport::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	IPlugin *plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
    if (plugin)
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
    if (plugin)
    {
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
        if (FXmppStreamManager)
        {
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onStreamOpened(IXmppStream *)));
			connect(FXmppStreamManager->instance(),SIGNAL(streamClosed(IXmppStream *)),SLOT(onStreamClosed(IXmppStream *)));
        }
    }

    plugin = APluginManager->pluginInterface("IRostersViewPlugin").value(0,NULL);
    if (plugin)
    {
        FRostersViewPlugin = qobject_cast<IRostersViewPlugin *>(plugin->instance());
        if (FRostersViewPlugin)
        {
            connect(FRostersViewPlugin->rostersView()->instance(),
                    SIGNAL(indexContextMenu(QList<IRosterIndex *>, quint32, Menu *)),
                    SLOT(onRosterIndexContextMenu(QList<IRosterIndex *>, quint32, Menu *)));
            //connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
        }
    }
    plugin = APluginManager->pluginInterface("IDataForms").value(0,NULL);

    AInitOrder = 200;
    return true;
}

bool WizardTransport::initObjects()
{
    return true;
}

bool WizardTransport::initSettings()
{
    return true;
}

void WizardTransport::onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu)
{
    if (ALabelId == AdvancedDelegateItem::DisplayId)
        for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
            if ((*it)->kind() == RIK_STREAM_ROOT && FStreamGateWay.contains((*it)->data(RDR_STREAM_JID).toString()))
            {
                Action *action = new Action(AMenu);
                action->setData(ADR_STREAM_JID, (*it)->data(RDR_STREAM_JID).toString());
				action->setText(tr("Connect to legacy network"));
				action->setIcon(RSR_STORAGE_SERVICEICONS, SRI_GATEWAY);
				connect(action, SIGNAL(triggered()), SLOT(onAddGateway()));
                AMenu->addAction(action, AG_RVCM_GATEWAY,true);
            }
}

void WizardTransport::onAddGateway()
{
	if (FTransportWizard)
		showTransportWizard();
	else
		startTransportWizard(qobject_cast<Action *>(sender())->data(ADR_STREAM_JID).toString());
}

void WizardTransport::onStreamOpened(IXmppStream *AXmppStream)
{
    FStreamGateWay.insert(AXmppStream->streamJid(), NULL);
}

void WizardTransport::onStreamClosed(IXmppStream *AXmppStream)
{
    FStreamGateWay.remove(AXmppStream->streamJid());
}


QWizard * WizardTransport::startTransportWizard(const Jid &AStreamJid)
{
    FAutoSubscribe=Options::node(OPV_ROSTER_AUTOSUBSCRIBE).value().toBool();
	FTransportWizard = new TransportWizard(AStreamJid);
	FTransportWizard->show();
	connect(FTransportWizard,SIGNAL(finished(int)),SLOT(onWizardFinished(int)));
	return FTransportWizard;
}

QWizard *WizardTransport::showTransportWizard()
{
	if (FTransportWizard)
		FTransportWizard->activateWindow();
	return FTransportWizard;
}

void WizardTransport::onWizardFinished(int AStatus)
{
	Q_UNUSED(AStatus)
    Options::node(OPV_ROSTER_AUTOSUBSCRIBE).setValue(FAutoSubscribe);
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_wizardtransport, WizardTransport)
#endif

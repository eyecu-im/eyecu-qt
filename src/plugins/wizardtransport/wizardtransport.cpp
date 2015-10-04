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

#define ADR_STREAM_JID		Action::DR_StreamJid
#define ADR_TRANSPORT_JID	Action::DR_Parametr1

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
	APluginInfo->dependences.append(SERVICEDISCOVERY_UUID);
	APluginInfo->dependences.append(REGISTRATION_UUID);
	APluginInfo->dependences.append(GATEWAYS_UUID);
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
	{
		QStringList streams;
		QHash<QString, QString>	agents;
		for (QList<IRosterIndex *>::const_iterator it=AIndexes.constBegin(); it!=AIndexes.constEnd(); it++)
			if (FStreamGateWay.contains((*it)->data(RDR_STREAM_JID).toString()))
			{
				if ((*it)->kind() == RIK_STREAM_ROOT)
					streams.append((*it)->data(RDR_STREAM_JID).toString());
				else if ((*it)->kind() == RIK_AGENT)
					agents.insert((*it)->data(RDR_FULL_JID).toString(), (*it)->data(RDR_STREAM_JID).toString());
			}
		if (streams.size() == 1)
		{
			Action *action = new Action(AMenu);
			action->setData(ADR_STREAM_JID, streams.first());
			action->setText(tr("Transport Wizard"));
			action->setIcon(RSR_STORAGE_SERVICEICONS, SRI_GATEWAY);
			connect(action, SIGNAL(triggered()), SLOT(onTransportWizard()));
			AMenu->addAction(action, AG_RVCM_GATEWAYS_CHANGE_TRANSPORT, true);
		}

		if (agents.size() == 1)
		{
			QHash<QString, QString>::ConstIterator it = agents.constBegin();
			Action *action = new Action(AMenu);
			action->setData(ADR_STREAM_JID, it.value());
			action->setData(ADR_TRANSPORT_JID, it.key());
			action->setText(tr("Change transport"));
			action->setIcon(RSR_STORAGE_MENUICONS, MNI_GATEWAYS_CHANGE);
			connect(action, SIGNAL(triggered()), SLOT(onTransportWizard()));
			AMenu->addAction(action, AG_RVCM_GATEWAYS_CHANGE_TRANSPORT, true);
		}
	}
}

void WizardTransport::onTransportWizard()
{
	if (FTransportWizard)
		showTransportWizard();
	else
	{
		Action *action = qobject_cast<Action *>(sender());
		startTransportWizard(action->data(ADR_STREAM_JID).toString(), action->data(ADR_TRANSPORT_JID).toString());
	}
}

void WizardTransport::onStreamOpened(IXmppStream *AXmppStream)
{
	FStreamGateWay.append(AXmppStream->streamJid().full());
}

void WizardTransport::onStreamClosed(IXmppStream *AXmppStream)
{
	FStreamGateWay.removeAll(AXmppStream->streamJid().full());
}


QWizard * WizardTransport::startTransportWizard(const Jid &AStreamJid, const Jid &ATransportJid)
{
	FAutoSubscribe = Options::node(OPV_ROSTER_AUTOSUBSCRIBE).value().toBool();
	FTransportWizard = new TransportWizard(AStreamJid, ATransportJid);
	FTransportWizard->show();
	return FTransportWizard;
}

QWizard *WizardTransport::showTransportWizard()
{
	if (FTransportWizard)
		FTransportWizard->activateWindow();
	return FTransportWizard;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_wizardtransport, WizardTransport)
#endif

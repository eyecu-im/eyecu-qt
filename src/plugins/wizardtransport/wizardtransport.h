#ifndef WIZARDTRANSPORT_H
#define WIZARDTRANSPORT_H

#include <QPointer>

#include <interfaces/iwizardtransport.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/iregistraton.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/iservicediscovery.h>

#include "wizardpages.h"

class WizardTransport : public QObject, public IPlugin, public IWizardTransport
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IWizardTransport)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IWizardTransport")
#endif
public:
    explicit WizardTransport(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return WIZARDTRANSPORT_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

	virtual QWizard * transportWizard() {return FTransportWizard;}
	virtual QWizard * startTransportWizard(const Jid &AStreamJid);
	virtual QWizard * showTransportWizard();

protected slots:
    void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);    
    void onStreamOpened(IXmppStream *AXmppStream);
    void onStreamClosed(IXmppStream *AXmppStream);
	void onWizardFinished(int AStatus);
	void onAddGateway();

private:
	IXmppStreamManager	*FXmppStreamManager;
	IRostersViewPlugin	*FRostersViewPlugin;
	QMap<Jid, QString>	FStreamGateWay;
	QPointer<TransportWizard> FTransportWizard;
	bool				FAutoSubscribe;
};

#endif // WIZARDTRANSPORT_H

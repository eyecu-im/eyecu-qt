#ifndef WIZARDACCOUNT_H
#define WIZARDACCOUNT_H

#include <interfaces/iwizardaccount.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/ioptionsmanager.h>

#include "wizardpages.h"

class WizardAccount : public QObject, public IPlugin, IWizardAccount
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IWizardAccount)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IWizardAccount")
#endif
public:
    WizardAccount(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return WIZARDACCOUNT_UUID;}
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

protected:
	void  askCreateAccount();

protected slots:
	QWizard *startWizard(QWidget *AParent = NULL);
	void onOptionsModeInitialized(bool AAdvanced);
	void onOptionsOpened();

private:
    IMainWindowPlugin   *FMainWindowPlugin;
    IOptionsManager     *FOptionsManager;
};

#endif // WIZARDACCOUNT_H

#include <QMessageBox>
#include <QApplication>
#include <definitions/optionvalues.h>
#include "wizardaccount.h"

WizardAccount::WizardAccount(QObject *parent) :
    QObject(parent),
    FMainWindowPlugin(NULL),
	FOptionsManager(NULL)
{}

void WizardAccount::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Connection Wizard");
    APluginInfo->description = tr("A Wizard, which helps unexperienced user to connect to Jabber network");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
}

bool WizardAccount::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder)

	IPlugin *plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
    if (plugin)
        FMainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
    {
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
        connect(FOptionsManager->instance(), SIGNAL(optionsModeInitialized(bool)), SLOT(onOptionsModeInitialized(bool)));
    }

	connect(Options::instance(), SIGNAL(optionsOpened()), SLOT(onOptionsOpened()));	
    return true;
}

bool WizardAccount::initObjects()
{
    return true;
}

bool WizardAccount::initSettings()
{
	return true;
}

void WizardAccount::askCreateAccount()
{
	QMessageBox *question = new QMessageBox(QMessageBox::Question,
											tr("Create an account?"),
											tr("It seems, you don't have a Jabber account yet. "
											   "Do you want to start a Wizard, which will help you to connect to Jabber?"),
											QMessageBox::Yes|QMessageBox::No,
											FMainWindowPlugin->mainWindow()->instance());
	question->setInformativeText(tr("You may start Connection Wizard anytime, by pressing \"%1\" link on \"Accounts\" page in the Options dialog.").arg(tr("Add account")));
	if (question->exec() == QMessageBox::Yes)
		startWizard(FMainWindowPlugin->mainWindow()->instance());
}

QWizard * WizardAccount::startWizard(QWidget *AParent)
{
//	FOptionsManager->showOptionsDialog()->accept();
	ConnectionWizard *wizard = new ConnectionWizard(AParent);
	wizard->show();
	return wizard;
}

void WizardAccount::onOptionsModeInitialized(bool AAdvanced)
{
	if (!AAdvanced && !Options::node(OPV_ACCOUNT_ROOT).childNames().contains("account"))
		askCreateAccount();
}

void WizardAccount::onOptionsOpened()
{
	if (!Options::node(OPV_ACCOUNT_ROOT).childNames().contains("account"))
	{
		QVariant advanced = Options::node(OPV_COMMON_ADVANCED).value();
		if (!advanced.isNull() && !advanced.toBool())
			askCreateAccount();
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_wizardaccount, WizardAccount)
#endif

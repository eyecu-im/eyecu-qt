#ifndef IWIZARDACCOUNT_H
#define IWIZARDACCOUNT_H

#include <QWizard>

#define WIZARDACCOUNT_UUID "{add578e7-bec4-42d1-a933-143bd813645b}"

class IWizardAccount
{
public:
    virtual QObject *instance() = 0;
	virtual QWizard *startWizard(QWidget *AParent = NULL) = 0;
};

Q_DECLARE_INTERFACE(IWizardAccount, "RWS.Plugin.IWizardAccount/1.0")

#endif // IWIZARDACCOUNT_H

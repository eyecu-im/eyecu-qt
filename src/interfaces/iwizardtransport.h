#ifndef IWIZARDTRANSPORT_H
#define IWIZARDTRANSPORT_H

#include <QWizard>
#include <utils/jid.h>

#define WIZARDTRANSPORT_UUID "{e4844e85-952f-4e2a-904d-5f20174387a2}"

class IWizardTransport{
public:
    virtual QObject *instance() = 0;
	virtual QWizard * transportWizard() = 0;
	virtual QWizard * showTransportWizard() = 0;
	virtual QWizard * startTransportWizard(const Jid &AStreamJid, const Jid &ATransportJid) = 0;
};

Q_DECLARE_INTERFACE(IWizardTransport, "RWS.Plugin.IWizardTransport/1.0")

#endif // IWIZARDTRANSPORT_H

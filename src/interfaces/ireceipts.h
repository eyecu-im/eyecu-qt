#ifndef IRECEIPTS_H
#define IRECEIPTS_H

#include <utils/jid.h>

#define RECEIPTS_UUID "{9F8DEB69-4AA4-3727-8474-A34B35B7630C}"
                     
class IReceipts {

public:
    virtual QObject *instance() =0;
	virtual bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const =0;

protected:
	void messageDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId);
};

Q_DECLARE_INTERFACE(IReceipts, "RWS.Plugin.IReceipts/1.0")

#endif	//IRECEIPTS_H

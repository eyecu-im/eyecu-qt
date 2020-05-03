#ifndef IPEPMANAGER_H
#define IPEPMANAGER_H

#include <QDomElement>
#include <interfaces/idataforms.h>
#include <utils/jid.h>
#include <utils/stanza.h>
#include <utils/action.h>

#define PEPMANAGER_UUID "{36dbd5c1-a3cd-11df-87fc-001cbf2edcfc}"

#define NODE_NOTIFY_SUFFIX             "+notify"

class IPEPHandler
{
public:
	virtual QObject *instance() =0;
	virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza) =0;
};

class IPEPManager
{
public:
	virtual bool isSupported(const Jid &AStreamJid) const =0;
	virtual bool publishItem(const Jid &AStreamJid, const QString &ANode, const QDomElement &AItem, const IDataForm *AOptions=NULL) =0;
    virtual bool deleteItem(const Jid &AStreamJid, const QString &ANode, const QDomElement &AItem) =0;

	virtual IPEPHandler *nodeHandler(int AHandleId) const =0;
	virtual int insertNodeHandler(const QString &ANode, IPEPHandler *AHandle) =0;
	virtual bool removeNodeHandler(int AHandleId) =0;
	// *** <<< eyeCU <<< ***
	virtual Action *addAction(int AGroup = 500, bool ASort = false) =0;
	virtual QList<Action *> groupActions(int AGroup = -1) = 0;
	// *** >>> eyeCU >>> ***
};

Q_DECLARE_INTERFACE(IPEPHandler,"Vacuum.Plugin.IPEPHandler/1.0")
Q_DECLARE_INTERFACE(IPEPManager,"Vacuum.Plugin.IPEPManager/1.1")

#endif // IPEPMANAGER_H

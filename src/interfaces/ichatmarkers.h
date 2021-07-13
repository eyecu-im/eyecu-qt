#ifndef ICHATMARKERS_H
#define ICHATMARKERS_H

#include <utils/jid.h>
#include <utils/stanza.h>

#define CHATMARKERS_UUID "{2f3c771d-fa90-6c23-b418-791bc2513d76}"
                     
class IChatMarkers {
public:
	enum Type {
		Unknown,
		Received,
		Displayed,
		Acknowledged,
		Acknowledge
	};
    virtual QObject *instance() =0;
	virtual bool addAcceptableElement(const QString &ANamespace, const QString &ATagName) =0;
	virtual bool removeAcceptableElement(const QString &ANamespace, const QString &ATagName) =0;
	virtual bool isElementAcceptable(const QString &ANamespace, const QString &ATagName) const =0;
	virtual bool isStanzaAcceptable(const Stanza &AStanza) const =0;
	virtual bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const =0;
	virtual bool isReceiptsSupported(const Jid &AStreamJid, const Jid &AContactJid) const =0;
protected:
	virtual void markable(const Jid &AStreamJid, const Jid &AContactJid) =0;
	virtual void messagesMarked(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId, Type AType) =0;
};

Q_DECLARE_INTERFACE(IChatMarkers, "RWS.Plugin.IChatMarkers/1.0")

#endif	//ICHATMARKERS_H

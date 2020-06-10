#ifndef ISTANZACONTENTENCRYPTION_H
#define ISTANZACONTENTENCRYPTION_H

#include <utils/stanza.h>

#define STANZACONTENTENCRYPTION_UUID "{d4631f05-23b5-a771-22f6-bc4b89d2f3a2}"
                     
class IStanzaContentEncrytion {
public:
	virtual QObject *instance() =0;
	virtual bool addAcceptableElement(const QString &ANamespace, const QString &ATagName) =0;
	virtual bool removeAcceptableElement(const QString &ANamespace, const QString &ATagName)  =0;
	virtual bool isElementAcceptable(const QString &ANamespace, const QString &ATagName) const  =0;
	virtual bool isStanzaAcceptable(const Stanza &AStanza) const  =0;
	virtual QByteArray getContentToEncrypt(const Stanza &AStanza, const QString &AFallbackBodyText) const =0;
};

Q_DECLARE_INTERFACE(IStanzaContentEncrytion, "RWS.Plugin.StanzaContentEncrytion/1.0")

#endif	//ISTANZACONTENTENCRYPTION_H

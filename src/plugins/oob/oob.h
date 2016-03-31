#ifndef OOB_H
#define OOB_H

#include <interfaces/ioob.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagestylemanager.h>

#include "ooblinklist.h"

class Oob:  public QObject,
            public IPlugin,
            public IOob,
            public IMessageEditor,
            public IMessageWriter
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IOob IMessageEditor IMessageWriter)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IOob")
#endif
public:
    Oob();
    ~Oob();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return OOB_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin() { return true; }

    //IMessageEditor
    virtual bool messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection);

    //IMessageWriter
	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang);
	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang);

protected:
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
    void registerDiscoFeatures();
    bool parseOOB(Stanza &AStanza, QTextDocument *ADocument);   // Parse message stanza,
                                                                // containing jabber:x:oob element
                                                                // to generate HTML code
    void appendLinks(Message &AMessage, OobLinkList *ALinkList);

private:
	IXmppStreamManager	*FXmppStreamManager;
	IMessageProcessor	*FMessageProcessor;
	IServiceDiscovery	*FDiscovery;
	IMessageWidgets		*FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;

	IconStorage			*FIconStorage;
	int					FOobHandlerIn;
	int					FOobHandlerOut;

    QMap <Jid, QString> FStreamOob;

protected:
    OobLinkList *findLinkList(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType);
    OobLinkList *getLinkList(const IMessageChatWindow *AWindow) const;
    OobLinkList *getLinkList(const IMessageNormalWindow *AWindow)const;

    void updateChatWindowActions(IMessageChatWindow *AChatWindow);

protected slots:
    void onStreamOpened(IXmppStream *AXmppStream);
    void onStreamClosed(IXmppStream *AXmppStream);
    void onInsertLink(bool);

    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onNormalWindowCreated(IMessageNormalWindow *AWindow);
    void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
};

#endif // OOB_H

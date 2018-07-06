#ifndef XMPPURIQUERIES_H
#define XMPPURIQUERIES_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ixmppuriqueries.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imapmessage.h>

class XmppUriQueries :
	public QObject,
	public IPlugin,
	public IXmppUriQueries,
	public IMessageViewUrlHandler,
	public IBubbleUrlEventHandler // *** <<< eyeCU >>> ***
{
	Q_OBJECT;
// *** <<< eyeCU <<< ***
	Q_INTERFACES(IPlugin IXmppUriQueries IMessageViewUrlHandler IBubbleUrlEventHandler);
// *** >>> eyeCU >>> ***
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.jrudevels.vacuum.IXmppUriQueries")
#endif
public:
	XmppUriQueries();
	~XmppUriQueries();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return XMPPURIQUERIES_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings() { return true; }
	virtual bool startPlugin() { return true; }
// *** <<< eyeCU <<< ***
	//IMessageViewUrlHandler
	virtual bool messageViewUrlOpen(int AOrder, IMessageViewWidget *AWidget, const QUrl &AUrl);
// *** >>> eyeCU >>> ***
	//IXmppUriQueries
	virtual bool openXmppUri(const Jid &AStreamJid, const QUrl &AUrl) const;
	virtual bool parseXmppUri(const QUrl &AUrl, Jid &AContactJid, QString &AAction, QMultiMap<QString, QString> &AParams) const;
	virtual QString makeXmppUri(const Jid &AContactJid, const QString &AAction, const QMultiMap<QString, QString> &AParams) const;
	virtual void insertUriHandler(int AOrder, IXmppUriHandler *AHandler);
	virtual void removeUriHandler(int AOrder, IXmppUriHandler *AHandler);
	// IBubbleUrlEventHandler interface
	virtual bool bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid);
signals:
	void uriHandlerInserted(int AOrder, IXmppUriHandler *AHandler);
	void uriHandlerRemoved(int AOrder, IXmppUriHandler *AHandler);
private:
	IMessageWidgets *FMessageWidgets;
	IMapMessage		*FMapMessage; // *** <<< eyeCU >>> ***
private:
	QMultiMap<int, IXmppUriHandler *> FHandlers;	
};

#endif // XMPPURIQUERIES_H

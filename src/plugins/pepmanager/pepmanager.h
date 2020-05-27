#ifndef PEPMANAGER_H
#define PEPMANAGER_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ixmppstreammanager.h>
// *** <<< eyeCU <<< ***
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imainwindow.h>
// *** >>> eyeCU >>> ***

class PEPManager : 
	public QObject,
	public IPlugin,
	public IPEPManager,
    public IStanzaHandler,
	public IOptionsDialogHolder  // *** <<< eyeCU >>>***
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IPEPManager IStanzaHandler IOptionsDialogHolder);	// *** <<< eyeCU >>>***
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.jrudevels.vacuum.IPEPManager")
#endif
public:
	PEPManager();
	~PEPManager();
	// IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return PEPMANAGER_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects() { return true; }
    virtual bool initSettings(); // *** <<< eyeCU >>> ***
	virtual bool startPlugin() { return true; }
	// IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
    // *** <<< eyeCU <<< ***
    // IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
    // *** >>> eyeCU >>> ***
	// IPEPManager
	virtual bool isSupported(const Jid &AStreamJid) const;
	virtual bool publishItem(const Jid &AStreamJid, const QString &ANode, const QDomElement &AItem, const IDataForm *AOptions=NULL);
    virtual bool deleteItem(const Jid &AStreamJid, const QString &ANode, const QDomElement &AItem);  // *** <<< eyeCU >>>***

	virtual IPEPHandler* nodeHandler(int AHandleId) const;
	virtual int insertNodeHandler(const QString &ANode, IPEPHandler *AHandle);
	virtual bool removeNodeHandler(int AHandleId);

	// *** <<< eyeCU <<< ***
	virtual Action *addAction(int AGroup = 500, bool ASort = false);
	virtual QList<Action *> groupActions(int AGroup = -1);
	// *** >>> eyeCU >>> ***
private slots:
	void onXmppStreamOpened(IXmppStream *AXmppStream);
	void onXmppStreamClosed(IXmppStream *AXmppStream);
	void onPEPHandlerDestroyed(QObject *AHandler);
private:
	IXmppStreamManager *FXmppStreamManager;
	IServiceDiscovery *FDiscovery;
	IStanzaProcessor *FStanzaProcessor;
// *** <<< eyeCU <<< ***
	IOptionsManager		*FOptionsManager;
	IMainWindowPlugin	*FMainWindowPlugin;
	Menu				*FMenu;
// *** >>> eyeCU >>> ***
private:
	QMap<Jid, int> FStanzaHandles;
	QMap<int, IPEPHandler *> FHandlersById;
	QMultiMap<QString, int> FHandlersByNode;
};

#endif // PEPMANAGER_H

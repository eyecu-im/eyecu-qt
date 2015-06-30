#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QSet>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iregistraton.h>
#include <interfaces/idataforms.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/iaccountmanager.h>	// *** <<< eyeCU >>> ***
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/ixmppuriqueries.h>
#include "registerdialog.h"
#include "registerfeature.h"

class Registration :
	public QObject,
	public IPlugin,
	public IRegistration,
	public IStanzaRequestOwner,
	public IXmppUriHandler,
	public IDiscoFeatureHandler,
	public IXmppFeatureFactory,
	public IOptionsDialogHolder, // *** <<< eyeCU >>> ***
	public IDataLocalizer
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IRegistration IStanzaRequestOwner IXmppUriHandler IDiscoFeatureHandler IXmppFeatureFactory IOptionsDialogHolder IDataLocalizer); // *** <<< eyeCU >>> ***
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.jrudevels.vacuum.IRegistration")
#endif
	friend class RegisterFeature;
public:
	Registration();
	~Registration();
	virtual QObject *instance() { return this; }
	//IPlugin
	virtual QUuid pluginUuid() const { return REGISTRATION_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
	//IStanzaRequestOwner
	virtual void stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza);
	//IXmppUriHandler
	virtual bool xmppUriOpen(const Jid &AStreamJid, const Jid &AContactJid, const QString &AAction, const QMultiMap<QString, QString> &AParams);
	//IDiscoFeatureHandler
	virtual bool execDiscoFeature(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo);
	virtual Action *createDiscoFeatureAction(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo, QWidget *AParent);
	//IXmppFeatureFactory
	virtual QList<QString> xmppFeatures() const;
	virtual IXmppFeature *newXmppFeature(const QString &AFeatureNS, IXmppStream *AXmppStream);
// *** <<< eyeCU <<< ***
	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
// *** >>> eyeCU >>> ***
	//IDataLocalizer
	virtual IDataFormLocale dataFormLocale(const QString &AFormType);
	//IRegistration
	virtual QString startStreamRegistration(IXmppStream *AXmppStream);
	virtual QString submitStreamRegistration(IXmppStream *AXmppStream, const IRegisterSubmit &ASubmit);
	virtual QString sendRegisterRequest(const Jid &AStreamJid, const Jid &AServiceJid);
	virtual QString sendUnregisterRequest(const Jid &AStreamJid, const Jid &AServiceJid);
	virtual QString sendChangePasswordRequest(const Jid &AStreamJid, const Jid &AServiceJid, const QString &AUserName, const QString &APassword);
	virtual QString sendRequestSubmit(const Jid &AStreamJid, const IRegisterSubmit &ASubmit);
	virtual QDialog *showRegisterDialog(const Jid &AStreamJid, const Jid &AServiceJid, int AOperation, QWidget *AParent = NULL);
signals:
	//IXmppFeatureFactory
	void featureCreated(IXmppFeature *AStreamFeature);
	void featureDestroyed(IXmppFeature *AStreamFeature);
	//IRegistration
	void registerFields(const QString &AId, const IRegisterFields &AFields);
	void registerError(const QString &AId, const XmppError &AError);
	void registerSuccess(const QString &AId);
protected:
	void registerDiscoFeatures();
	IRegisterFields readFields(const Jid &AServiceJid, const QDomElement &AQuery) const;
	bool writeSubmit(QDomElement &AQuery, const IRegisterSubmit &ASubmit) const;
	void processFields(const QString &AId, const IRegisterFields &AFields); // *** <<< eyeCU >>> ***
protected slots:
	void onXmppFeatureFields(const IRegisterFields &AFields);
	void onXmppFeatureFinished(bool ARestart);
	void onXmppFeatureDestroyed();
protected slots:
	void onXmppStreamOpened();
	void onXmppStreamClosed();
	void onXmppStreamError(const XmppError &AError);
protected slots:
	void onRegisterActionTriggered(bool);
private:
	IDataForms *FDataForms;
	IXmppStreamManager *FXmppStreamManager;
	IStanzaProcessor *FStanzaProcessor;
	IServiceDiscovery *FDiscovery;
	IPresenceManager *FPresenceManager;
// *** <<< eyeCU <<< ***
	IOptionsManager *FOptionsManager;
	IAccountManager *FAccountManager;
	QSet<IXmppStream *> FAutoRegisterStreams;
// *** >>> eyeCU >>> ***
	IXmppUriQueries *FXmppUriQueries;
private:
	QList<QString> FSendRequests;
	QList<QString> FSubmitRequests;
private:
	QMap<IXmppStream *, QString> FStreamRegisterId;
	QMap<IXmppStream *, RegisterFeature *> FStreamFeatures;
};

#endif // REGISTRATION_H

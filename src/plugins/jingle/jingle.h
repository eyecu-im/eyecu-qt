#ifndef JINGLE_H
#define JINGLE_H

#include <interfaces/ijingle.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ioptionsmanager.h>
#include "jinglesession.h"

class Jingle: public QObject,
              public IPlugin,
              public IJingle,
              public IStanzaHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IJingle IStanzaHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IJingle")
#endif
public:
	Jingle(QObject *parent = nullptr);
	~Jingle() override;
    bool sendStanzaOut(Stanza &AStanza);

    //IPlugin
	virtual QObject *instance() override { return this; }
	virtual QUuid pluginUuid() const  override { return JINGLE_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo) override;
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	virtual bool initObjects() override;
	virtual bool initSettings() override;
	virtual bool startPlugin() override {return true;}

    //IStanzaHandler
	virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept) override;

    //IJingle
	virtual IJingleApplication *appByNS(const QString &AApplicationNS)  override {
		return FApplications.value(AApplicationNS);
	}
	virtual QString sessionCreate(const Jid &AStreamJid, const Jid &AContactJid,
						  const QString &AApplicationNS) override;
//	virtual bool    sessionInitiate(const QString &ASid) override;
	virtual bool    sessionAccept(const QString &ASid) override;
	virtual bool    sessionTerminate(const QString &ASid, Reason AReason) override;
	virtual bool	sessionDestroy(const QString &ASid) override;
	virtual bool    sendAction(const QString &ASid, IJingle::Action AAction,
					   const QDomElement &AJingleElement) override;
	virtual bool    sendAction(const QString &ASid, IJingle::Action AAction,
					   const QDomNodeList &AJingleElements) override;
	virtual IJingleContent *contentAdd(const QString &ASid, const QString &AName,
							   const QString &AMediaType, int AComponentCount,
							   IJingleTransport::Type ATransportType,
							   bool AFromResponder) override;
	virtual QHash<QString, IJingleContent *> contents(const QString &ASid) const override;
	virtual IJingleContent *content(const QString &ASid, const QString &AName) const override;
	virtual IJingleContent *content(const QString &ASid, QIODevice *ADevice) const override;
	virtual bool    selectTransportCandidate(const QString &ASid, const QString &AContentName,
									 const QString &ACandidateId) override;
	virtual bool    connectContent(const QString &ASid, const QString &AName) override;
	virtual bool    setConnected(const QString &ASid) override;
	virtual bool    fillIncomingTransport(IJingleContent *AContent) override;
	virtual void    freeIncomingTransport(IJingleContent *AContent) override;

	virtual SessionStatus sessionStatus(const QString &ASid) const override;
	virtual bool    isOutgoing(const QString &ASid) const override;
	virtual Jid     contactJid(const QString &ASid) const override;
	virtual Jid     streamJid(const QString &ASid) const override;
	virtual QString errorMessage(Reason AReason) const override;

protected:
    void registerDiscoFeatures();
	bool processSessionInitiate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
    bool processSessionAccept(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
	bool processSessionTerminate(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
	bool processSessionInfo(const Jid &AStreamJid, const JingleStanza &AStanza, bool &AAccept);
	IJingleTransport *transportByNs(const QString &ANameSpace);

protected slots:
	void onConnectionOpened(IJingleContent *AContent);
	void onConnectionFailed(IJingleContent *AContent);
    void onIncomingTransportFilled(IJingleContent *AContent);
    void onIncomingTransportFillFailed(IJingleContent *AContent);

signals:
	void connectionOpened(IJingleContent *AContent) override;
	void connectionFailed(IJingleContent *AContent) override;
	void contentAdded(IJingleContent *AContent) override;
	void contentAddFailed(IJingleContent *AContent) override;

//    void incomingTransportFilled(IJingleContent *AContent);
//    void incomingTransportFillFailed(IJingleContent *AContent);

private:
    IStanzaProcessor    *FStanzaProcessor;
    IServiceDiscovery   *FServiceDiscovery;
	IOptionsManager		*FOptionsManager;
	QMap<QString, IJingleApplication*>	FApplications;
	QMap<int, IJingleTransport*>	FTransports;
	QHash<QIODevice *, IJingleContent *>	FCandidateTries;
//	QList<IJingleContent *>	FPendingContents;
    int FSHIRequest;
    int FSHIResult;
    int FSHIError;

	// IJingle interface
public:

};

#endif // JINGLE_H

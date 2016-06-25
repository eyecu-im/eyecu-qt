#ifndef IJINGLE_H
#define IJINGLE_H

#include <QDomDocument>
#include <QAbstractSocket>
#include <utils/jid.h>

#define JINGLE_UUID "{c2fdea84-bc30-f2dc-9b11-45bc7df68e23}"

class IJingleApplication;
class IJingleTransport;
class IJingleContent;

class IJingle
{
public:
	enum SessionStatus
	{
		None,
		Initiated,
		Accepted,
		Connected,
		ReceivingData,
		Terminated
	};

	enum SessionState
	{
		NoState,
		Pending,
		Active,
		Ended
	};

	enum Action
	{
		NoAction,
		SessionInitiate,
		SessionAccept,
		SessionTerminate,
		SessionInfo,
		ScurityInfo,
		DescriptionInfo,
		ContentAccept,
		ContentAdd,
		ContentModify,
		ContentReject,
		ContentRemove,
		TransportAccept,
		TransportInfo,
		TransportReject,
		TransportReplace
	};

	enum CommandRespond
	{
		Acknowledge,
		ServiceUnavailable,
		Redirect,
		ResourceConstraint,
		BadRequest
	};

	enum Reason
	{
		NoReason,
		Success,
		Busy,
		Cancel,
		AlternativeSession,
		ConnectivityError,
		Decline,
		Expired,
		FailedApplication,
		FailedTransport,
		GeneralError,
		Gone,
		IncompatibleParameters,
		MediaError,
		SecurityError,
		Timeout,
		UnsupportedApplications,
		UnsupportedTransports
	};

	virtual QObject *instance() =0;
	virtual IJingleApplication *appByNS(const QString &AApplicationNS) =0;
	virtual QString sessionCreate(const Jid &AStreamJid, const Jid &AContactJid, const QString &AApplicationNS) =0;
	virtual bool    sessionInitiate(const Jid &AStreamJid, const QString &ASid) =0;
	virtual bool    sessionAccept(const Jid &AStreamJid, const QString &ASid) =0;
	virtual bool    sessionTerminate(const Jid &AStreamJid, const QString &ASid, Reason AReason) =0;
	virtual bool    sendAction(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, const QDomElement &AJingleElement) =0;
	virtual bool    sendAction(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, const QDomNodeList &AJingleElements) =0;
	virtual IJingleContent *contentAdd(const Jid &AStreamJid, const QString &ASid, const QString &AName, const QString &AMediaType, const QString &ATransportNameSpace, bool AFromResponder) =0;
	virtual QHash<QString, IJingleContent *> contents(const Jid &AStreamJid, const QString &ASid) const =0;
	virtual IJingleContent *content(const Jid &AStreamJid, const QString &ASid, const QString &AName) const =0;
	virtual IJingleContent *content(const Jid &AStreamJid, const QString &ASid, QIODevice *ADevice) const =0;
	virtual bool    connectContent(const Jid &AStreamJid, const QString &ASid, const QString &ANAme) =0;
	virtual bool    setConnected(const Jid &AStreamJid, const QString &ASid) =0;
	virtual bool    fillIncomingTransport(IJingleContent *AContent) =0;
	virtual void    freeIncomingTransport(IJingleContent *AContent) =0;
	virtual Jid     contactJid(const Jid &AStreamJid, const QString &ASid) const =0;
	virtual bool    selectTransportCandidate(const Jid &AStreamJid, const QString &ASid, const QString &AContentName, const QString &ACandidateId) =0;

	virtual SessionStatus sessionStatus(const Jid &AStreamJid, const QString &ASid) const =0;
	virtual bool isOutgoing(const Jid &AStreamJid, const QString &ASid) const =0;
	virtual QString errorMessage(Reason AReason) const =0;

protected:
	virtual void startSendData(IJingleContent *AContent) =0;
	virtual void connectionFailed(IJingleContent *AContent) =0;
	virtual void contentAdded(IJingleContent *AContent) =0;
	virtual void contentAddFailed(IJingleContent *AContent) =0;
};

class IJingleApplication
{
public:
	virtual QObject *instance() =0;
	virtual QString ns() const =0;
	virtual bool checkSupported(QDomElement &ADescription) =0;  // To check if Jingle request is supported

	virtual void onSessionInitiated(const Jid &AStreamJid, const QString &ASid) =0;       // To notify, about new initiate request
	virtual void onSessionAccepted(const Jid &AStreamJid, const QString&ASid) =0;
	virtual void onSessionConnected(const Jid &AStreamJid, const QString&ASid) =0;
	virtual void onSessionTerminated(const Jid &AStreamJid, const QString &ASid, IJingle::SessionStatus ASessionStatus, IJingle::Reason AReason) =0;
	virtual void onActionAcknowledged(const Jid &AStreamJid, const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus ASessionStatus, const Jid &ARedirect, IJingle::Reason AReason) =0; // To notify, about own initiate request acknowleged
	virtual void onDataReceived(const Jid &AStreamJid, const QString&ASid, QIODevice *ADevice) =0;

	virtual void onConnectionEstablished(IJingleContent *AContent) =0;
	virtual void onConnectionFailed(IJingleContent *AContent) =0;
};

class IJingleTransport
{
public:
	virtual QObject *instance() =0;
	virtual bool    isStreaming() const =0; // Is the transport streaming or datagram
	virtual QString ns() const =0;          // Transport namespace
	virtual int     priority() const =0;        // Transport priority
	//!
	//! \brief openConnection Try to open connections
	//! \param AContent Jingle content
	//! \return success indicator
	//!
	virtual bool openConnection(IJingleContent *AContent) =0;
	//!
	//! \brief fillIncomingTransport fills incoming transport data in jingle content
	//! \param AContent jingle content to process
	//! \return success indicator
	//!
	virtual bool fillIncomingTransport(IJingleContent *AContent) =0;
	virtual void freeIncomingTransport(IJingleContent *AContent) =0;

protected:
	virtual void connectionOpened(IJingleContent *AContent) =0;
	virtual void connectionError(IJingleContent *AContent) =0;
	virtual void incomingTransportFilled(IJingleContent *AContent) =0;
	virtual void incomingTransportFillFailed(IJingleContent *AContent) =0;
};

class IJingleContent
{
public:
	virtual const   QString     &name() const =0;
	virtual const   QString     &sid() const =0;
	virtual const   QString     &streamJid() const =0;
	virtual         bool        fromResponder() const =0;
	virtual         QStringList candidateIds() const =0;
	virtual         QList<QDomElement>  candidates() const =0;
	virtual const   QDomElement candidate(const QString &AId) const =0;
	virtual         bool        chooseCandidate(const QString &AId) =0;

	virtual         bool        enumerateCandidates() =0;
	virtual         QDomElement nextCandidate() =0;

	virtual         QString     transportNS() const =0;
	virtual const   QDomElement &description() const =0;
	virtual const   QDomElement &transportOutgoing() const =0;
	virtual const   QDomElement &transportIncoming() const =0;
	virtual const   QDomDocument &document() const =0;
//	virtual			QAbstractSocket *socket();
//	virtual			setSocket(QAbstractSocket *ASocket);
	virtual         QIODevice   *inputDevice(const QString &AId) const =0;
	virtual         void        setInputDevice(const QString &AId, QIODevice *ADevice) =0;
	virtual         QIODevice   *outputDevice(const QString &AId) const =0;
	virtual         void        setOutputDevice(const QString &AId, QIODevice *ADevice) =0;
};

Q_DECLARE_INTERFACE(IJingle, "RWS.Plugin.IJingle/1.0")
Q_DECLARE_INTERFACE(IJingleTransport, "RWS.Plugin.IJingleTransport/1.0")
Q_DECLARE_INTERFACE(IJingleApplication, "RWS.Plugin.IJingleApplication/1.0")

#endif	//IJINGLERTP_H

#ifndef IJINGLE_H
#define IJINGLE_H

#include <QDomDocument>
#include <QAbstractSocket>
#include <utils/jid.h>

#define JINGLE_UUID "{c2fdea84-bc30-f2dc-9b11-45bc7df68e23}"

class IJingleApplication;
class IJingleContent;

class IJingleTransport
{
public:
	enum Type {
		Datagram,
		Streaming
	};
	Q_DECLARE_FLAGS(Types, Type)

	virtual QObject *instance() =0;
	virtual Types    types() const =0;		// Is the transport streaming or datagram
	virtual QString ns() const =0;			// Transport namespace
	virtual int     priority() const =0;	// Transport priority
	//!
	//! \brief openConnection Try to open connections
	//! \param ASid Jingle session ID
	//! \param AContentName Jingle content name
	//! \return success indicator
	//!
	virtual bool openConnection(const QString &ASid, const QString &AContentName) =0;

	//!
	//! \brief fillIncomingTransport fills incoming transport data in jingle content
	//! \param ASid Jingle session ID
	//! \param AContentName Jingle content name
	//! \return success indicator
	//!
	virtual bool fillIncomingTransport(const QString &ASid, const QString &AContentName) =0;

	//!
	//! \brief freeIncomingTransport frees resources allocatd by transport for the content
	//! \param ASid Jingle session ID
	//! \param AContentName Jingle content name
	//!
	virtual void freeIncomingTransport(const QString &ASid, const QString &AContentName) =0;

protected:
	virtual void connectionOpened(const QString &ASid, const QString &AContentName) =0;
	virtual void connectionError(const QString &ASid, const QString &AContentName) =0;
	virtual void incomingTransportFilled(const QString &ASid, const QString &AContentName) =0;
	virtual void incomingTransportFillFailed(const QString &ASid, const QString &AContentName) =0;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(IJingleTransport::Types)

class IJingle
{
public:
	enum SessionStatus
	{
		None,
		Initiating,
		Initiated,
		Accepting,
		Accepted,
		Connected,
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
	virtual bool    sessionTerminate(const QString &ASid, Reason AReason) =0;
	virtual bool    sessionDestroy(const QString &ASid) =0;
	virtual bool    sendAction(const QString &ASid, IJingle::Action AAction,
							   const QDomElement &AJingleElement) =0;
	virtual bool    sendAction(const QString &ASid, IJingle::Action AAction,
							   const QDomNodeList &AJingleElements) =0;
	virtual IJingleContent *contentAdd(const QString &ASid, const QString &AName,
									   const QString &AMediaType, int AComponentCount,
									   IJingleTransport::Type ATransportType,
									   bool AFromResponder) =0;
	virtual QHash<QString, IJingleContent *> contents(const QString &ASid) const =0;
	virtual IJingleContent *content(const QString &ASid, const QString &AName) const =0;
	virtual IJingleContent *content(const QString &ASid, QIODevice *ADevice) const =0;
	virtual bool    connectContent(const QString &ASid, const QString &ANAme) =0;
	virtual bool    setConnected(const QString &ASid) =0;
	virtual bool    setAccepting(const QString &ASid) =0;
	virtual bool    fillIncomingTransport(IJingleContent *AContent) =0;
	virtual void    freeIncomingTransport(IJingleContent *AContent) =0;
	virtual Jid     contactJid(const QString &ASid) const =0;
	virtual Jid     streamJid(const QString &ASid) const =0;
	virtual QString findSid(const Jid &AStreamJid, const Jid &AContactJid) const =0;
	virtual bool    selectTransportCandidate(const QString &ASid, const QString &AContentName,
											 const QString &ACandidateId) =0;
	virtual SessionStatus sessionStatus(const QString &ASid) const =0;
	virtual bool isOutgoing(const QString &ASid) const =0;
	virtual QString errorMessage(Reason AReason) const =0;

protected:
	virtual void connectionOpened(const QString &ASid, const QString &AName) =0;
	virtual void connectionFailed(const QString &ASid, const QString &AName) =0;
//	virtual void contentAdded(const QString &ASid, const QString &AName) =0;
//	virtual void contentAddFailed(const QString &ASid, const QString &AName) =0;
};

class IJingleApplication
{
public:
	virtual QObject *instance() =0;
	virtual QString ns() const =0;
	virtual bool checkSupported(QDomElement &ADescription) =0;	// To check if Jingle request is supported

	virtual void onSessionInitiated(const QString &ASid) =0;	// To notify about new initiate request
	virtual void onSessionAccepted(const QString &ASid) =0;		// To notify about session acceptance
	virtual void onSessionConnected(const QString &ASid) =0;	// To notify about session connection
	virtual void onSessionTerminated(const QString &ASid, IJingle::SessionStatus ASessionStatus, IJingle::Reason AReason) =0;
	virtual void onSessionInformed(const QDomElement &AInfoElement) =0; // To notify about session inform request
	virtual void onActionAcknowledged(const QString &ASid, IJingle::Action AAction, IJingle::CommandRespond ARespond, IJingle::SessionStatus ASessionStatus, const Jid &ARedirect, IJingle::Reason AReason) =0; // To notify, about own initiate request acknowleged
	virtual void onSessionDestroyed(const QString &ASid) =0;	// Session destroyed

	virtual void onConnectionEstablished(const QString &ASid, const QString &AName) =0;
	virtual void onConnectionFailed(const QString &ASid, const QString &AName) =0;
};

class IJingleContent
{
public:
	virtual const   QString     &name() const =0;
	virtual const   QString     &sid() const =0;
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
	virtual			int			componentCount() const =0;
	virtual			void		setComponentCount(int ACount) =0;
	virtual			int			component(QIODevice *) const =0;
	virtual         QIODevice   *ioDevice(int AComponentId) const =0;
	virtual         bool        setIoDevice(int AComponentId, QIODevice *ADevice) =0;
};

Q_DECLARE_INTERFACE(IJingle, "RWS.Plugin.IJingle/1.0")
Q_DECLARE_INTERFACE(IJingleTransport, "RWS.Plugin.IJingleTransport/1.0")
Q_DECLARE_INTERFACE(IJingleApplication, "RWS.Plugin.IJingleApplication/1.0")

#endif	//IJINGLERTP_H

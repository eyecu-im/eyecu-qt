#include "xmppstream.h"

#include <QTextDocument>
#include <definitions/namespaces.h>
#include <definitions/optionvalues.h>
#include <definitions/internalerrors.h>
#include <definitions/xmppstanzahandlerorders.h>
#include <utils/versionparser.h>
#include <utils/options.h>
#include <utils/logger.h>

XmppStream::XmppStream(IXmppStreamManager *AXmppStreamManager, const Jid &AStreamJid) : QObject(AXmppStreamManager->instance())
{
	FXmppStreamManager = AXmppStreamManager;

	FReady = false;
	FClosed = true;
	FEncrypt = true;
	FNodeChanged = false;
	FDomainChanged = false;
	FPasswordRequested = false;
	FConnection = NULL;
	FStreamState = SS_OFFLINE;

	FStreamJid = AStreamJid;
	FOfflineJid = FStreamJid;

	connect(&FParser,SIGNAL(opened(const QDomElement &)), SLOT(onParserOpened(const QDomElement &)));
	connect(&FParser,SIGNAL(element(const QDomElement &)), SLOT(onParserElement(const QDomElement &)));
	connect(&FParser,SIGNAL(error(const XmppError &)), SLOT(onParserError(const XmppError &)));
	connect(&FParser,SIGNAL(closed()), SLOT(onParserClosed()));

	FKeepAliveTimer.setSingleShot(false);
	connect(&FKeepAliveTimer,SIGNAL(timeout()),SLOT(onKeepAliveTimeout()));
}

XmppStream::~XmppStream()
{
	abort(XmppError(IERR_XMPPSTREAM_DESTROYED));
	clearActiveFeatures();
	emit streamDestroyed();
}

bool XmppStream::xmppStanzaIn(IXmppStream *AXmppStream, Stanza &AStanza, int AOrder)
{
	if (AXmppStream==this && AOrder==XSHO_XMPP_STREAM && AStanza.namespaceURI()==NS_JABBER_STREAMS)
	{
		if (FStreamState==SS_INITIALIZE && AStanza.kind()=="stream")
		{
			FStreamId = AStanza.id();
			setStreamState(SS_FEATURES);
			if (VersionParser(AStanza.attribute("version","0.0")) < VersionParser(1,0))
			{
				Stanza stanza("features",NS_JABBER_STREAMS);
				stanza.addElement("auth",NS_FEATURE_IQAUTH);
				xmppStanzaIn(AXmppStream, stanza, AOrder);
			}
			return true;
		}
		else if (FStreamState==SS_FEATURES && AStanza.kind()=="features")
		{
			FServerFeatures = AStanza.element().cloneNode(true).toElement();
			FAvailFeatures = FXmppStreamManager->xmppFeatures();
			processFeatures();
			return true;
		}
		else if (AStanza.kind() == "error")
		{
			abort(XmppStreamError(AStanza.element()));
			return true;
		}
	}
	return false;
}

bool XmppStream::xmppStanzaOut(IXmppStream *AXmppStream, Stanza &AStanza, int AOrder)
{
	Q_UNUSED(AXmppStream); Q_UNUSED(AStanza); Q_UNUSED(AOrder);
	return false;
}

bool XmppStream::isOpen() const
{
	return FReady && !FClosed;
}

bool XmppStream::isConnected() const
{
	return FStreamState!=SS_OFFLINE;
}

bool XmppStream::open()
{
	if (FConnection && FStreamState==SS_OFFLINE)
	{
		FError = XmppError::null;

		LOG_STRM_INFO(streamJid(),"Opening XMPP stream");
		if (FConnection->connectToHost())
		{
			FNodeChanged = false;
			FDomainChanged = false;
			FOnlineJid = FOfflineJid;
			setStreamState(SS_CONNECTING);
			return true;
		}
		else
		{
			abort(XmppError(IERR_XMPPSTREAM_FAILED_START_CONNECTION));
		}
	}
	else if (!FConnection)
	{
		LOG_STRM_ERROR(streamJid(),"Failed to open XMPP stream: Connection not set");
		emit error(tr("Connection not specified"));
	}
	return false;
}

void XmppStream::close()
{
	if (FConnection && FStreamState!=SS_OFFLINE && FStreamState!=SS_ERROR && FStreamState!=SS_DISCONNECTING)
	{
		LOG_STRM_INFO(streamJid(),"Closing XMPP stream");
		setStreamState(SS_DISCONNECTING);
		if (FConnection->isOpen())
		{
			emit aboutToClose();
			sendData("</stream:stream>");
			LOG_STRM_INFO(streamJid(),"XMPP stream finish request sent");

			setKeepAliveTimerActive(true);
			FClosed = true;
		}
		else
		{
			FClosed = true;
			FConnection->disconnectFromHost();
		}
	}
}

void XmppStream::abort(const XmppError &AError)
{
	if (FStreamState!=SS_OFFLINE && FStreamState!=SS_ERROR)
	{
		LOG_STRM_WARNING(streamJid(),QString("Aborting XMPP stream: %1").arg(AError.condition()));

		if (FStreamState != SS_DISCONNECTING)
		{
			setStreamState(SS_ERROR);
			FError = AError;
			emit error(AError);
		}

		FClosed = true;
		FConnection->disconnectFromHost();
	}
}

QString XmppStream::streamId() const
{
	return FStreamId;
}

XmppError XmppStream::error() const
{
	return FError;
}

Jid XmppStream::streamJid() const
{
	return FStreamJid;
}

void XmppStream::setStreamJid(const Jid &AStreamJid)
{
	if (FStreamJid!=AStreamJid && AStreamJid.isValid())
	{
		if (FStreamState==SS_OFFLINE || !FStreamJid.hasNode())
		{
			LOG_STRM_INFO(streamJid(),QString("Changing offline XMPP stream JID, from=%1, to=%2").arg(FOfflineJid.full(),AStreamJid.full()));

			Jid before = FStreamJid;
			Jid after = AStreamJid;
			emit jidAboutToBeChanged(after);

			FOfflineJid = after;
			FStreamJid = after;
			emit jidChanged(before);
		}
		else if (FStreamState == SS_FEATURES)
		{
			LOG_STRM_INFO(streamJid(),QString("Changing online XMPP stream JID, from=%1, to=%2").arg(FOnlineJid.full(),AStreamJid.full()));

			Jid before = FStreamJid;
			Jid after(FStreamJid.node(),FStreamJid.domain(),AStreamJid.resource());
			emit jidAboutToBeChanged(after);

			FOnlineJid = AStreamJid;
			FStreamJid = after;
			FNodeChanged = FOnlineJid.pNode()!=FOfflineJid.pNode();
			FDomainChanged = FOnlineJid.pDomain()!=FOfflineJid.pDomain();
			emit jidChanged(before);
		}
		else
		{
			LOG_STRM_WARNING(streamJid(),QString("Failed to change stream jid to=%1: Wrong stream state").arg(AStreamJid.full()));
		}
	}
	else if (!AStreamJid.isValid())
	{
		REPORT_ERROR("Failed to change stream jid: Invalid parameters");
	}
}

bool XmppStream::requestPassword()
{
	if (!FPasswordRequested)
	{
		LOG_STRM_DEBUG(streamJid(),QString("XMPP stream password request"));
		emit passwordRequested(FPasswordRequested);
	}
	return FPasswordRequested;
}

QString XmppStream::password() const
{
	return FPassword;
}

void XmppStream::setPassword(const QString &APassword)
{
	if (FPassword != APassword)
	{
		FPassword = APassword;
		LOG_STRM_DEBUG(streamJid(),"XMPP stream password changed");
	}
	if (FPasswordRequested)
	{
		FPasswordRequested = false;
		LOG_STRM_DEBUG(streamJid(),"XMPP stream password provided");
		QMetaObject::invokeMethod(this,"passwordProvided",Qt::QueuedConnection,Q_ARG(QString,APassword));
	}
}

QString XmppStream::defaultLang() const
{
	return FDefLang;
}

void XmppStream::setDefaultLang(const QString &ADefLang)
{
	if (FDefLang != ADefLang)
	{
		FDefLang = ADefLang;
		LOG_STRM_DEBUG(streamJid(),QString("Default XMPP stream language changed to=%1").arg(ADefLang));
	}
}

bool XmppStream::isEncryptionRequired() const
{
	return FEncrypt;
}

void XmppStream::setEncryptionRequired(bool ARequire)
{
	if (FEncrypt != ARequire)
	{
		FEncrypt = ARequire;
		LOG_STRM_DEBUG(streamJid(),QString("XMPP stream encryption require changed to=%1").arg(ARequire));
	}
}

IConnection *XmppStream::connection() const
{
	return FConnection;
}

void XmppStream::setConnection(IConnection *AConnection)
{
	if (FStreamState==SS_OFFLINE && FConnection!=AConnection)
	{
		if (FConnection)
			FConnection->instance()->disconnect(this);

		if (AConnection)
		{
			connect(AConnection->instance(),SIGNAL(connected()),SLOT(onConnectionConnected()));
			connect(AConnection->instance(),SIGNAL(readyRead(qint64)),SLOT(onConnectionReadyRead(qint64)));
			connect(AConnection->instance(),SIGNAL(error(const XmppError &)),SLOT(onConnectionError(const XmppError &)));
			connect(AConnection->instance(),SIGNAL(disconnected()),SLOT(onConnectionDisconnected()));
			LOG_STRM_INFO(streamJid(),QString("XMPP stream connection changed to=%1").arg(AConnection->instance()->metaObject()->className()));
		}
		else
		{
			LOG_STRM_INFO(streamJid(),"XMPP stream connection removed");
		}

		FConnection = AConnection;
		emit connectionChanged(AConnection);
	}
	else if (FStreamState != SS_OFFLINE)
	{
		LOG_STRM_WARNING(streamJid(),"Failed to change XMPP stream connection: Stream is not offline");
	}
}

bool XmppStream::isKeepAliveTimerActive() const
{
	return FKeepAliveTimer.isActive();
}

void XmppStream::setKeepAliveTimerActive(bool AActive)
{
	if (AActive)
	{
		switch (FStreamState)
		{
		case SS_OFFLINE:
		case SS_CONNECTING:
			FKeepAliveTimer.stop();
			break;
		case SS_INITIALIZE:
		case SS_FEATURES:
			FKeepAliveTimer.start(Options::node(OPV_XMPPSTREAMS_TIMEOUT_HANDSHAKE).value().toInt());
			break;
		case SS_ONLINE:
		case SS_ERROR:
			FKeepAliveTimer.start(Options::node(OPV_XMPPSTREAMS_TIMEOUT_KEEPALIVE).value().toInt());
			break;
		case SS_DISCONNECTING:
			FKeepAliveTimer.start(Options::node(OPV_XMPPSTREAMS_TIMEOUT_DISCONNECT).value().toInt());
			break;
		}
	}
	else
	{
		FKeepAliveTimer.stop();
	}
}

qint64 XmppStream::sendStanza(Stanza &AStanza)
{
	if (FStreamState!=SS_OFFLINE && FStreamState!=SS_ERROR)
	{
		if (!FClosed && !processStanzaHandlers(AStanza,true))
		{
			if (FNodeChanged || FDomainChanged)
			{
				Jid toJid = AStanza.to();
				if (FNodeChanged && toJid.pBare()==FOfflineJid.pBare())
				{
					// Replace node and domain of destination
					Jid newToJid = Jid(FOnlineJid.node(),FOnlineJid.domain(),toJid.resource());
					AStanza.setTo(newToJid.full());
				}
				else if (FDomainChanged && toJid.pBare()==FOfflineJid.pDomain())
				{
					// Replace domain of destination
					Jid newToJid = Jid(toJid.node(),FOnlineJid.domain(),toJid.resource());
					AStanza.setTo(newToJid.full());
				}
			}
			return sendData(AStanza.toByteArray());
		}
		else if (FClosed)
		{
			LOG_STRM_WARNING(streamJid(),"Failed to send XMPP stream stanza: XML stream is finished");
		}
	}
	else
	{
		LOG_STRM_WARNING(streamJid(),"Failed to send XMPP stream stanza: Stream is not connected");
	}
	return -1;
}

void XmppStream::insertXmppDataHandler(int AOrder, IXmppDataHandler *AHandler)
{
	if (AHandler && !FDataHandlers.contains(AOrder, AHandler))
	{
		LOG_STRM_DEBUG(streamJid(),QString("XMPP data handler inserted, order=%1, address=%2").arg(AOrder).arg((quint64)AHandler));
		FDataHandlers.insertMulti(AOrder, AHandler);
		emit dataHandlerInserted(AOrder,AHandler);
	}
}

void XmppStream::removeXmppDataHandler(int AOrder, IXmppDataHandler *AHandler)
{
	if (FDataHandlers.contains(AOrder, AHandler))
	{
		LOG_STRM_DEBUG(streamJid(),QString("XMPP data handler removed, order=%1, address=%2").arg(AOrder).arg((quint64)AHandler));
		FDataHandlers.remove(AOrder, AHandler);
		emit dataHandlerRemoved(AOrder,AHandler);
	}
}

void XmppStream::insertXmppStanzaHandler(int AOrder, IXmppStanzaHadler *AHandler)
{
	if (AHandler && !FStanzaHandlers.contains(AOrder, AHandler))
	{
		LOG_STRM_DEBUG(streamJid(),QString("XMPP stanza handler inserted, order=%1, address=%2").arg(AOrder).arg((quint64)AHandler));
		FStanzaHandlers.insertMulti(AOrder, AHandler);
		emit stanzaHandlerInserted(AOrder, AHandler);
	}
}

void XmppStream::removeXmppStanzaHandler(int AOrder, IXmppStanzaHadler *AHandler)
{
	if (FStanzaHandlers.contains(AOrder, AHandler))
	{
		LOG_STRM_DEBUG(streamJid(),QString("XMPP stanza handler removed, order=%1, address=%2").arg(AOrder).arg((quint64)AHandler));
		FStanzaHandlers.remove(AOrder, AHandler);
		emit stanzaHandlerRemoved(AOrder, AHandler);
	}
}

void XmppStream::startStream()
{
	LOG_STRM_DEBUG(streamJid(),"Starting XMPP stream");

	FParser.restart();
	setKeepAliveTimerActive(true);

	Stanza stream("stream:stream",NS_JABBER_STREAMS);
	stream.setAttribute("to", FStreamJid.domain());
	stream.setAttribute("xmlns", NS_JABBER_CLIENT);
	stream.setAttribute("xmlns:xml", NS_XML);
	stream.setAttribute("xml:lang", !FDefLang.isEmpty() ? FDefLang : QLocale().name().split('_').value(0));

	setStreamState(SS_INITIALIZE);
	if (!processStanzaHandlers(stream,true))
	{
		QByteArray data = QString("<?xml version=\"1.0\"?>").toUtf8() + stream.toByteArray().trimmed();
		data.remove(data.size()-2,1);
		sendData(data);
	}
}

void XmppStream::processFeatures()
{
	bool started = false;
	while (!started && !FAvailFeatures.isEmpty())
	{
		QString featureNS = FAvailFeatures.takeFirst();
		QDomElement featureElem = FServerFeatures.firstChildElement();
		while (!featureElem.isNull() && featureElem.namespaceURI()!=featureNS)
			featureElem = featureElem.nextSiblingElement();
		started = !featureElem.isNull() ? startFeature(featureNS, featureElem) : false;
	}
	if (!started)
	{
		if (!isEncryptionRequired() || connection()->isEncrypted())
		{
			FReady = true;
			setStreamState(SS_ONLINE);
			LOG_STRM_INFO(streamJid(),"XMPP stream opened");
			emit opened();
		}
		else
		{
			abort(XmppError(IERR_XMPPSTREAM_NOT_SECURE));
		}
	}
}

void XmppStream::clearActiveFeatures()
{
	foreach(IXmppFeature *feature, FActiveFeatures.toSet())
		delete feature->instance();
	FActiveFeatures.clear();
}

void XmppStream::setStreamState(StreamState AState)
{
	if (FStreamState != AState)
	{
		LOG_STRM_DEBUG(streamJid(),QString("XMPP stream state changed to=%1").arg(AState));
		FStreamState = AState;
	}
}

bool XmppStream::startFeature(const QString &AFeatureNS, const QDomElement &AFeatureElem)
{
	LOG_STRM_DEBUG(streamJid(),QString("Starting XMPP stream feature=%1").arg(AFeatureNS));
	foreach(IXmppFeatureFactory *factory, FXmppStreamManager->xmppFeatureFactories(AFeatureNS))
	{
		IXmppFeature *feature = factory->newXmppFeature(AFeatureNS, this);
		if (feature && feature->start(AFeatureElem))
		{
			FActiveFeatures.append(feature);
			connect(feature->instance(),SIGNAL(finished(bool)),SLOT(onFeatureFinished(bool)));
			connect(feature->instance(),SIGNAL(error(const XmppError &)),SLOT(onFeatureError(const XmppError &)));
			connect(feature->instance(),SIGNAL(featureDestroyed()),SLOT(onFeatureDestroyed()));
			connect(this,SIGNAL(closed()),feature->instance(),SLOT(deleteLater()));
			return true;
		}
		else if (feature)
		{
			feature->instance()->deleteLater();
		}
	}
	return false;
}

bool XmppStream::processDataHandlers(QByteArray &AData, bool ADataOut)
{
	bool hooked = false;
	QMapIterator<int, IXmppDataHandler *> it(FDataHandlers);
	if (!ADataOut)
		it.toBack();
	while (!hooked && (ADataOut ? it.hasNext() : it.hasPrevious()))
	{
		if (ADataOut)
		{
			it.next();
			hooked = it.value()->xmppDataOut(this, AData, it.key());
		}
		else
		{
			it.previous();
			hooked = it.value()->xmppDataIn(this, AData, it.key());
		}
	}
	return hooked;
}

bool XmppStream::processStanzaHandlers(Stanza &AStanza, bool AStanzaOut)
{
	if (!AStanzaOut)
		LOG_STRM_TYPE(Logger::Stanza,streamJid(),QString("Stanza received:\n\n%1").arg(AStanza.toString(2)));

	bool hooked = false;
	QMapIterator<int, IXmppStanzaHadler *> it(FStanzaHandlers);
	if (!AStanzaOut)
	{
		AStanza.setTo(FStreamJid.full());
		it.toBack();
	}
	while (!hooked && (AStanzaOut ? it.hasNext() : it.hasPrevious()))
	{
		if (AStanzaOut)
		{
			it.next();
			hooked = it.value()->xmppStanzaOut(this, AStanza, it.key());
		}
		else
		{
			it.previous();
			hooked = it.value()->xmppStanzaIn(this, AStanza, it.key());
		}
	}

	if (AStanzaOut && !hooked)
		LOG_STRM_TYPE(Logger::Stanza,streamJid(),QString("Stanza sent:\n\n%1").arg(AStanza.toString(2)));

	return hooked;
}

qint64 XmppStream::sendData(QByteArray AData)
{
	if (!processDataHandlers(AData,true))
	{
		setKeepAliveTimerActive(true);
		return FConnection->write(AData);
	}
	return 0;
}

QByteArray XmppStream::receiveData(qint64 ABytes)
{
	return FConnection->read(ABytes);
}

void XmppStream::onConnectionConnected()
{
	if (FStreamState!=SS_OFFLINE && FStreamState!=SS_ERROR)
	{
		FClosed = false;
		insertXmppStanzaHandler(XSHO_XMPP_STREAM,this);
		startStream();
	}
}

void XmppStream::onConnectionReadyRead(qint64 ABytes)
{
	if (FStreamState!=SS_OFFLINE && FStreamState!=SS_ERROR)
	{
		QByteArray data = receiveData(ABytes);
		if (!processDataHandlers(data,false))
			if (!data.isEmpty())
				FParser.parseData(data);
	}
}

void XmppStream::onConnectionError(const XmppError &AError)
{
	abort(AError);
}

void XmppStream::onConnectionDisconnected()
{
	if (FStreamState != SS_OFFLINE)
	{
		FReady = false;
		FClosed = true;

		if (FStreamState != SS_DISCONNECTING)
			abort(XmppError(IERR_XMPPSTREAM_CLOSED_UNEXPECTEDLY));

		setStreamState(SS_OFFLINE);
		setKeepAliveTimerActive(false);
		removeXmppStanzaHandler(XSHO_XMPP_STREAM,this);

		LOG_STRM_INFO(streamJid(),"XMPP stream closed");
		emit closed();

		clearActiveFeatures();
		setStreamJid(FOfflineJid);

		FNodeChanged = false;
		FDomainChanged = false;
		FPasswordRequested = false;
		FOnlineJid = Jid::null;
	}
}

void XmppStream::onParserOpened(const QDomElement &AElem)
{
	Stanza stanza(AElem);
	processStanzaHandlers(stanza,false);
}

void XmppStream::onParserElement(const QDomElement &AElem)
{
	Stanza stanza(AElem);
	processStanzaHandlers(stanza,false);
}

void XmppStream::onParserError(const XmppError &AError)
{
	static const QString xmlError(
		"<stream:error>"
			"<xml-not-well-formed xmlns='urn:ietf:params:xml:ns:xmpp-streams'/>"
			"<text xmlns='urn:ietf:params:xml:ns:xmpp-streams'>%1</text>"
		"</stream:error></stream:stream>");

	sendData(xmlError.arg(AError.errorText()).toUtf8());
	abort(AError);
}

void XmppStream::onParserClosed()
{
	FClosed = true;
	LOG_STRM_INFO(streamJid(),"XMPP stream finished");
	FConnection->disconnectFromHost();
}

void XmppStream::onFeatureFinished(bool ARestart)
{
	if (!ARestart)
		processFeatures();
	else
		startStream();
}

void XmppStream::onFeatureError(const XmppError &AError)
{
	abort(AError);
}

void XmppStream::onFeatureDestroyed()
{
	IXmppFeature *feature = qobject_cast<IXmppFeature *>(sender());
	if (feature)
		FActiveFeatures.removeAll(feature);
}

void XmppStream::onKeepAliveTimeout()
{
	static const QByteArray space(1,' ');
	if (FStreamState == SS_DISCONNECTING)
		FConnection->disconnectFromHost();
	else if (FStreamState != SS_ONLINE)
		abort(XmppStreamError(XmppStreamError::EC_CONNECTION_TIMEOUT));
	else
		sendData(space);
}

#include "defaultconnection.h"
#include <QSslConfiguration>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <definitions/internalerrors.h>
#include <utils/logger.h>

#define DISCONNECT_TIMEOUT    5000

DefaultConnection::DefaultConnection(IConnectionEngine *AEngine, QObject *AParent) : QObject(AParent)
{
	FEngine = AEngine;
	FSSLError = false;
	FUseLegacySSL = false;
	FVerifyMode = CertificateVerifyMode::Disabled;
	FDisconnecting = false;
	FDnsLookup.setType(QPDnsLookup::SRV);

	// Make FDnsLookup.isFinished to be true
	FDnsLookup.lookup();
	FDnsLookup.abort();
	connect(&FDnsLookup,SIGNAL(finished()),SLOT(onDnsLookupFinished()));

	FSocket.setSocketOption(QAbstractSocket::KeepAliveOption,1);
	connect(&FSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
		SLOT(onSocketProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
	connect(&FSocket, SIGNAL(connected()), SLOT(onSocketConnected()));
	connect(&FSocket, SIGNAL(encrypted()), SLOT(onSocketEncrypted()));
	connect(&FSocket, SIGNAL(readyRead()), SLOT(onSocketReadyRead()));
	connect(&FSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onSocketError(QAbstractSocket::SocketError)));
	connect(&FSocket, SIGNAL(sslErrors(const QList<QSslError> &)), SLOT(onSocketSSLErrors(const QList<QSslError> &)));
	connect(&FSocket, SIGNAL(disconnected()), SLOT(onSocketDisconnected()));
}

DefaultConnection::~DefaultConnection()
{
	disconnectFromHost();
	emit connectionDestroyed();
}

bool DefaultConnection::isOpen() const
{
	return FSocket.state() == QAbstractSocket::ConnectedState;
}

bool DefaultConnection::isEncrypted() const
{
	return FSocket.isEncrypted();
}

bool DefaultConnection::isEncryptionSupported() const
{
	return FSocket.supportsSsl();
}

bool DefaultConnection::connectToHost()
{
	if (FDnsLookup.isFinished() && FSocket.state()==QAbstractSocket::UnconnectedState)
	{
		emit aboutToConnect();

		FRecords.clear();
		FSSLError = false;

		QString host = option(IDefaultConnection::Host).toString();
		quint16 port = option(IDefaultConnection::Port).toInt();
		QString domain = option(IDefaultConnection::Domain).toString();
		FUseLegacySSL = option(IDefaultConnection::UseLegacySsl).toBool();
		FVerifyMode = (CertificateVerifyMode)option(IDefaultConnection::CertVerifyMode).toInt();
		SrvRecord record;
		record.target = !host.isEmpty() ? host : domain;
		record.port = port;
		FRecords.append(record);
		if (host.isEmpty())
		{
			LOG_DEBUG(QString("Starting DNS SRV lookup, domain=%1").arg(domain));
			FDnsLookup.setName(QString("_xmpp-client._tcp.%1.").arg(domain));
			FDnsLookup.lookup();
		}
		else
		{
			LOG_ERROR("Failed to init DNS SRV lookup");
			connectToNextHost();
		}
		return true;
	}
	else
	{
		LOG_ERROR("Failed to start connection to host: Previous connection is not finished");
	}
	return false;
}

bool DefaultConnection::startEncryption()
{
	FSocket.startClientEncryption();
	return true;
}

void DefaultConnection::disconnectFromHost()
{
	if (!FDisconnecting)
	{
		FRecords.clear();
		FDisconnecting = true;

		if (FSocket.state() != QSslSocket::UnconnectedState)
		{
			LOG_INFO(QString("Disconnecting from host=%1").arg(FSocket.peerName()));

			if (FSocket.state() == QSslSocket::ConnectedState)
			{
				emit aboutToDisconnect();
				FSocket.flush();
				FSocket.disconnectFromHost();
			}
			else
			{
				FSocket.abort();
				emit disconnected();
			}
		}

		else if (!FDnsLookup.isFinished())
		{
			FDnsLookup.abort();
		}

		emit disconnected();

		FDisconnecting = false;
	}
}

void DefaultConnection::abortConnection(const XmppError &AError)
{
	if (!FDisconnecting && FSocket.state()!=QSslSocket::UnconnectedState)
	{
		LOG_WARNING(QString("Aborting connection to host=%1: %2").arg(FSocket.peerName(),AError.condition()));
		emit error(AError);
		disconnectFromHost();
	}
}

qint64 DefaultConnection::write(const QByteArray &AData)
{
	return FSocket.write(AData);
}

QByteArray DefaultConnection::read(qint64 ABytes)
{
	return FSocket.read(ABytes);
}

IConnectionEngine *DefaultConnection::engine() const
{
	return FEngine;
}

QSslCertificate DefaultConnection::hostCertificate() const
{
	return FSocket.peerCertificate();
}

void DefaultConnection::ignoreSslErrors()
{
	FSSLError = false;
	FSocket.ignoreSslErrors();
}

QList<QSslError> DefaultConnection::sslErrors() const
{
	return FSocket.sslErrors();
}

QSsl::SslProtocol DefaultConnection::protocol() const
{
	return FSocket.protocol();
}

void DefaultConnection::setProtocol(QSsl::SslProtocol AProtocol)
{
	FSocket.setProtocol(AProtocol);
}

QSslKey DefaultConnection::privateKey() const
{
	return FSocket.privateKey();
}

void DefaultConnection::setPrivateKey(const QSslKey &AKey)
{
	FSocket.setPrivateKey(AKey);
}

QSslCertificate DefaultConnection::localCertificate() const
{
	return FSocket.localCertificate();
}

void DefaultConnection::setLocalCertificate(const QSslCertificate &ACertificate)
{
	FSocket.setLocalCertificate(ACertificate);
}

QList<QSslCertificate> DefaultConnection::caCertificates() const
{
	return FSocket.sslConfiguration().caCertificates();
}

void DefaultConnection::addCaSertificates(const QList<QSslCertificate> &ACertificates)
{
	QList<QSslCertificate> curSerts = caCertificates();
	foreach(const QSslCertificate &cert, ACertificates)
	{
		if (!cert.isNull() && !curSerts.contains(cert))
			FSocket.addCaCertificate(cert);
	}
}

void DefaultConnection::setCaCertificates(const QList<QSslCertificate> &ACertificates)
{
	FSocket.sslConfiguration().setCaCertificates(ACertificates);
}

QNetworkProxy DefaultConnection::proxy() const
{
	return FSocket.proxy();
}

void DefaultConnection::setProxy(const QNetworkProxy &AProxy)
{
	if (AProxy!= FSocket.proxy())
	{
		LOG_INFO(QString("Connection proxy changed, host=%1, port=%2").arg(AProxy.hostName()).arg(AProxy.port()));
		FSocket.setProxy(AProxy);
		emit proxyChanged(AProxy);
	}
}

QVariant DefaultConnection::option(int ARole) const
{
	return FOptions.value(ARole);
}

void DefaultConnection::setOption(int ARole, const QVariant &AValue)
{
	FOptions.insert(ARole, AValue);
}

void DefaultConnection::connectToNextHost()
{
	if (!FRecords.isEmpty())
	{
		SrvRecord record = FRecords.takeFirst();

		if (FUseLegacySSL)
		{
			LOG_INFO(QString("Connecting to host with encryption, host=%1, port=%2").arg(record.target).arg(record.port));
			FSocket.connectToHostEncrypted(record.target, record.port);
		}
		else
		{
			LOG_INFO(QString("Connecting to host=%1, port=%2").arg(record.target).arg(record.port));
			FSocket.connectToHost(record.target, record.port);
		}
	}
}

void DefaultConnection::onDnsLookupFinished()
{
	QList<QPDnsServiceRecord> dnsRecords = FDnsLookup.serviceRecords();
	LOG_DEBUG(QString("SRV records received, count=%1").arg(dnsRecords.count()));
	if (!dnsRecords.isEmpty())
	{
		FRecords.clear();
		foreach (const QPDnsServiceRecord &dnsRecord, dnsRecords)
		{
			SrvRecord srvRecord;
			srvRecord.target = dnsRecord.target();
			srvRecord.port = dnsRecord.port();
			FRecords.append(srvRecord);
		}
	}
	else if (FDnsLookup.error()!=QPDnsLookup::NoError)
		LOG_ERROR(QString("SRV resolve failed! error: %1(%2)").arg(FDnsLookup.error()).arg(FDnsLookup.errorString()));

	if (!FRecords.isEmpty())
		connectToNextHost();
}

void DefaultConnection::onSocketProxyAuthenticationRequired(const QNetworkProxy &AProxy, QAuthenticator *AAuth)
{
	LOG_INFO(QString("Proxy authentication requested, host=%1, proxy=%2, user=%3").arg(FSocket.peerName(),AProxy.hostName(),AProxy.user()));
	AAuth->setUser(AProxy.user());
	AAuth->setPassword(AProxy.password());
}

void DefaultConnection::onSocketConnected()
{
	LOG_INFO(QString("Socket connected, host=%1").arg(FSocket.peerName()));
	if (!FUseLegacySSL)
	{
		FRecords.clear();
		emit connected();
	}
}

void DefaultConnection::onSocketEncrypted()
{
	LOG_INFO(QString("Socket encrypted, host=%1").arg(FSocket.peerName()));
	if (FVerifyMode!=IDefaultConnection::TrustedOnly || caCertificates().contains(hostCertificate()))
	{
		emit encrypted();
		if (FUseLegacySSL)
		{
			FRecords.clear();
			emit connected();
		}
	}
	else
	{
		abortConnection(XmppError(IERR_DEFAULTCONNECTION_CERT_NOT_TRUSTED));
	}
}

void DefaultConnection::onSocketReadyRead()
{
	emit readyRead(FSocket.bytesAvailable());
}

void DefaultConnection::onSocketSSLErrors(const QList<QSslError> &AErrors)
{
	LOG_INFO(QString("Socket SSL errors occurred, host=%1, verify=%2").arg(FSocket.peerName()).arg(FVerifyMode));
	if (FVerifyMode == IDefaultConnection::Disabled)
	{
		ignoreSslErrors();
	}
	else if (FVerifyMode == IDefaultConnection::TrustedOnly)
	{
		ignoreSslErrors();
	}
	else
	{
		FSSLError = true;
		emit sslErrorsOccured(AErrors);
	}
}

void DefaultConnection::onSocketError(QAbstractSocket::SocketError AError)
{
	Q_UNUSED(AError);
	LOG_INFO(QString("Socket error, host=%1: %2").arg(FSocket.peerName(),FSocket.errorString()));
	if (FRecords.isEmpty())
	{
		if (FSocket.state()!=QSslSocket::ConnectedState || FSSLError)
		{
			emit error(XmppError(IERR_CONNECTIONMANAGER_CONNECT_ERROR,FSocket.errorString()));
			emit disconnected();
		}
		else if (!FDisconnecting || AError!=QAbstractSocket::RemoteHostClosedError)
		{
			emit error(XmppError(IERR_CONNECTIONMANAGER_CONNECT_ERROR,FSocket.errorString()));
		}
	}
	else
	{
		connectToNextHost();
	}
}

void DefaultConnection::onSocketDisconnected()
{
	LOG_INFO(QString("Socket disconnected, host=%1").arg(FSocket.peerName()));
	emit disconnected();
}

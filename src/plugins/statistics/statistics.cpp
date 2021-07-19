#include "statistics.h"

#include <QScreen>
#include <QApplication>
#include <QDir>
#include <QSslError>
#include <QDataStream>
#include <QAuthenticator>
#include <QNetworkRequest>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#if QT_VERSION >= 0x050900
#include <QOperatingSystemVersion>
#endif
#endif
#include <definitions/version.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/statisticsparams.h>
#include <utils/filecookiejar.h>
#include <utils/options.h>
#include <utils/logger.h>

#ifdef Q_OS_WIN32
#	include <windows.h>
#endif

#define MP_VER                       "1"
// *** <<< eyeCU <<< ***
#define MP_ID                        "UA-58579215-1"
// *** >>> eyeCU >>> ***
#if !defined(DEBUG_MODE)
#  define MP_URL                     "https://www.google-analytics.com/collect"
#else
#  define MP_URL                     "https://www.google-analytics.com/debug/collect"
#endif

#define DIR_STATISTICS               "statistics"
#define FILE_COOKIES                 "cookies.dat"
#define RESEND_TIMEOUT               60*1000
#define SESSION_TIMEOUT              5*60*1000

QDataStream &operator>>(QDataStream &AStream, IStatisticsHit& AHit)
{
	AStream >> AHit.type;
	AStream >> AHit.session;
	AStream >> AHit.profile;
	AStream >> AHit.screen;
	AStream >> AHit.timestamp;

	AStream >> AHit.event.category;
	AStream >> AHit.event.action;
	AStream >> AHit.event.label;
	AStream >> AHit.event.value;

	AStream >> AHit.timing.category;
	AStream >> AHit.timing.variable;
	AStream >> AHit.timing.label;
	AStream >> AHit.timing.time;

	AStream >> AHit.exception.descr;
	AStream >> AHit.exception.fatal;

	return AStream;
}

QDataStream &operator<<(QDataStream &AStream, const IStatisticsHit &AHit)
{
	AStream << AHit.type;
	AStream << AHit.session;
	AStream << AHit.profile;
	AStream << AHit.screen;
	AStream << AHit.timestamp;
	
	AStream << AHit.event.category;
	AStream << AHit.event.action;
	AStream << AHit.event.label;
	AStream << AHit.event.value;

	AStream << AHit.timing.category;
	AStream << AHit.timing.variable;
	AStream << AHit.timing.label;
	AStream << AHit.timing.time;

	AStream << AHit.exception.descr;
	AStream << AHit.exception.fatal;

	return AStream;
}

Statistics::Statistics()
{
	FBookmarks = NULL;
	FDiscovery = NULL;
	FClientInfo = NULL;
	FStatusChanger = NULL;
	FPluginManager = NULL;
	FRosterManager = NULL;
	FOptionsManager = NULL;
	FAccountManager = NULL;
	FMessageWidgets = NULL;
	FConnectionManager = NULL;
	FXmppStreamManager = NULL;
	FMultiChatManager = NULL;

	FSendHits = true;
	FDesktopWidget = new QDesktopWidget;

	FNetworkManager = new QNetworkAccessManager(this);
	connect(FNetworkManager,SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
		SLOT(onNetworkManagerProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
	connect(FNetworkManager,SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),
		SLOT(onNetworkManagerSSLErrors(QNetworkReply *, const QList<QSslError> &)));
	connect(FNetworkManager,SIGNAL(finished(QNetworkReply *)),SLOT(onNetworkManagerFinished(QNetworkReply *)));

	FPendingTimer.setSingleShot(true);
	connect(&FPendingTimer,SIGNAL(timeout()),SLOT(onPendingHitsTimerTimeout()));

	FSessionTimer.setSingleShot(false);
	FSessionTimer.setInterval(SESSION_TIMEOUT);
	connect(&FSessionTimer,SIGNAL(timeout()),SLOT(onSessionTimerTimeout()));

	connect(Logger::instance(),SIGNAL(viewReported(const QString &)),
		SLOT(onLoggerViewReported(const QString &)));
	connect(Logger::instance(),SIGNAL(errorReported(const QString &, const QString &, bool)),
		SLOT(onLoggerErrorReported(const QString &, const QString &, bool)));
	connect(Logger::instance(),SIGNAL(eventReported(const QString &, const QString &, const QString &, const QString &, qint64)),
		SLOT(onLoggerEventReported(const QString &, const QString &, const QString &, const QString &, qint64)));
	connect(Logger::instance(),SIGNAL(timingReported(const QString &, const QString &, const QString &, const QString &, qint64)),
		SLOT(onLoggerTimingReported(const QString &, const QString &, const QString &, const QString &, qint64)));
}

Statistics::~Statistics()
{
	if (!FPendingHits.isEmpty())
		LOG_WARNING(QString("Failed to send pending statistics hints, count=%1").arg(FPendingHits.count()));
	delete FDesktopWidget;
}

void Statistics::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Statistics");
	APluginInfo->description = tr("Allows to collect application statistics");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool Statistics::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	FPluginManager = APluginManager;

	IPlugin *plugin = APluginManager->pluginInterface("IConnectionManager").value(0,NULL);
	if (plugin)
	{
		FConnectionManager = qobject_cast<IConnectionManager *>(plugin->instance());
		if (FConnectionManager)
			connect(FConnectionManager->instance(),SIGNAL(defaultProxyChanged(const QUuid &)),SLOT(onDefaultConnectionProxyChanged(const QUuid &)));
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
		if (FXmppStreamManager)
		{
			connect(FXmppStreamManager->instance(),SIGNAL(streamOpened(IXmppStream *)),SLOT(onXmppStreamOpened(IXmppStream *)));
		}
	}

	plugin = APluginManager->pluginInterface("IClientInfo").value(0,NULL);
	if (plugin)
	{
		FClientInfo = qobject_cast<IClientInfo *>(plugin->instance());
		if (FClientInfo)
		{
			connect(FClientInfo->instance(),SIGNAL(softwareInfoChanged(const Jid &)),SLOT(onSoftwareInfoChanged(const Jid &)));
		}
	}

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
	if (plugin)
	{
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMultiUserChatManager").value(0,NULL);
	if (plugin)
	{
		FMultiChatManager = qobject_cast<IMultiUserChatManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
	if (plugin)
	{
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IBookmarks").value(0,NULL);
	if (plugin)
	{
		FBookmarks = qobject_cast<IBookmarks *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IStatusChanger").value(0,NULL);
	if (plugin)
	{
		FStatusChanger = qobject_cast<IStatusChanger *>(plugin->instance());
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return true;
}

bool Statistics::initObjects()
{
	if (FPluginManager->revisionDate().isValid())
		FClientVersion = QString("%1.%2").arg(FPluginManager->version(),FPluginManager->revisionDate().date().toString("yyyyMMdd"));
	else
		FClientVersion = QString("%1.0").arg(FPluginManager->version());
	LOG_DEBUG(QString("Statistics application name=%1 and version=%2").arg(CLIENT_NAME).arg(FClientVersion));

	FUserAgent = userAgent();
	LOG_DEBUG(QString("Statistics user-agent header=%1").arg(FUserAgent));

	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

bool Statistics::initSettings()
{
	Options::setDefaultValue(OPV_COMMON_STATISTICTS_ENABLED,true);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> Statistics::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_COMMON)
	{
		widgets.insertMulti(OWO_COMMON_SENDSTATISTICS,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_COMMON_STATISTICTS_ENABLED),tr("Send anonymous statistics to developer"),AParent));
	}
	return widgets;
}

QUuid Statistics::profileId() const
{
	return FProfileId;
}

bool Statistics::isValidHit(const IStatisticsHit &AHit) const
{
	if (AHit.screen.isEmpty() || AHit.screen.size()>2048)
		return false;

	if (!AHit.timestamp.isValid() || AHit.timestamp>QDateTime::currentDateTime())
		return false;

	for (QMap<int, qint64>::const_iterator it=AHit.metrics.constBegin(); it!=AHit.metrics.constEnd(); ++it)
		if (it.key() > 20)
			return false;

	for (QMap<int, QString>::const_iterator it=AHit.dimensions.constBegin(); it!=AHit.dimensions.constEnd(); ++it)
		if (it.key()>20 || it.value().size() > 150)
			return false;

	switch (AHit.type)
	{
	case IStatisticsHit::HitView:
		break;
	case IStatisticsHit::HitEvent:
		if (AHit.event.category.isEmpty() || AHit.event.category.size()>150)
			return false;
		if (AHit.event.action.isEmpty() || AHit.event.action.size()>500)
			return false;
		if (AHit.event.label.size() > 500)
			return false;
		if (AHit.event.value < 0)
			return false;
		break;
	case IStatisticsHit::HitTiming:
		if (AHit.timing.category.isEmpty() || AHit.timing.category.size()>150)
			return false;
		if (AHit.timing.variable.isEmpty() || AHit.timing.variable.size()>500)
			return false;
		if (AHit.timing.time < 0)
			return false;
		break;
	case IStatisticsHit::HitException:
		if (AHit.exception.descr.isEmpty() || AHit.exception.descr.size()>150)
			return false;
		break;
	default:
		return false;
	}

	return true;
}

bool Statistics::sendStatisticsHit(const IStatisticsHit &AHit)
{
	if (FSendHits && isValidHit(AHit))
	{
		if (!FProfileId.isNull() || !AHit.profile.isNull())
		{
			QNetworkRequest request(buildHitUrl(AHit));
			request.setRawHeader("User-Agent",FUserAgent.toUtf8());
			QNetworkReply *reply = FNetworkManager->get(request);
			if (!reply->isFinished())
			{
				FReplyHits.insert(reply,AHit);
				FPluginManager->delayShutdown();
			}
		}
		else
		{
			FPendingHits.append(AHit);
			FPendingTimer.start(RESEND_TIMEOUT);
		}
		return true;
	}
	else if (FSendHits)
	{
		LOG_ERROR(QString("Failed to send statistics hit, type=%1, screen=%2: Invalid hit").arg(AHit.type).arg(AHit.screen));
	}
	return false;
}

QString Statistics::userAgent() const
{
	static QString firstPart;
	static QString secondPart;
	static QString thirdPart;

	if (firstPart.isNull() || secondPart.isNull() || thirdPart.isNull())
	{
		QString firstPartTemp;
		firstPartTemp.reserve(150);
		firstPartTemp += QString::fromLatin1(CLIENT_NAME)+QString::fromLatin1("/")+FPluginManager->version();

		firstPartTemp += QString::fromLatin1(" ("
			// Platform
#ifdef Q_OS_MACOS
			"Macintosh; "
#elif defined Q_WS_QWS
			"QtEmbedded; "
#elif defined Q_WS_MAEMO_5
			"Maemo"
#elif defined Q_WS_MAEMO_6
			"MeeGo"
#elif defined Q_OS_WIN
			// Nothing
#elif defined Q_WS_X11
			"X11; "
// *** <<< eyeCU <<< ***
#elif defined Q_OS_OS2
			"OS/2; "
// *** >>> eyeCU >>> ***
#else
			"Unknown; "
#endif
			);

#if defined(QT_NO_OPENSSL)
		// No SSL support
		firstPartTemp += QString::fromLatin1("N; ");
#endif

		// Operating system
#ifdef Q_OS_AIX
		firstPartTemp += QString::fromLatin1("AIX");
#elif defined Q_OS_WIN
#if QT_VERSION >= 0x050900
		firstPartTemp += windowsVersion();
#endif
#elif defined Q_OS_DARWIN
#if defined(__powerpc__)
		firstPartTemp += QString::fromLatin1("PPC Mac OS X");
#else
		firstPartTemp += QString::fromLatin1("Intel Mac OS X");
#endif

#elif defined Q_OS_BSDI
		firstPartTemp += QString::fromLatin1("BSD");
#elif defined Q_OS_BSD4
		firstPartTemp += QString::fromLatin1("BSD Four");
#elif defined Q_OS_CYGWIN
		firstPartTemp += QString::fromLatin1("Cygwin");
#elif defined Q_OS_DGUX
		firstPartTemp += QString::fromLatin1("DG/UX");
#elif defined Q_OS_DYNIX
		firstPartTemp += QString::fromLatin1("DYNIX/ptx");
#elif defined Q_OS_FREEBSD
		firstPartTemp += QString::fromLatin1("FreeBSD");
#elif defined Q_OS_HPUX
		firstPartTemp += QString::fromLatin1("HP-UX");
#elif defined Q_OS_HURD
		firstPartTemp += QString::fromLatin1("GNU Hurd");
#elif defined Q_OS_IRIX
		firstPartTemp += QString::fromLatin1("SGI Irix");
#elif defined Q_OS_LINUX
#if !defined(Q_WS_MAEMO_5) && !defined(Q_WS_MAEMO_6)
#if defined(__x86_64__)
		firstPartTemp += QString::fromLatin1("Linux x86_64");
#elif defined(__i386__)
		firstPartTemp += QString::fromLatin1("Linux i686");
#else
		firstPartTemp += QString::fromLatin1("Linux");
#endif
#endif
#elif defined Q_OS_LYNX
		firstPartTemp += QString::fromLatin1("LynxOS");
#elif defined Q_OS_NETBSD
		firstPartTemp += QString::fromLatin1("NetBSD");
#elif defined Q_OS_OS2
		firstPartTemp += QString::fromLatin1("OS/2");
#elif defined Q_OS_OPENBSD
		firstPartTemp += QString::fromLatin1("OpenBSD");
#elif defined Q_OS_OS2EMX
		firstPartTemp += QString::fromLatin1("OS/2");
#elif defined Q_OS_OSF
		firstPartTemp += QString::fromLatin1("HP Tru64 UNIX");
#elif defined Q_OS_QNX6
		firstPartTemp += QString::fromLatin1("QNX RTP Six");
#elif defined Q_OS_QNX
		firstPartTemp += QString::fromLatin1("QNX");
#elif defined Q_OS_RELIANT
		firstPartTemp += QString::fromLatin1("Reliant UNIX");
#elif defined Q_OS_SCO
		firstPartTemp += QString::fromLatin1("SCO OpenServer");
#elif defined Q_OS_SOLARIS
		firstPartTemp += QString::fromLatin1("Sun Solaris");
#elif defined Q_OS_ULTRIX
		firstPartTemp += QString::fromLatin1("DEC Ultrix");
#elif defined Q_OS_UNIX
		firstPartTemp += QString::fromLatin1("UNIX BSD/SYSV system");
#elif defined Q_OS_UNIXWARE
		firstPartTemp += QString::fromLatin1("UnixWare Seven, Open UNIX Eight");
#else
		firstPartTemp += QString::fromLatin1("Unknown");
#endif
		firstPartTemp += QString::fromLatin1(")");
		firstPartTemp.squeeze();
		firstPart = firstPartTemp;

		secondPart = QString::fromLatin1("Qt/") + QString::fromLatin1(qVersion());

		QString thirdPartTemp;
		thirdPartTemp.reserve(150);
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(Q_WS_MAEMO_6)
		thirdPartTemp += QString::fromLatin1("Mobile Safari/");
#else
		thirdPartTemp += QString::fromLatin1("Safari/");
#endif
		thirdPartTemp += QString::fromLatin1(QT_VERSION_STR);
		thirdPartTemp.squeeze();
		thirdPart = thirdPartTemp;

		Q_ASSERT(!firstPart.isNull());
		Q_ASSERT(!secondPart.isNull());
		Q_ASSERT(!thirdPart.isNull());
	}

	return firstPart + " " + secondPart+ " " + thirdPart;
}
#if QT_VERSION >= 0x050900
QString Statistics::windowsVersion() const
{
#if QT_VERSION > 0x050900 && defined Q_OS_WIN
	QOperatingSystemVersion currentWindows = QOperatingSystemVersion::current();
	int majorVersion = currentWindows.majorVersion();
	int minorVersion = currentWindows.minorVersion();

	return QString("Windows NT %1.%2").arg(majorVersion).arg(minorVersion);
#endif
	return QString();
}
#endif
QUrl Statistics::buildHitUrl(const IStatisticsHit &AHit) const
{
	QUrl url(MP_URL);
#if QT_VERSION < 0x050000
	url.setQueryDelimiters('=','&');
#define APPEND_QUERY(NAME,VALUE) url.addQueryItem(NAME,VALUE)
#else
	QList< QPair<QString,QString> > query;
#define APPEND_QUERY(NAME,VALUE) query.append(qMakePair<QString,QString>(NAME,QUrl::toPercentEncoding(VALUE)))
#endif
	// Protocol Version
	APPEND_QUERY("v",MP_VER);

	// Tracking ID
	APPEND_QUERY("tid",MP_ID);

	// Queue Time
	qint64 qt =
#if QT_VERSION >= 0x040700
				AHit.timestamp.msecsTo(QDateTime::currentDateTime());
#else
				AHit.timestamp.time().msecsTo(QTime::currentTime());
#endif
	if (qt > 0)
		APPEND_QUERY("qt",QString::number(qt));

	// Client ID
	QString cid = !AHit.profile.isNull() ? AHit.profile.toString() : FProfileId.toString();
	cid.remove(0,1); cid.chop(1);
	APPEND_QUERY("cid",cid);

	// Session Control
	if (AHit.session == IStatisticsHit::SessionStart)
		APPEND_QUERY("sc","start");
	else if (AHit.session == IStatisticsHit::SessionEnd)
		APPEND_QUERY("sc","end");

	// Screen Resolution
#if QT_VERSION >= 0x050000
	QRect sr =  QApplication::primaryScreen()->availableGeometry();
#else
	QRect sr =  QApplication::desktop()->screenGeometry();
#endif
	APPEND_QUERY("sr",QString("%1.%2").arg(sr.width()).arg(sr.height()));

	// User Language
	APPEND_QUERY("ul",QLocale().name());

	// Flash Version (Qt Version)
	APPEND_QUERY("fl",qVersion());

	// Screen Name
	if (!AHit.screen.isEmpty())
		APPEND_QUERY("cd",AHit.screen);

	// Application Name
	APPEND_QUERY("an",CLIENT_NAME);

	// Application Version
	APPEND_QUERY("av",FClientVersion);

	// Custom Metric
	for (QMap<int, qint64>::const_iterator it=AHit.metrics.constBegin(); it!=AHit.metrics.constEnd(); ++it)
		APPEND_QUERY(QString("cm%1").arg(it.key()),QString::number(it.value()));

	// Custom Dimension
	for (QMap<int, QString>::const_iterator it=AHit.dimensions.constBegin(); it!=AHit.dimensions.constEnd(); ++it)
		APPEND_QUERY(QString("cd%1").arg(it.key()),it.value());

	if (AHit.type == IStatisticsHit::HitView)
	{
		// Hit Type
		APPEND_QUERY("t","screenview");
	}
	else if (AHit.type == IStatisticsHit::HitEvent)
	{
		// Hit Type
		APPEND_QUERY("t","event");

		// Event Category
		APPEND_QUERY("ec",AHit.event.category);

		// Event Action
		APPEND_QUERY("ea",AHit.event.action);

		// Event Label
		if (!AHit.event.label.isEmpty())
			APPEND_QUERY("el",AHit.event.label);

		// Event Value
		if (AHit.event.value >= 0)
			APPEND_QUERY("ev",QString::number(AHit.event.value));
	}
	else if (AHit.type == IStatisticsHit::HitTiming)
	{
		// Hit Type
		APPEND_QUERY("t","timing");
		// User timing category
		APPEND_QUERY("utc",AHit.timing.category);
		// User timing variable name
		APPEND_QUERY("utv",AHit.timing.variable);
		// User timing time
		APPEND_QUERY("utt",QString::number(AHit.timing.time));

		// User timing label
		if (!AHit.timing.label.isEmpty())
			APPEND_QUERY("utl",AHit.timing.label);
	}
	else if (AHit.type == IStatisticsHit::HitException)
	{
		// Hit Type
		APPEND_QUERY("t","exception");
		// Exception Description
		APPEND_QUERY("exd",AHit.exception.descr);
		// Is Exception Fatal?
		APPEND_QUERY("exf",AHit.exception.fatal ? "1" : "0");
	}

	// Cache Buster
	APPEND_QUERY("z",QString::number(qrand()));

#if QT_VERSION >= 0x050000
	QUrlQuery urlQuery;
	urlQuery.setQueryDelimiters('=','&');
	urlQuery.setQueryItems(query);
	url.setQuery(urlQuery);
#endif
	return url;
}

QString Statistics::getStatisticsFilePath(const QString &AFileName) const
{
	QDir dir(FPluginManager->homePath());
	if (!dir.exists(DIR_STATISTICS))
		dir.mkdir(DIR_STATISTICS);
	dir.cd(DIR_STATISTICS);

	if (!FProfileId.isNull())
	{
		QString profileDir = FProfileId.toString();
		if (!dir.exists(profileDir))
			dir.mkdir(profileDir);
		dir.cd(profileDir);
	}

	return dir.absoluteFilePath(AFileName);
}

IStatisticsHit Statistics::makeViewHit() const
{
	IStatisticsHit hit;
	hit.type = IStatisticsHit::HitView;
	hit.screen = staticMetaObject.className();
	return hit;
}

IStatisticsHit Statistics::makeEventHit(const QString &AParams, int AValue) const
{
	QStringList params = QString(AParams).split("|");

	IStatisticsHit hit;
	hit.type = IStatisticsHit::HitEvent;
	hit.screen = staticMetaObject.className();
	hit.event.category = params.value(0);
	hit.event.action = params.value(0)+"-"+params.value(1);
	hit.event.label = params.value(2);
	hit.event.value = AValue;
	return hit;
}

IStatisticsHit Statistics::makeSessionEvent(const QString &AParams, int ASession) const
{
	IStatisticsHit hit = makeEventHit(AParams);
	hit.session = ASession;
	return hit;
}

void Statistics::sendServerInfoHit(const QString &AName, const QString &AVersion)
{
	if (!AName.isEmpty())
	{
		IStatisticsHit hit = makeEventHit(SEVP_STATISTICS_SERVERS);

		hit.dimensions[SCDP_SERVER_NAME] = AName;
		if (!AVersion.isEmpty())
			hit.dimensions[SCDP_SERVER_VERSION] = AVersion;
		else
			hit.dimensions[SCDP_SERVER_VERSION] = "Unknown";

		sendStatisticsHit(hit);
	}
}

void Statistics::onPendingHitsTimerTimeout()
{
	bool sent = false;
	while (!FPendingHits.isEmpty() && !sent)
	{
		IStatisticsHit hit = FPendingHits.takeFirst();
		sent = sendStatisticsHit(hit);
	}
}

void Statistics::onNetworkManagerFinished(QNetworkReply *AReply)
{
	AReply->deleteLater();
	if (FReplyHits.contains(AReply))
	{
		IStatisticsHit hit = FReplyHits.take(AReply);
		if (AReply->error() != QNetworkReply::NoError)
		{
			hit.profile = FProfileId;
			FPendingHits.append(hit);
			FPendingTimer.start(RESEND_TIMEOUT);
			LOG_WARNING(QString("Failed to send statistics hit, type=%1, screen=%2: %3").arg(hit.type).arg(hit.screen).arg(AReply->errorString()));
		}
		else
		{
			FPendingTimer.start(0);
			LOG_DEBUG(QString("Statistics hit sent, type=%1, screen=%2: %3").arg(hit.type).arg(hit.screen).arg(AReply->request().url().toString()));
		}
		FPluginManager->continueShutdown();
	}
}

void Statistics::onNetworkManagerSSLErrors(QNetworkReply *AReply, const QList<QSslError> &AErrors)
{
	LOG_WARNING(QString("Statistics connection SSL error: %1").arg(AErrors.value(0).errorString()));
	AReply->ignoreSslErrors();
}

void Statistics::onNetworkManagerProxyAuthenticationRequired(const QNetworkProxy &AProxy, QAuthenticator *AAuth)
{
	AAuth->setUser(AProxy.user());
	AAuth->setPassword(AProxy.password());
}

void Statistics::onDefaultConnectionProxyChanged(const QUuid &AProxyId)
{
	FNetworkManager->setProxy(FConnectionManager->proxyById(AProxyId).proxy);
}

void Statistics::onOptionsOpened()
{
	FSendHits = Options::node(OPV_COMMON_STATISTICTS_ENABLED).value().toBool();

	FProfileId = Options::node(OPV_STATISTICS_PROFILEID).value().toString();
	if (FProfileId.isNull())
	{
		FProfileId = QUuid::createUuid();
		Options::node(OPV_STATISTICS_PROFILEID).setValue(FProfileId.toString());
	}

	if (FNetworkManager->cookieJar() != NULL)
		FNetworkManager->cookieJar()->deleteLater();
	FNetworkManager->setCookieJar(new FileCookieJar(getStatisticsFilePath(FILE_COOKIES)));

	sendStatisticsHit(makeSessionEvent(SEVP_SESSION_STARTED,IStatisticsHit::SessionStart));
	FSessionTimer.start();
}

void Statistics::onOptionsClosed()
{
	sendStatisticsHit(makeSessionEvent(SEVP_SESSION_FINISHED,IStatisticsHit::SessionEnd));
	FSessionTimer.stop();
}

void Statistics::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_COMMON_STATISTICTS_ENABLED)
	{
		if (ANode.value().toBool())
		{
			FSendHits = true;
			sendStatisticsHit(makeEventHit(SEVP_STATISTICS_ENABLED));
		}
		else
		{
			sendStatisticsHit(makeEventHit(SEVP_STATISTICS_DISABLED));
			FSendHits = false;
		}
	}
}

void Statistics::onSessionTimerTimeout()
{
	IStatisticsHit hit = makeEventHit(SEVP_STATISTICS_METRICS);

	QList<Jid> streams;
	if (FAccountManager)
	{
		foreach(IAccount *account, FAccountManager->accounts())
		{
			if (account->isActive())
				streams.append(account->xmppStream()->streamJid());
		}
		hit.metrics[SCMP_ACCOUNTS_COUNT] = streams.count();
	}

	if (FRosterManager)
	{
		int agentsCount = 0;
		int contactsCount = 0;
		QSet<QString> groups;
		foreach(IRoster *roster, FRosterManager->rosters())
		{
			foreach(const IRosterItem &ritem, roster->items())
			{
				if (!ritem.itemJid.hasNode())
					agentsCount++;
				else
					contactsCount++;
			}
			groups += roster->groups();
		}
		hit.metrics[SCMP_CONTACTS_COUNT] = contactsCount;
		hit.metrics[SCMP_AGENTS_COUNT] = agentsCount;
		hit.metrics[SCMP_GROUPS_COUNT] = groups.count();
	}

	if (FMessageWidgets)
		hit.metrics[SCMP_CHATWINDOWS_COUNT] = FMessageWidgets->chatWindows().count();

	if (FMultiChatManager)
		hit.metrics[SCMP_MULTICHATWINDOWS_COUNT] = FMultiChatManager->multiChatWindows().count();

	if (FBookmarks)
	{
		foreach(const Jid &streamJid, streams)
			hit.metrics[SCMP_BOOKMARKS_COUNT] += FBookmarks->bookmarks(streamJid).count();
	}

	if (FStatusChanger)
	{
		foreach(int statusId, FStatusChanger->statusItems())
		{
			if (statusId > STATUS_NULL_ID)
				hit.metrics[SCMP_STATUSITEMS_COUNT] += 1;
		}
	}

	sendStatisticsHit(hit);
}

void Statistics::onXmppStreamOpened(IXmppStream *AXmppStream)
{
	if (FClientInfo)
	{
		if (FClientInfo->requestSoftwareInfo(AXmppStream->streamJid(),AXmppStream->streamJid().domain()))
			FSoftwareRequests.insert(AXmppStream->streamJid().domain(),AXmppStream->streamJid());
	}
}

void Statistics::onSoftwareInfoChanged(const Jid &AContactJid)
{
	if (FSoftwareRequests.contains(AContactJid))
	{
		Jid streamJid = FSoftwareRequests.take(AContactJid);
		if (FClientInfo->hasSoftwareInfo(AContactJid))
		{
			sendServerInfoHit(FClientInfo->softwareName(AContactJid),FClientInfo->softwareVersion(AContactJid));
		}
		else if (FDiscovery && FDiscovery->hasDiscoInfo(streamJid,AContactJid))
		{
			IDiscoInfo info = FDiscovery->discoInfo(streamJid,AContactJid);
			int index = FDiscovery->findIdentity(info.identity,"server","im");
			sendServerInfoHit(index>=0 ? info.identity.value(index).name : QString(), QString());
		}
	}
}

void Statistics::onLoggerViewReported(const QString &AClass)
{
	if (!AClass.isEmpty())
	{
		IStatisticsHit hit;
		hit.type = IStatisticsHit::HitView;
		hit.screen = AClass;
		sendStatisticsHit(hit);
	}
}

void Statistics::onLoggerErrorReported(const QString &AClass, const QString &AMessage, bool AFatal)
{
	if (!AClass.isEmpty() && !AMessage.isEmpty() && !FReportedErrors.contains(AClass,AMessage))
	{
		IStatisticsHit hit;
		hit.type = IStatisticsHit::HitException;
		hit.screen = AClass;
		hit.exception.fatal = AFatal;
		hit.exception.descr = AMessage;
		sendStatisticsHit(hit);
		FReportedErrors.insertMulti(AClass,AMessage);
	}
}

void Statistics::onLoggerEventReported(const QString &AClass, const QString &ACategory, const QString &AAction, const QString &ALabel, qint64 AValue)
{
	if (!ACategory.isEmpty() && !AAction.isEmpty())
	{
		IStatisticsHit hit;
		hit.type = IStatisticsHit::HitEvent;
		hit.screen = AClass;
		hit.event.category = ACategory;
		hit.event.action = AAction;
		hit.event.label = ALabel;
		hit.event.value = AValue;
		sendStatisticsHit(hit);
	}
}

void Statistics::onLoggerTimingReported(const QString &AClass, const QString &ACategory, const QString &AVariable, const QString &ALabel, qint64 ATime)
{
	if (!ACategory.isEmpty() && !AVariable.isEmpty() && ATime>=0)
	{
		IStatisticsHit hit;
		hit.type = IStatisticsHit::HitTiming;
		hit.screen = AClass;
		hit.timing.category = ACategory;
		hit.timing.variable = AVariable;
		hit.timing.label = ALabel;
		hit.timing.time = ATime;
		sendStatisticsHit(hit);
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_statistics, Statistics)
#endif

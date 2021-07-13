#include "xmppuriqueries.h"

#include <QPair>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <definitions/messageviewurlhandlerorders.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

XmppUriQueries::XmppUriQueries()
{
	FMessageWidgets = NULL;
	FMapMessage = NULL; // *** <<< eyeCU >>> ***
}

XmppUriQueries::~XmppUriQueries()
{

}

void XmppUriQueries::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("XMPP URI Queries");
	APluginInfo->description = tr("Allows other plugins to handle XMPP URI queries");
	APluginInfo ->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool XmppUriQueries::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
	}
// *** <<< eyeCU <<< ***
	plugin = APluginManager->pluginInterface("IMapMessage").value(0,NULL);
	if (plugin)
	{
		FMapMessage = qobject_cast<IMapMessage *>(plugin->instance());
	}
// *** >>> eyeCU >>> ***
	return true;
}

bool XmppUriQueries::initObjects()
{
	if (FMessageWidgets)
		FMessageWidgets->insertViewUrlHandler(MVUHO_XMPPURIQUERIES, this);
// *** <<< eyeCU <<< ***
	if (FMapMessage)
		FMapMessage->insertUrlHandler(MVUHO_XMPPURIQUERIES, this);
// *** >>> eyeCU >>> ***
	return true;
}

bool XmppUriQueries::messageViewUrlOpen(int AOrder, IMessageViewWidget *AWidget, const QUrl &AUrl)
{
	if (AOrder == MVUHO_XMPPURIQUERIES)
		return openXmppUri(AWidget->messageWindow()->streamJid(), AUrl);
	return false;
}

bool XmppUriQueries::openXmppUri(const Jid &AStreamJid, const QUrl &AUrl) const
{
	Jid contactJid;
	QString action;
	QMultiMap<QString, QString> params;
	if (parseXmppUri(AUrl,contactJid,action,params))
	{
		LOG_STRM_INFO(AStreamJid,QString("Opening XMPP URI, url=%1").arg(AUrl.toString()));
		foreach (IXmppUriHandler *handler, FHandlers)
		{
			if (handler->xmppUriOpen(AStreamJid, contactJid, action, params))
				return true;
		}
		LOG_STRM_WARNING(AStreamJid,QString("Failed to open XMPP URI, url=%1").arg(AUrl.toString()));
	}
	return false;
}

bool XmppUriQueries::parseXmppUri(const QUrl &AUrl, Jid &AContactJid, QString &AAction, QMultiMap<QString, QString> &AParams) const
{
	if (AUrl.isValid() && AUrl.scheme()==XMPP_URI_SCHEME)
	{
		QUrl url =  QUrl::fromEncoded(AUrl.toEncoded().replace(';','&'), QUrl::StrictMode);
		QList< QPair<QString, QString> > keyValues = URL_QUERY_ITEMS(url);

		if (!keyValues.isEmpty())
		{
			AContactJid = url.path();
			AAction = keyValues.takeAt(0).first;
			if (AContactJid.isValid() && !AAction.isEmpty())
			{
				for (int i=0; i<keyValues.count(); i++)
					AParams.insertMulti(keyValues.at(i).first, keyValues.at(i).second);
				return true;
			}
		}
	}
	return false;
}

QString XmppUriQueries::makeXmppUri(const Jid &AContactJid, const QString &AAction, const QMultiMap<QString, QString> &AParams) const
{
	if (AContactJid.isValid() && !AAction.isEmpty())
	{
		QUrl url;
		URL_SET_QUERY_DELIMITERS(url,'=',';');

		url.setScheme(XMPP_URI_SCHEME);
		url.setPath(AContactJid.full());

		QList< QPair<QString, QString> > queryItems;
		queryItems.append(qMakePair<QString,QString>(AAction,QString()));
		for(QMultiMap<QString, QString>::const_iterator it=AParams.constBegin(); it!=AParams.end(); ++it)
            queryItems.append(qMakePair<QString,QString>(it.key(),it.value()));
        URL_SET_QUERY_ITEMS(url,queryItems);

		return url.toString().replace(QString("?%1=;").arg(AAction),QString("?%1;").arg(AAction));
	}
	return QString();
}

void XmppUriQueries::insertUriHandler(int AOrder, IXmppUriHandler *AHandler)
{
	if (!FHandlers.contains(AOrder, AHandler))
	{
		LOG_DEBUG(QString("URI handler inserted, order=%1, address=%2").arg(AOrder).arg((quint64)AHandler));
		FHandlers.insertMulti(AOrder, AHandler);
		emit uriHandlerInserted(AOrder, AHandler);
	}
}

void XmppUriQueries::removeUriHandler(int AOrder, IXmppUriHandler *AHandler)
{
	if (FHandlers.contains(AOrder, AHandler))
	{
		LOG_DEBUG(QString("URI handler removed, order=%1, address=%2").arg(AOrder).arg((quint64)AHandler));
		FHandlers.remove(AOrder, AHandler);
		emit uriHandlerRemoved(AOrder, AHandler);
	}
}
// *** <<< eyeCU <<< ***
bool XmppUriQueries::bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid)
{
	Q_UNUSED(AContactJid)

	if (AOrder == MVUHO_XMPPURIQUERIES)
		return openXmppUri(AStreamJid, AUrl);
	return false;
}
// *** >>> eyeCU >>> ***
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_xmppuriqueries, XmppUriQueries)
#endif

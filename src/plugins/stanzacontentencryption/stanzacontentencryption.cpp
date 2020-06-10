#include "stanzacontentencryption.h"

#include <definitions/namespaces.h>
#include <utils/datetime.h>

#define TAG_NAME_BODY "body"

static QString getRandomString(int ALength)
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-!%^$#@*/_.,;?'`~");

   QString randomString;
   for(int i=0; i<ALength; ++i)
   {
	   int index = qrand() % possibleCharacters.length();
	   QChar nextChar = possibleCharacters.at(index);
	   randomString.append(nextChar);
   }
   return randomString;
}

StanzaContentEncrytion::StanzaContentEncrytion()
{}

StanzaContentEncrytion::~StanzaContentEncrytion()
{}

void StanzaContentEncrytion::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Stanza Content Encryption");
	APluginInfo->description = tr("Encrypts appropriate stanza elements");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
}

bool StanzaContentEncrytion::initConnections(IPluginManager *APluginManager, int & /*AInitOrder*/)
{
	return true;
}

bool StanzaContentEncrytion::initSettings()
{
	return true;
}

bool StanzaContentEncrytion::initObjects()
{
	return true;
}

bool StanzaContentEncrytion::addAcceptableElement(const QString &ANamespace, const QString &ATagName)
{
	if (FAcceptableElements.contains(ANamespace, ATagName))
		return false;
	else
		FAcceptableElements.insert(ANamespace, ATagName);
	return true;
}

bool StanzaContentEncrytion::removeAcceptableElement(const QString &ANamespace, const QString &ATagName)
{
	if (FAcceptableElements.contains(ANamespace, ATagName))
	{
		FAcceptableElements.remove(ANamespace, ATagName);
		return true;
	}
	return false;
}

bool StanzaContentEncrytion::isElementAcceptable(const QString &ANamespace, const QString &ATagName) const
{
	return FAcceptableElements.contains(ANamespace, ATagName);
}

bool StanzaContentEncrytion::isStanzaAcceptable(const Stanza &AStanza) const
{
	for (QDomElement e=AStanza.firstElement(); !e.isNull(); e=e.nextSiblingElement())
		if (FAcceptableElements.contains(e.namespaceURI(), e.tagName()))
			return true;
	return false;
}

QByteArray StanzaContentEncrytion::getContentToEncrypt(const Stanza &AStanza, const QString &AFallbackBodyText) const
{
	QDomDocument d;
	QDomElement payload = d.createElement("payload");
	for (QDomElement e=AStanza.firstElement(); !e.isNull(); e=e.nextSiblingElement())
	{
		if (FAcceptableElements.contains(e.namespaceURI(), e.tagName()))
		{
			payload.appendChild(d.importNode(e, true));
			if (e.tagName()==TAG_NAME_BODY && e.namespaceURI()==NS_JABBER_CLIENT)
			{
				QDomNode text = e.firstChild();				
				if (text.isText())
					text.toText().setData(AFallbackBodyText);
				else
					qWarning("Invalid <body/> child node: non-text!");
			}
			else
				AStanza.element().removeChild(e);
		}
	}

	if (payload.hasChildNodes())
	{
		QDomElement content = d.createElementNS(NS_SCE, "content");
		content.appendChild(payload);
		d.appendChild(content);
		if (!AStanza.from().isEmpty())
		{
			QDomElement from = d.createElement("from");
			content.appendChild(from);
			from.setAttribute("jid", AStanza.from());
		}
		if (!AStanza.to().isEmpty())
		{
			QDomElement to = d.createElement("from");
			content.appendChild(to);
			to.setAttribute("jid", AStanza.to());
		}
		QDomElement rpad = d.createElement("rpad");
		content.appendChild(rpad);
		rpad.appendChild(d.createTextNode(getRandomString(qrand()*200/RAND_MAX)));
		QDomElement time = d.createElement("time");
		content.appendChild(time);
		time.setAttribute("stamp", DateTime(QDateTime::currentDateTime()).toX85UTC());
		return d.toByteArray();
	}
	else
	{
		qCritical("No acceptable message elements! Returning NULL data.");
		return QByteArray();
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_stanzacontentencryption, StanzaContentEncrytion)
#endif

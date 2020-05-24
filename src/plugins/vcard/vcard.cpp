#include "vcard.h"
#include "vcardmanager.h"

#include <QFile>
#include <QBuffer>
#include <QImageReader>
#include <definitions/namespaces.h>
#include <utils/logger.h>

#define DEFAUL_IMAGE_FORMAT       "png"

VCard::VCard(VCardManager *APlugin, const Jid &AContactJid) : QObject(APlugin)
{
	FContactJid = AContactJid;
	FVCardPlugin = APlugin;
	connect(FVCardPlugin,SIGNAL(vcardReceived(const Jid &)),SLOT(onVCardReceived(const Jid &)));
	connect(FVCardPlugin,SIGNAL(vcardPublished(const Jid &)),SLOT(onVCardPublished(const Jid &)));
	connect(FVCardPlugin,SIGNAL(vcardError(const Jid &, const XmppError &)),SLOT(onVCardError(const Jid &, const XmppError &)));
	loadVCardFile();
}

VCard::~VCard()
{

}

bool VCard::isValid() const
{
	return FContactJid.isValid() && !vcardElem().isNull();
}

bool VCard::isEmpty() const
{
	return !isValid() || !vcardElem().hasChildNodes();
}

Jid VCard::contactJid() const
{
	return FContactJid;
}

QDomElement VCard::vcardElem() const
{
	return FDoc.documentElement().firstChildElement(VCARD_TAGNAME);
}

QDateTime VCard::loadDateTime() const
{
	return FLoadDateTime;
}

QMultiHash<QString,QStringList> VCard::values(const QString &AName, const QStringList &ATagList) const
{
	QMultiHash<QString,QStringList> result;
	QDomElement elem = firstElementByName(AName);
	while (!elem.isNull())
	{
		if (!elem.text().isEmpty())
		{
			QStringList tags;
			QDomElement parentElem = elem.parentNode().toElement();
			foreach(const QString &tag, ATagList)
				if (!parentElem.firstChildElement(tag).isNull())
					tags.append(tag);
			result.insertMulti(elem.text(),tags);
		}
		elem = nextElementByName(AName, elem);
	}
	return result;
}

QString VCard::value(const QString &AName, const QStringList &ATags, const QStringList &ATagList) const
{
	bool tagsFailed = true;
	QDomElement elem = firstElementByName(AName);
	while (!elem.isNull() && tagsFailed)
	{
		tagsFailed = false;
		QDomElement parentElem = elem.parentNode().toElement();
		foreach(const QString &tag, ATagList)
		{
			QDomElement tagElem = parentElem.firstChildElement(tag);
			if ((tagElem.isNull() && ATags.contains(tag)) || (!tagElem.isNull() && !ATags.contains(tag)))
			{
				tagsFailed = true;
				elem = nextElementByName(AName,elem);
				break;
			}
		}
	}
	return elem.text();
}

void VCard::setTagsForValue(const QString &AName, const QString &AValue, const QStringList &ATags, const QStringList &ATagList)
{
	QDomElement elem = firstElementByName(AName);
	while (!elem.isNull() && elem.text()!=AValue)
		elem = nextElementByName(AName,elem);

	if (elem.isNull())
	{
		elem = createElementByName(AName,ATags,ATagList);
		setTextToElem(elem,AValue);
	}

	if (!ATags.isEmpty() || !ATagList.isEmpty())
	{
		elem = elem.parentNode().toElement();
		foreach(const QString &tag, ATags)
			if (elem.firstChildElement(tag).isNull())
				elem.appendChild(FDoc.createElement(tag));

		elem = elem.firstChildElement();
		while (!elem.isNull())
		{
			QDomElement nextElem = elem.nextSiblingElement();
			if (ATagList.contains(elem.tagName()) && !ATags.contains(elem.tagName()))
				elem.parentNode().removeChild(elem);
			elem = nextElem;
		}
	}
}

void VCard::setValueForTags(const QString &AName, const QString &AValue, const QStringList &ATags, const QStringList &ATagList)
{
	bool tagsFaild = true;
	QDomElement elem = firstElementByName(AName);
	while (!elem.isNull() && tagsFaild)
	{
		tagsFaild = false;
		QDomElement parentElem = elem.parentNode().toElement();
		foreach(const QString &tag, ATagList)
		{
			QDomElement tagElem = parentElem.firstChildElement(tag);
			if ((tagElem.isNull() && ATags.contains(tag)) || (!tagElem.isNull() && !ATags.contains(tag)))
			{
				tagsFaild = true;
				elem = nextElementByName(AName,elem);
				break;
			}
		}
	}

	if (elem.isNull())
		elem = createElementByName(AName,ATags,ATagList);
	setTextToElem(elem,AValue);

	if (!ATags.isEmpty())
	{
		elem = elem.parentNode().toElement();
		foreach(const QString &tag, ATags)
			if (elem.firstChildElement(tag).isNull())
				elem.appendChild(FDoc.createElement(tag));
	}
}

void VCard::clear()
{
	FDoc.documentElement().removeChild(FDoc.documentElement().firstChildElement(VCARD_TAGNAME));
	FDoc.documentElement().appendChild(FDoc.createElementNS(NS_VCARD_TEMP,VCARD_TAGNAME));
}

bool VCard::update(const Jid &AStreamJid)
{
	if (FContactJid.isValid() && AStreamJid.isValid())
	{
		FStreamJid = AStreamJid;
		return FVCardPlugin->requestVCard(AStreamJid,FContactJid);
	}
	return false;
}

bool VCard::publish(const Jid &AStreamJid, const Jid &AContactJid, bool AMuc)
{
	Jid vcardJid = AMuc ? AContactJid : AStreamJid;
	if (isValid() && AStreamJid.isValid())
	{
		FStreamJid = AStreamJid;
		return FVCardPlugin->publishVCard(AStreamJid,vcardJid,this);
	}
	return false;
}

void VCard::unlock()
{
	FVCardPlugin->unlockVCard(FContactJid);
}

void VCard::loadVCardFile()
{
	QFile file(FVCardPlugin->vcardFileName(FContactJid));
	if (file.open(QIODevice::ReadOnly))
	{
		QString xmlError;
		if (!FDoc.setContent(&file,true,&xmlError))
		{
			REPORT_ERROR(QString("Failed to load vCard from file content: %1").arg(xmlError));
			file.remove();
		}
	}
	else if (file.exists())
	{
		REPORT_ERROR(QString("Failed to load vCard from file: %1").arg(file.errorString()));
	}

	if (vcardElem().isNull())
	{
		FDoc.clear();
		QDomElement elem = FDoc.appendChild(FDoc.createElement(VCARD_TAGNAME)).toElement();
		elem.setAttribute("jid",FContactJid.full());
		elem.appendChild(FDoc.createElementNS(NS_VCARD_TEMP,VCARD_TAGNAME));
	}
	else
	{
		FLoadDateTime = QDateTime::fromString(FDoc.documentElement().attribute("dateTime"),Qt::ISODate);
	}

	emit vcardUpdated();
}

QDomElement VCard::createElementByName(const QString &AName, const QStringList &ATags, const QStringList &ATagList)
{
	QStringList tagTree = AName.split('/',QString::SkipEmptyParts);
	QDomElement elem = vcardElem().firstChildElement(tagTree.at(0));

	bool tagsFaild = !ATags.isEmpty() || !ATagList.isEmpty();
	while (!elem.isNull() && tagsFaild)
	{
		tagsFaild = false;
		foreach(const QString &tag, ATagList)
		{
			QDomElement tagElem = elem.firstChildElement(tag);
			if ((tagElem.isNull() && ATags.contains(tag)) || (!tagElem.isNull() && !ATags.contains(tag)))
			{
				tagsFaild = true;
				elem = elem.nextSiblingElement(elem.tagName());
				break;
			}
		}
	}

	if (elem.isNull())
		elem = vcardElem().appendChild(FDoc.createElement(tagTree.at(0))).toElement();

	for (int deep = 1; deep<tagTree.count(); deep++)
		elem = elem.appendChild(FDoc.createElement(tagTree.at(deep))).toElement();

	return elem;
}

QDomElement VCard::firstElementByName(const QString &AName) const
{
	int index = 0;
	QDomElement elem = vcardElem();
	QStringList tagTree = AName.split('/',QString::SkipEmptyParts);
	while (!elem.isNull() && index < tagTree.count())
		elem = elem.firstChildElement(tagTree.at(index++));
	return elem;
}

QDomElement VCard::nextElementByName(const QString &AName, const QDomElement &APrevElem) const
{
	QDomElement elem = APrevElem;
	QStringList tagTree = AName.split('/',QString::SkipEmptyParts);
	int index = tagTree.count();
	while (index > 1)
	{
		index--;
		elem = elem.parentNode().toElement();
	}
	elem = elem.nextSiblingElement(elem.tagName());
	while (!elem.isNull() && index < tagTree.count())
		elem = elem.firstChildElement(tagTree.at(index++));
	return elem;
}

QDomElement VCard::setTextToElem(QDomElement &AElem, const QString &AText) const
{
	if (!AElem.isNull())
	{
		QDomNode node = AElem.firstChild();
		while (!node.isNull() && !node.isText())
			node = node.nextSibling();
		if (node.isNull() && !AText.isEmpty())
			AElem.appendChild(AElem.ownerDocument().createTextNode(AText));
		else if (!node.isNull() && !AText.isNull())
			node.toText().setData(AText);
		else if (!node.isNull())
			AElem.removeChild(node);
	}
	return AElem;
}

void VCard::onVCardReceived(const Jid &AContactJid)
{
	if (FContactJid == AContactJid)
	{
		FStreamJid = Jid::null;
		loadVCardFile();
	}
}

void VCard::onVCardPublished(const Jid &AStreamJid)
{
	if (FStreamJid == AStreamJid)
	{
		FStreamJid = Jid::null;
		emit vcardPublished();
	}
}

void VCard::onVCardError(const Jid &AContactJid, const XmppError &AError)
{
	if (FContactJid == AContactJid)
	{
		FStreamJid = Jid::null;
		emit vcardError(AError);
	}
	else if (FStreamJid == AContactJid)
	{
		FStreamJid = Jid::null;
		emit vcardError(AError);
	}
}

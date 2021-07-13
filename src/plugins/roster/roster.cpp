#include "roster.h"

#include <QSet>
#include <QFile>
#include <definitions/namespaces.h>
#include <definitions/optionvalues.h>
#include <definitions/internalerrors.h>
#include <definitions/stanzahandlerorders.h>
#include <definitions/xmppstanzahandlerorders.h>
#include <utils/xmpperror.h>
#include <utils/options.h>
#include <utils/logger.h>

#define SHC_ROSTER            "/iq[@type='set']/query[@xmlns='" NS_JABBER_ROSTER "']"
#define SHC_PRESENCE          "/presence[@type]"

Roster::Roster(IXmppStream *AXmppStream, IStanzaProcessor *AStanzaProcessor) : QObject(AXmppStream->instance())
{
	FXmppStream = AXmppStream;
	FStanzaProcessor = AStanzaProcessor;

	FOpened = false;
	FVerSupported = false;

	IStanzaHandle itemHandle;
	itemHandle.handler = this;
	itemHandle.order = SHO_DEFAULT;
	itemHandle.direction = IStanzaHandle::DirectionIn;
	itemHandle.streamJid = FXmppStream->streamJid();
	itemHandle.conditions.append(SHC_ROSTER);
	FSHIRosterPush = FStanzaProcessor->insertStanzaHandle(itemHandle);

	IStanzaHandle subscrHandle;
	subscrHandle.handler = this;
	subscrHandle.order = SHO_DEFAULT;
	subscrHandle.direction = IStanzaHandle::DirectionIn;
	subscrHandle.streamJid = FXmppStream->streamJid();
	subscrHandle.conditions.append(SHC_PRESENCE);
	FSHISubscription = FStanzaProcessor->insertStanzaHandle(subscrHandle);

	FXmppStream->insertXmppStanzaHandler(XSHO_XMPP_FEATURE,this);

	connect(FXmppStream->instance(),SIGNAL(opened()),SLOT(onXmppStreamOpened()));
	connect(FXmppStream->instance(),SIGNAL(closed()),SLOT(onXmppStreamClosed()));
	connect(FXmppStream->instance(),SIGNAL(jidAboutToBeChanged(const Jid &)),SLOT(onXmppStreamJidAboutToBeChanged(const Jid &)));
	connect(FXmppStream->instance(),SIGNAL(jidChanged(const Jid &)),SLOT(onXmppStreamJidChanged(const Jid &)));
}

Roster::~Roster()
{
	FStanzaProcessor->removeStanzaHandle(FSHIRosterPush);
	FStanzaProcessor->removeStanzaHandle(FSHISubscription);
	FXmppStream->removeXmppStanzaHandler(XSHO_XMPP_FEATURE,this);

	clearRosterItems();
	emit rosterDestroyed();
}

bool Roster::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
	if (AHandlerId == FSHIRosterPush)
	{
		if (isOpen() && AStanza.isFromServer())
		{
			AAccept = true;
			LOG_STRM_DEBUG(streamJid(),"Roster items push received");
			processItemsElement(AStanza.firstElement("query",NS_JABBER_ROSTER),false);

			Stanza result = FStanzaProcessor->makeReplyResult(AStanza);
			FStanzaProcessor->sendStanzaOut(AStreamJid,result);
		}
		else if (!isOpen())
		{
			REPORT_ERROR("Failed to process roster items push: Roster is closed");
		}
		else if (!AStanza.isFromServer())
		{
			REPORT_ERROR("Failed to process roster items push: Invalid stanza sender");
		}
	}
	else if (AHandlerId == FSHISubscription)
	{
		Jid contactJid = AStanza.from();
		QString status = AStanza.firstElement("status").text();
		if (AStanza.type() == SUBSCRIPTION_SUBSCRIBE)
		{
			AAccept = true;
			FSubscriptionRequests += contactJid.bare();
			LOG_STRM_INFO(streamJid(),QString("Subscribe presence received from=%1, status=%2").arg(contactJid.full(),status));
			emit subscriptionReceived(AStanza.from(),IRoster::Subscribe,status);
		}
		else if (AStanza.type() == SUBSCRIPTION_SUBSCRIBED)
		{
			AAccept = true;
			LOG_STRM_INFO(streamJid(),QString("Subscribed presence received from=%1, status=%2").arg(contactJid.full(),status));
			emit subscriptionReceived(AStanza.from(),IRoster::Subscribed,status);
		}
		else if (AStanza.type() == SUBSCRIPTION_UNSUBSCRIBE)
		{
			AAccept = true;
			FSubscriptionRequests -= contactJid.bare();
			LOG_STRM_INFO(streamJid(),QString("Unsubscribe presence received from=%1, status=%2").arg(contactJid.full(),status));
			emit subscriptionReceived(AStanza.from(),IRoster::Unsubscribe,status);
		}
		else if (AStanza.type() == SUBSCRIPTION_UNSUBSCRIBED)
		{
			AAccept = true;
			LOG_STRM_INFO(streamJid(),QString("Unsubscribed presence received from=%1, status=%2").arg(contactJid.full(),status));
			emit subscriptionReceived(AStanza.from(),IRoster::Unsubscribed,status);
		}
	}
	return false;
}

void Roster::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	if (AStanza.id() == FDelimRequestId)
	{
		FDelimRequestId.clear();
		QString groupDelim = ROSTER_GROUP_DELIMITER;
		if (AStanza.isResult())
		{
			groupDelim = AStanza.firstElement("query",NS_JABBER_PRIVATE).firstChildElement("roster").text();
			if (groupDelim.isEmpty())
			{
				groupDelim = ROSTER_GROUP_DELIMITER;
				LOG_STRM_INFO(streamJid(),QString("Saving default roster group delimiter on server, delimiter='%1'").arg(groupDelim));

				Stanza delim(STANZA_KIND_IQ);
				delim.setType(STANZA_TYPE_SET).setUniqueId();
				QDomElement elem = delim.addElement("query",NS_JABBER_PRIVATE);
				elem.appendChild(delim.createElement("roster",NS_STORAGE_GROUP_DELIMITER)).appendChild(delim.createTextNode(groupDelim));
				FStanzaProcessor->sendStanzaOut(AStreamJid,delim);
			}
			else
			{
				LOG_STRM_INFO(streamJid(),QString("Roster group delimiter loaded, delimiter='%1'").arg(groupDelim));
			}
		}
		else
		{
			LOG_STRM_WARNING(streamJid(),QString("Failed to load roster group delimiter: %1").arg(XmppStanzaError(AStanza).condition()));
		}
		setGroupDelimiter(groupDelim);
		requestRosterItems();
	}
	else if (AStanza.id() == FOpenRequestId)
	{
		FOpenRequestId.clear();
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(streamJid(),"Roster items loaded");
			processItemsElement(AStanza.firstElement("query",NS_JABBER_ROSTER),true);
			FOpened = true;
			emit opened();
		}
		else
		{
			LOG_STRM_WARNING(streamJid(),QString("Failed to load roster items: %1").arg(XmppStanzaError(AStanza).condition()));
			FXmppStream->abort(XmppError(IERR_ROSTER_REQUEST_FAILED));
		}
	}
}

bool Roster::xmppStanzaIn(IXmppStream *AXmppStream, Stanza &AStanza, int AOrder)
{
	if (!FVerSupported && !isOpen() && FXmppStream==AXmppStream && AOrder==XSHO_XMPP_FEATURE)
	{
		if (AStanza.namespaceURI()==NS_JABBER_STREAMS && AStanza.kind()=="features" && !AStanza.firstElement("ver",NS_FEATURE_ROSTER_VER).isNull())
		{
			FVerSupported = true;
			LOG_STRM_INFO(streamJid(),"Roster versioning is supported by server");
		}
	}
	return false;
}

bool Roster::xmppStanzaOut(IXmppStream *AXmppStream, Stanza &AStanza, int AOrder)
{
	Q_UNUSED(AXmppStream); Q_UNUSED(AStanza); Q_UNUSED(AOrder);
	return false;
}

Jid Roster::streamJid() const
{
	return FXmppStream->streamJid();
}

IXmppStream *Roster::xmppStream() const
{
	return FXmppStream;
}

bool Roster::isOpen() const
{
	return FOpened;
}

QString Roster::groupDelimiter() const
{
	return FGroupDelim;
}

QList<Jid> Roster::itemsJid() const
{
	return FItems.keys();
}

QList<IRosterItem> Roster::items() const
{
	return FItems.values();
}

bool Roster::hasItem(const Jid &AItemJid) const
{
	return FItems.contains(AItemJid.bare());
}

IRosterItem Roster::findItem(const Jid &AItemJid) const
{
	return FItems.value(AItemJid.bare());
}

QSet<QString> Roster::groups() const
{
	QSet<QString> rgroups;
	foreach(const IRosterItem &ritem, FItems)
		rgroups += ritem.groups;
	return rgroups;
}

bool Roster::hasGroup(const QString &AGroup) const
{
	foreach(const IRosterItem &ritem, FItems)
	{
		foreach(const QString &group, ritem.groups)
		{
			if (isSubgroup(AGroup,group))
				return true;
		}
	}
	return false;
}

QList<IRosterItem> Roster::groupItems(const QString &AGroup) const
{
	QList<IRosterItem> ritems;
	foreach(const IRosterItem &ritem, FItems)
	{
		foreach(const QString &group, ritem.groups)
		{
			if (isSubgroup(AGroup,group))
			{
				ritems.append(ritem);
				break;
			}
		}
	}
	return ritems;
}

QSet<QString> Roster::itemGroups(const Jid &AItemJid) const
{
	return findItem(AItemJid).groups;
}

bool Roster::isSubgroup(const QString &ASubGroup, const QString &AGroup) const
{
	return AGroup==ASubGroup || AGroup.startsWith(ASubGroup+ROSTER_GROUP_DELIMITER);
}

void Roster::setItem(const Jid &AItemJid, const QString &AName, const QSet<QString> &AGroups)
{
	if (isOpen())
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_SET).setUniqueId();

		QDomElement itemElem = request.addElement("query",NS_JABBER_ROSTER).appendChild(request.createElement("item")).toElement();
		if (!AName.isEmpty())
			itemElem.setAttribute("name",AName);
		itemElem.setAttribute("jid", AItemJid.bare());

		foreach (QString group, AGroups)
		{
			group = replaceGroupDelimiter(group,ROSTER_GROUP_DELIMITER,FGroupDelim);
			if (!group.isEmpty())
				itemElem.appendChild(request.createElement("group")).appendChild(request.createTextNode(group));
		}

		if (FStanzaProcessor->sendStanzaOut(FXmppStream->streamJid(),request))
			LOG_STRM_INFO(streamJid(),QString("Roster item update request sent, jid=%1, name=%2, groups=%3").arg(AItemJid.bare(),AName,QStringList(AGroups.toList()).join("; ")));
		else
			LOG_STRM_WARNING(streamJid(),QString("Failed to send roster item update request, jid=%1").arg(AItemJid.bare()));
	}
	else
	{
		LOG_STRM_ERROR(streamJid(),QString("Failed to send roster item update request, jid=%1: Roster is not opened").arg(AItemJid.bare()));
	}
}

void Roster::setItems(const QList<IRosterItem> &AItems)
{
	if (isOpen() && !AItems.isEmpty())
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_SET).setUniqueId();

		QDomElement elem = request.addElement("query",NS_JABBER_ROSTER);
		foreach(const IRosterItem &ritem, AItems)
		{
			QDomElement itemElem = elem.appendChild(request.createElement("item")).toElement();
			if (!ritem.name.isEmpty())
				itemElem.setAttribute("name",ritem.name);
			itemElem.setAttribute("jid", ritem.itemJid.bare());

			foreach(QString group, ritem.groups)
			{
				group = replaceGroupDelimiter(group,ROSTER_GROUP_DELIMITER,FGroupDelim);
				if (!group.isEmpty())
					itemElem.appendChild(request.createElement("group")).appendChild(request.createTextNode(group));
			}
		}

		if (FStanzaProcessor->sendStanzaOut(FXmppStream->streamJid(),request))
			LOG_STRM_INFO(streamJid(),QString("Roster items update request sent, count=%1").arg(AItems.count()));
		else
			LOG_STRM_WARNING(streamJid(),"Failed to send roster items update request");
	}
	else if (!isOpen())
	{
		LOG_STRM_ERROR(streamJid(),"Failed to send roster items update request: Roster is not opened");
	}
}

QSet<Jid> Roster::subscriptionRequests() const
{
	return FSubscriptionRequests;
}

void Roster::sendSubscription(const Jid &AItemJid, int ASubsType, const QString &AText)
{
	if (isOpen())
	{
		QString type;
		if (ASubsType == IRoster::Subscribe)
			type = SUBSCRIPTION_SUBSCRIBE;
		else if (ASubsType == IRoster::Subscribed)
			type = SUBSCRIPTION_SUBSCRIBED;
		else if (ASubsType == IRoster::Unsubscribe)
			type = SUBSCRIPTION_UNSUBSCRIBE;
		else if (ASubsType == IRoster::Unsubscribed)
			type = SUBSCRIPTION_UNSUBSCRIBED;

		if (!type.isEmpty())
		{
			Stanza request(STANZA_KIND_PRESENCE);
			request.setTo(AItemJid.bare()).setType(type);
			if (!AText.isEmpty())
				request.addElement("status").appendChild(request.createTextNode(AText));
			if (FStanzaProcessor->sendStanzaOut(FXmppStream->streamJid(),request))
			{
				LOG_STRM_INFO(streamJid(),QString("Subscription request sent to=%1, type=%2").arg(AItemJid.bare(),type));
				if (ASubsType==IRoster::Subscribed || ASubsType==IRoster::Unsubscribed)
					FSubscriptionRequests -= AItemJid.bare();
				emit subscriptionSent(AItemJid.bare(),ASubsType,AText);
			}
			else
			{
				LOG_STRM_WARNING(streamJid(),QString("Failed to send subscription request to=%1, type=%2").arg(AItemJid.bare(),type));
			}
		}
		else
		{
			LOG_STRM_ERROR(streamJid(),QString("Failed to send subscription request to=%1, type=%2: Invalid subscription type").arg(AItemJid.bare()).arg(ASubsType));
		}
	}
	else
	{
		LOG_STRM_ERROR(streamJid(),QString("Failed to send subscription request to=%1, type=%2: Roster is not opened").arg(AItemJid.bare()).arg(ASubsType));
	}
}

void Roster::removeItem(const Jid &AItemJid)
{
	if (isOpen())
	{
		Stanza query(STANZA_KIND_IQ);
		query.setType(STANZA_TYPE_SET).setUniqueId();
		QDomElement itemElem = query.addElement("query",NS_JABBER_ROSTER).appendChild(query.createElement("item")).toElement();
		itemElem.setAttribute("jid", AItemJid.bare());
		itemElem.setAttribute("subscription",SUBSCRIPTION_REMOVE);
		if (FStanzaProcessor->sendStanzaOut(FXmppStream->streamJid(),query))
			LOG_STRM_INFO(streamJid(),QString("Roster item remove request sent, jid=%1").arg(AItemJid.bare()));
		else
			LOG_STRM_WARNING(streamJid(),QString("Failed to send roster item remove request, jid=%1").arg(AItemJid.bare()));
	}
	else
	{
		LOG_STRM_ERROR(streamJid(),QString("Failed to send roster item remove request, jid=%1: Roster is not opened").arg(AItemJid.bare()));
	}
}

void Roster::removeItems(const QList<IRosterItem> &AItems)
{
	if (isOpen() && !AItems.isEmpty())
	{
		Stanza query(STANZA_KIND_IQ);
		query.setType(STANZA_TYPE_SET).setUniqueId();
		QDomElement elem = query.addElement("query",NS_JABBER_ROSTER);
		foreach(const IRosterItem &ritem, AItems)
		{
			QDomElement itemElem = elem.appendChild(query.createElement("item")).toElement();
			itemElem.setAttribute("jid", ritem.itemJid.bare());
			itemElem.setAttribute("subscription",SUBSCRIPTION_REMOVE);
		}
		if (FStanzaProcessor->sendStanzaOut(FXmppStream->streamJid(),query))
			LOG_STRM_INFO(streamJid(),QString("Roster items remove request sent, count=%1").arg(AItems.count()));
		else
			LOG_STRM_WARNING(streamJid(),QString("Failed to send roster items remove request, count=%1").arg(AItems.count()));
	}
	else if (!isOpen())
	{
		LOG_STRM_ERROR(streamJid(),QString("Failed to send roster items remove request, count=%1: Roster is not opened").arg(AItems.count()));
	}
}

void Roster::saveRosterItems(const QString &AFileName) const
{
	QDomDocument xml;
	QDomElement elem = xml.appendChild(xml.createElement("roster")).toElement();
	elem.setAttribute("ver",FRosterVer);
	elem.setAttribute("streamJid",streamJid().pBare());
	elem.setAttribute("groupDelimiter",FGroupDelim);
	foreach(const IRosterItem &ritem, FItems)
	{
		QDomElement itemElem = elem.appendChild(xml.createElement("item")).toElement();
		itemElem.setAttribute("jid",ritem.itemJid.bare());
		itemElem.setAttribute("name",ritem.name);
		itemElem.setAttribute("subscription",ritem.subscription);
		itemElem.setAttribute("ask",ritem.subscriptionAsk);
		foreach(QString group, ritem.groups)
		{
			group = replaceGroupDelimiter(group,ROSTER_GROUP_DELIMITER,FGroupDelim);
			if (!group.isEmpty())
				itemElem.appendChild(xml.createElement("group")).appendChild(xml.createTextNode(group));
		}
	}

	QFile file(AFileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		LOG_STRM_INFO(streamJid(),QString("Roster items saved to file=%1").arg(AFileName));
		file.write(xml.toByteArray());
		file.close();
	}
	else
	{
		REPORT_ERROR(QString("Failed to save roster items to file: %1").arg(file.errorString()));
	}
}

void Roster::loadRosterItems(const QString &AFileName)
{
	if (!isOpen())
	{
		QFile file(AFileName);
		if (file.open(QIODevice::ReadOnly))
		{
			QString xmlError;
			QDomDocument doc;
			if (doc.setContent(&file,true,&xmlError))
			{
				QDomElement itemsElem = doc.firstChildElement("roster");
				if (!itemsElem.isNull() && itemsElem.attribute("streamJid")==streamJid().pBare())
				{
					LOG_STRM_INFO(streamJid(),QString("Roster items loaded from file=%1").arg(AFileName));
					setGroupDelimiter(itemsElem.attribute("groupDelimiter"));
					processItemsElement(itemsElem,true);
				}
				else if (!itemsElem.isNull())
				{
					REPORT_ERROR("Failed to load roster items from file content: Invalid stream JID");
					file.remove();
				}
			}
			else
			{
				REPORT_ERROR(QString("Failed to load roster items from file content: %1").arg(xmlError));
				file.remove();
			}
		}
		else if (file.exists())
		{
			REPORT_ERROR(QString("Failed to load roster items from file: %1").arg(file.errorString()));
		}
	}
	else
	{
		LOG_STRM_ERROR(streamJid(),QString("Failed to load roster items from file=%1: Roster is opened").arg(AFileName));
	}
}

void Roster::renameItem(const Jid &AItemJid, const QString &AName)
{
	IRosterItem ritem = findItem(AItemJid);
	if (!ritem.isNull() && ritem.name!=AName)
	{
		LOG_STRM_INFO(streamJid(),QString("Renaming roster item, jid=%1, name=%2").arg(AItemJid.bare(),AName));
		setItem(AItemJid,AName,ritem.groups);
	}
}

void Roster::copyItemToGroup(const Jid &AItemJid, const QString &AGroupTo)
{
	IRosterItem ritem = findItem(AItemJid);
	if (!ritem.isNull() && !AGroupTo.isEmpty() && !ritem.groups.contains(AGroupTo))
	{
		LOG_STRM_INFO(streamJid(),QString("Coping roster item to group, jid=%1, to_group=%2").arg(AItemJid.bare(),AGroupTo));
		QSet<QString> allItemGroups = ritem.groups;
		setItem(AItemJid,ritem.name,allItemGroups += AGroupTo);
	}
}

void Roster::moveItemToGroup(const Jid &AItemJid, const QString &AGroupFrom, const QString &AGroupTo)
{
	IRosterItem ritem = findItem(AItemJid);
	if (!ritem.isNull() && !ritem.groups.contains(AGroupTo))
	{
		LOG_STRM_INFO(streamJid(),QString("Moving roster item to group, jid=%1, from_group=%2, to_group=%3").arg(AItemJid.bare(),AGroupFrom,AGroupTo));
		QSet<QString> allItemGroups = ritem.groups;
		if (!AGroupTo.isEmpty())
		{
			allItemGroups += AGroupTo;
			allItemGroups -= AGroupFrom;
		}
		else
		{
			allItemGroups.clear();
		}
		setItem(AItemJid,ritem.name,allItemGroups);
	}
}

void Roster::removeItemFromGroup(const Jid &AItemJid, const QString &AGroupFrom)
{
	IRosterItem ritem = findItem(AItemJid);
	if (!ritem.isNull() && ritem.groups.contains(AGroupFrom))
	{
		LOG_STRM_INFO(streamJid(),QString("Removing roster item from group, jid=%1, from_group=%2").arg(AItemJid.bare(),AGroupFrom));
		QSet<QString> allItemGroups = ritem.groups;
		setItem(AItemJid,ritem.name,allItemGroups -= AGroupFrom);
	}
}

void Roster::renameGroup(const QString &AGroup, const QString &AGroupTo)
{
	if (!AGroup.isEmpty() && !AGroupTo.isEmpty() && AGroup!=AGroupTo)
	{
		LOG_STRM_INFO(streamJid(),QString("Renaming roster group from=%1 to=%2").arg(AGroup,AGroupTo));
		QList<IRosterItem> allGroupItems = groupItems(AGroup);
		for (QList<IRosterItem>::iterator it=allGroupItems.begin(); it!=allGroupItems.end(); ++it)
		{
			QSet<QString> newItemGroups;
			foreach(QString group, it->groups)
			{
				if (isSubgroup(AGroup,group))
				{
					group.remove(0,AGroup.size());
					group.prepend(AGroupTo);
				}
				newItemGroups += group;
			}
			it->groups = newItemGroups;
		}
		setItems(allGroupItems);
	}
}

void Roster::copyGroupToGroup(const QString &AGroup, const QString &AGroupTo)
{
	if (AGroup != AGroupTo)
	{
		LOG_STRM_INFO(streamJid(),QString("Coping roster group=%1 to group=%2").arg(AGroup,AGroupTo));
		QList<IRosterItem> allGroupItems = groupItems(AGroup);
		QString groupName = AGroup.split(ROSTER_GROUP_DELIMITER).last();
		for (QList<IRosterItem>::iterator it = allGroupItems.begin(); it != allGroupItems.end(); ++it)
		{
			foreach(QString group, it->groups)
			{
				if (isSubgroup(AGroup,group))
				{
					group.remove(0,AGroup.size());
					if (!AGroupTo.isEmpty())
						group.prepend(AGroupTo + ROSTER_GROUP_DELIMITER + groupName);
					else
						group.prepend(groupName);
					it->groups += group;
				}
			}
		}
		setItems(allGroupItems);
	}
}

void Roster::moveGroupToGroup(const QString &AGroup, const QString &AGroupTo)
{
	if (AGroup != AGroupTo)
	{
		LOG_STRM_INFO(streamJid(),QString("Moving roster group=%1 to group=%2").arg(AGroup,AGroupTo));
		QList<IRosterItem> allGroupItems = groupItems(AGroup);
		QString groupName = AGroup.split(ROSTER_GROUP_DELIMITER).last();
		for (QList<IRosterItem>::iterator it=allGroupItems.begin(); it!=allGroupItems.end(); ++it)
		{
			foreach(QString group, it->groups)
			{
				if (isSubgroup(AGroup,group))
				{
					it->groups -= group;
					group.remove(0,AGroup.size());
					if (!AGroupTo.isEmpty())
						group.prepend(AGroupTo + ROSTER_GROUP_DELIMITER + groupName);
					else
						group.prepend(groupName);
					it->groups += group;
				}
			}
		}
		setItems(allGroupItems);
	}
}

void Roster::removeGroup(const QString &AGroup)
{
	if (!AGroup.isEmpty())
	{
		LOG_STRM_INFO(streamJid(),QString("Removing roster group=%1").arg(AGroup));
		QList<IRosterItem> allGroupItems = groupItems(AGroup);
		for (QList<IRosterItem>::iterator it=allGroupItems.begin(); it!=allGroupItems.end(); ++it)
		{
			foreach(const QString &group, it->groups)
				if (isSubgroup(AGroup,group))
					it->groups -= group;
		}
		setItems(allGroupItems);
	}
}

void Roster::clearRosterItems()
{
	for(QHash<Jid, IRosterItem>::iterator it=FItems.begin(); it!=FItems.end(); it=FItems.erase(it))
	{
		IRosterItem before = it.value();
		it->subscription = SUBSCRIPTION_REMOVE;
		emit itemReceived(it.value(),before);
	}
	FRosterVer.clear();
}

void Roster::requestRosterItems()
{
	Stanza query(STANZA_KIND_IQ);
	query.setType(STANZA_TYPE_GET).setUniqueId();

	if (!FVerSupported)
		query.addElement("query",NS_JABBER_ROSTER);
	else
		query.addElement("query",NS_JABBER_ROSTER).setAttribute("ver",FRosterVer);

	if (FStanzaProcessor->sendStanzaRequest(this,FXmppStream->streamJid(),query,Options::node(OPV_XMPPSTREAMS_TIMEOUT_ROSTERREQUEST).value().toInt()))
	{
		FOpenRequestId = query.id();
		LOG_STRM_INFO(streamJid(),QString("Roster items request sent, ver=%1").arg(FVerSupported ? FRosterVer : QString()));
	}
	else
	{
		LOG_STRM_WARNING(streamJid(),"Failed to send roster items request");
	}
}

void Roster::requestGroupDelimiter()
{
	Stanza query(STANZA_KIND_IQ);
	query.setType(STANZA_TYPE_GET).setUniqueId();
	query.addElement("query",NS_JABBER_PRIVATE).appendChild(query.createElement("roster",NS_STORAGE_GROUP_DELIMITER));
	if (FStanzaProcessor->sendStanzaRequest(this,FXmppStream->streamJid(),query,Options::node(OPV_XMPPSTREAMS_TIMEOUT_ROSTERREQUEST).value().toInt()))
	{
		FDelimRequestId = query.id();
		LOG_STRM_INFO(streamJid(),"Roster delimiter request sent");
	}
	else
	{
		LOG_STRM_WARNING(streamJid(),"Failed to send roster delimiter request");
	}
}

void Roster::setGroupDelimiter(const QString &ADelimiter)
{
	if (FGroupDelim != ADelimiter)
	{
		LOG_STRM_INFO(streamJid(),QString("Changing group delimiter to='%1'").arg(ADelimiter));
		clearRosterItems();
		FGroupDelim = ADelimiter;
	}
}

void Roster::processItemsElement(const QDomElement &AItemsElem, bool ACompleteRoster)
{
	if (!AItemsElem.isNull())
	{
		FRosterVer = AItemsElem.attribute("ver");
		QSet<Jid> oldItems = ACompleteRoster ? FItems.keys().toSet() : QSet<Jid>();
		QDomElement itemElem = AItemsElem.firstChildElement("item");
		while (!itemElem.isNull())
		{
			Jid itemJid = itemElem.attribute("jid");
			if (itemJid.isValid() && !itemJid.hasResource())
			{
				QString subs = itemElem.attribute("subscription");
				if (subs==SUBSCRIPTION_BOTH || subs==SUBSCRIPTION_TO || subs==SUBSCRIPTION_FROM || subs==SUBSCRIPTION_NONE)
				{
					IRosterItem &ritem = FItems[itemJid];
					IRosterItem before = ritem;

					ritem.itemJid = itemJid;
					ritem.name = itemElem.attribute("name");
					ritem.subscription = subs;
					ritem.subscriptionAsk = itemElem.attribute("ask");
					oldItems -= ritem.itemJid;

					QSet<QString> allItemGroups;
					QDomElement groupElem = itemElem.firstChildElement("group");
					while (!groupElem.isNull())
					{
						QString group = replaceGroupDelimiter(groupElem.text(),FGroupDelim,ROSTER_GROUP_DELIMITER);
						if (!group.trimmed().isEmpty())
							allItemGroups += group;
						groupElem = groupElem.nextSiblingElement("group");
					}
					ritem.groups = allItemGroups;

					if (ritem != before)
					{
						LOG_STRM_DEBUG(streamJid(),QString("Roster item updated, jid=%1, name=%2, groups=%3, subscr=%4").arg(ritem.itemJid.bare(),ritem.name,QStringList(ritem.groups.toList()).join("; "),ritem.subscription));
						emit itemReceived(ritem,before);
					}
				}
				else if (subs == SUBSCRIPTION_REMOVE)
				{
					oldItems += itemJid;
				}
			}
			itemElem = itemElem.nextSiblingElement("item");
		}

		foreach(const Jid &itemJid, oldItems)
		{
			IRosterItem ritem = FItems.take(itemJid);
			IRosterItem before = ritem;
			ritem.subscription = SUBSCRIPTION_REMOVE;
			LOG_STRM_DEBUG(streamJid(),QString("Roster item removed, jid=%1").arg(ritem.itemJid.bare()));
			emit itemReceived(ritem,before);
		}
	}
}

QString Roster::replaceGroupDelimiter(const QString &AGroup, const QString &AFrom, const QString &ATo) const
{
	return AGroup.split(AFrom,QString::SkipEmptyParts).join(ATo);
}

void Roster::onXmppStreamOpened()
{
	static const QStringList skipDelimRequestDomains = QStringList() << "facebook.com";

	FXmppStream->removeXmppStanzaHandler(XSHO_XMPP_FEATURE,this);

	bool skipDelimRequest = false;
	QString serverDomain = FXmppStream->streamJid().pDomain();
	foreach(const QString &skipDomain, skipDelimRequestDomains)
	{
		if (serverDomain==skipDomain || serverDomain.endsWith("."+skipDomain))
		{
			skipDelimRequest = true;
			break;
		}
	}

	if (skipDelimRequest)
	{
		setGroupDelimiter(ROSTER_GROUP_DELIMITER);
		requestRosterItems();
	}
	else
	{
		requestGroupDelimiter();
	}
}

void Roster::onXmppStreamClosed()
{
	if (isOpen())
	{
		FOpened = false;
		emit closed();
	}
	FVerSupported = false;
	FSubscriptionRequests.clear();
	FXmppStream->insertXmppStanzaHandler(XSHO_XMPP_FEATURE,this);
}

void Roster::onXmppStreamJidAboutToBeChanged(const Jid &AAfter)
{
	emit streamJidAboutToBeChanged(AAfter);
	if (FXmppStream->streamJid().pBare() != AAfter.pBare())
		clearRosterItems();
}

void Roster::onXmppStreamJidChanged(const Jid &ABefore)
{
	emit streamJidChanged(ABefore);
}

#include <QDateTime>
#include "jinglestanza.h"
#include "jinglesession.h"
#include "definitions/namespaces.h"

QString JingleStanza::actions[]={"session-initiate",
                                 "session-accept",
                                 "session-terminate",
                                 "session-info",
                                 "session-info",
                                 "description-info",
                                 "content-add",
                                 "content-modify",
                                 "content-accept",
                                 "content-reject",
                                 "content-remove",
                                 "transport-accept",
                                 "transport-info",
                                 "transport-reject",
                                 "transport-replace"};

QString JingleStanza::reasons[]={"success",
                                 "busy",
                                 "cancel",
                                 "alternative-session",
                                 "connectivity-error",
                                 "decline",
                                 "expired",
                                 "failed-application",
                                 "failed-transport",
                                 "general-error",
                                 "gone",
                                 "incompatible-parameters",
                                 "media-error",
                                 "security-error",
                                 "timeout",
                                 "unsupported-applications",
                                 "unsupported-transports"};

JingleStanza::JingleStanza(const Jid &AFrom, const Jid &ATo, const QString &ASid, IJingle::Action AAction):
    Stanza("iq"), jingle(addElement("jingle", NS_JINGLE))
{
    QString     action;
    switch (AAction)
    {
        case IJingle::SessionInitiate:
            action="session-initiate";
            break;
        case IJingle::SessionAccept:
            action="session-accept";
            break;
        case IJingle::SessionTerminate:
            action="session-terminate";
            break;
        case IJingle::SessionInfo:
            action="session-info";
            break;
        case IJingle::ScurityInfo:
            action="security-info";
            break;
        case IJingle::DescriptionInfo:
            action="description-info";
            break;
        case IJingle::ContentAdd:
            action="content-add";
            break;
        case IJingle::ContentModify:
            action="content-modify";
            break;
        case IJingle::ContentAccept:
            action="content-accept";
            break;
        case IJingle::ContentReject:
            action="content-reject";
            break;
        case IJingle::ContentRemove:
            action="content-remove";
            break;
        case IJingle::TransportAccept:
            action="transport-accept";
            break;
        case IJingle::TransportInfo:
            action="transport-info";
            break;
        case IJingle::TransportReject:
            action="transport-reject";
            break;
        case IJingle::TransportReplace:
            action="transport-replace";
            break;
        default:
            return;
    }

    setType("set");
    setFrom(AFrom.full());
    setTo(ATo.full());
    setId(QString("id%1").arg(QDateTime::currentDateTime().toTime_t(),0,16));

    jingle.setAttribute("action", action);
    jingle.setAttribute("sid", ASid);
}

JingleStanza::JingleStanza(const Stanza &AStanza):Stanza(AStanza)
{
    QDomElement root=AStanza.document().documentElement();
    if (root.tagName()=="iq")
        jingle=root.firstChildElement("jingle");
}

void JingleStanza::setInitiator(const Jid &AInitiator)
{
    jingle.setAttribute("initiator", AInitiator.full());
}

void JingleStanza::setResponder(const Jid &AInitiator)
{
    jingle.setAttribute("responder", AInitiator.full());
}

QDomElement JingleStanza::importJingleElement(const QDomElement &AElement)
{
    return jingle.appendChild(document().importNode(AElement, true)).toElement();
}

QDomElement JingleStanza::addJingleElement(const QDomElement &AElement)
{
    return jingle.appendChild(AElement).toElement();
}

QDomElement JingleStanza::createJingleElement(const QString &ATagName, const QString &ANameSpace)
{
    return addJingleElement(ANameSpace.isEmpty()?document().createElement(ATagName)
                                                :document().createElementNS(ATagName, ANameSpace));
}

QDomElement JingleStanza::setReason(IJingle::Reason AReason)
{
    if (AReason==IJingle::NoReason)
        return QDomElement();
    QDomElement reason=createJingleElement("reason");
    if (!reason.isNull())
        reason.appendChild(createElement(reasons[(int)AReason-1]));
    return reason;
}

QString JingleStanza::reasonName() const
{
    QDomElement reason=jingle.firstChildElement("reason");
    if (!reason.isNull())
    {
        QDomElement name=reason.firstChild().toElement();        
        if (!name.isNull())
            return name.tagName();
    }
    return QString::null;
}

IJingle::Reason JingleStanza::reason() const
{
    return reason4name(reasonName());
}

bool JingleStanza::isValid() const
{
    return !id().isNull() &&
           !jingle.isNull() &&
           !jingle.attribute("sid").isNull() &&
           jingle.namespaceURI()==NS_JINGLE &&
            action()!=IJingle::NoAction;
}

Stanza JingleStanza::ack(IJingle::CommandRespond ARespond) const
{
    Stanza ack("iq");
    ack.setId(id());
    ack.setFrom(to());
    ack.setTo(from());
    if (ARespond==IJingle::Acknowledge)
        ack.setType("result");
    else
    {
        ack.setType("error");
        QDomElement error=ack.addElement("error");
        switch (ARespond)
        {
            case IJingle::ServiceUnavailable:
                error.setAttribute("type", "cancel");
                error.appendChild(ack.createElement("service-unavailable", "urn:ietf:params:xml:ns:xmpp-stanzas"));
                break;
            case IJingle::Redirect:
                error.setAttribute("type", "modify");
                error.appendChild(ack.createElement("redirect", "urn:ietf:params:xml:ns:xmpp-stanzas"));
                break;
            case IJingle::ResourceConstraint:
                error.setAttribute("type", "wait");
                error.appendChild(ack.createElement("resource-constraint", "urn:ietf:params:xml:ns:xmpp-stanzas"));
                break;
            case IJingle::BadRequest:
                error.setAttribute("type", "cancel");
                error.appendChild(ack.createElement("bad-request", "urn:ietf:params:xml:ns:xmpp-stanzas"));
                break;
            default:
                break;
        }
    }
    return ack;
}

IJingle::Action JingleStanza::action() const
{
    return action4name(actionName());
}

IJingle::Action JingleStanza::action4name(const QString &AActionName)
{
    for (int i=0; i<15; i++)
        if (actions[i]==AActionName)
            return IJingle::Action(i+1);
    return IJingle::NoAction;
}

IJingle::Reason JingleStanza::reason4name(const QString &AReasonName)
{
    for (int i=0; i<17; i++)
        if (reasons[i]==AReasonName)
            return IJingle::Reason(i+1);
    return IJingle::NoReason;
}

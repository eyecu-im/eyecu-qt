#ifndef JINGLESTANZA_H
#define JINGLESTANZA_H

#include "utils/stanza.h"
#include "utils/jid.h"
#include "interfaces/ijingle.h"

class JingleStanza : public Stanza
{
public:
    JingleStanza(const Jid &AFrom, const Jid &ATo, const QString &ASid, IJingle::Action AAction);
    JingleStanza(const Stanza &AStanza);
    void    setInitiator(const Jid &AInitiator);
    QString initiator() const {return jingle.attribute("initiator");}
    void    setResponder(const Jid &AInitiator);
    QString responder() const {return jingle.attribute("responder");}
    QString sid() const {return jingle.attribute("sid");}
    QString actionName() const {return jingle.attribute("action");}
    IJingle::Action action() const;
    QDomElement jingleElement() const {return jingle;}

    QDomElement setReason(IJingle::Reason AReason);    
    QString reasonName() const;
    IJingle::Reason reason()const;    

    bool    isValid() const;

    QDomElement importJingleElement(const QDomElement &AElement);
    QDomElement addJingleElement(const QDomElement &AElement);
    QDomElement createJingleElement(const QString &ATagName, const QString &ANameSpace=QString::null);

    Stanza  ack(IJingle::CommandRespond ARespond=IJingle::Acknowledge) const;

protected:   
    static QString name4action(IJingle::Action AAction) {return actions[AAction];}
    static IJingle::Action action4name(const QString &AActionName);
    static QString name4reason(IJingle::Reason AReason) {return AReason?reasons[AReason-1]:QString::null;}
    static IJingle::Reason reason4name(const QString &AReasonName);

private:
    QDomElement jingle;
    static QString actions[15];
    static QString reasons[17];
};    
#endif // JINGLESTANZA_H

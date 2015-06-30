#ifndef IMAPMESSAGE
#define IMAPMESSAGE_H

#include <QObject>

#define MAPMESSAGE_UUID "{89dd35ee-bd44-49fc-8495-edd2cfebb685}"

class IBubbleUrlEventHandler
{
public:
    virtual QObject *instance() =0;
    virtual bool bubbleUrlOpen(int AOrder, const QUrl &AUrl, const Jid &AStreamJid, const Jid &AContactJid) = 0;
};


class IMapMessage {
public:
    virtual QObject *instance() =0;
    virtual void insertUrlHandler(int AOrder, IBubbleUrlEventHandler *ABubbleUrlEventHandler) = 0;
    virtual void removeUrlHandler(int AOrder, IBubbleUrlEventHandler *ABubbleUrlEventHandler) = 0;
};
// Q_DECLARE_INTERFACE(IBubbleEventListener, "RWS.Plugin.IBubbleEventListener/1.0")
Q_DECLARE_INTERFACE(IBubbleUrlEventHandler, "RWS.Plugin.IBubbleUrlEventHandler/1.0")
Q_DECLARE_INTERFACE(IMapMessage, "RWS.Plugin.IMapMessage/1.0")

#endif	//MAPMESSAGE_H

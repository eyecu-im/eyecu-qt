#ifndef ICEUDPTHREAD_H
#define ICEUDPTHREAD_H

#include <QThread>
#include <QUdpSocket>
#include "utils/jid.h"

class IceUdpThread : public QThread
{
    Q_OBJECT
public:
    IceUdpThread(const Jid &AStreamJid, const QString &ASid, const QString &AContentName, const QString &ACandidateId, QObject *parent = 0);


signals:
    void connectionEstablished(const QString &AStreamJid, const QString &ASid, const QString &AContentName, const QString &ACandidateId, QIODevice *ADevice);
    void connectionFailed(const QString &AStreamJid, const QString &ASid, const QString &AContentName, const QString &AErrorString);
    
public slots:

protected:
    void run();

private:
    const Jid     FStreamJid;
    const QString FSid;
    const QString FContentName;
    const QString FCandidateId;
};

#endif // ICEUDPTHREAD_H

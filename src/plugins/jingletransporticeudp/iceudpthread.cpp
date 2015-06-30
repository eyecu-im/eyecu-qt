#include <QDebug>
#include "iceudpthread.h"

IceUdpThread::IceUdpThread(const Jid &AStreamJid, const QString &ASid, const QString &AContentName, const QString &ACandidateId, QObject *parent):
    QThread(parent), FStreamJid(AStreamJid), FSid(ASid),
    FContentName(AContentName), FCandidateId(ACandidateId)
{}

void IceUdpThread::run()
{
    qDebug() << "IceUdpThread::run()";
    QUdpSocket *socket=new QUdpSocket();
    sleep(1);    // Just wait for one second
    qDebug() << "Emitting connectionEstablished(" << FStreamJid.full() << ", "
             << FSid << ", "
             << FCandidateId << ", "
             << socket << ")";
    emit connectionEstablished(FStreamJid.full(), FSid, FContentName, FCandidateId, socket);
}

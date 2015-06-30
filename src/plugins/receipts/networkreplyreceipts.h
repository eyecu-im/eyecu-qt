#ifndef NETWORKREPLYRECEIPTS_H
#define NETWORKREPLYRECEIPTS_H

#include "receipts.h"
#include <QNetworkReply>
#include <utils/jid.h>

class NetworkReplyReceipts : public QNetworkReply
{
    Q_OBJECT
public:
    NetworkReplyReceipts(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, Receipts *AReceipts, const QByteArray *AImageData, QObject *parent = 0);
    virtual bool isSequential() const {return false;}
    virtual qint64 size() const;
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual void abort();

protected slots:
    virtual void readDataChunk();
    virtual void onDelivered(const QString &AId);

private:
    const Receipts      *FReceipts;
    const QByteArray    *FImageData;
    bool                ready;
};

#endif // NETWORKREPLYRECEIPTS_H

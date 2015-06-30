#ifndef NETWORKREPLYPRIVATE_H
#define NETWORKREPLYPRIVATE_H

#include <QNetworkReply>
#include <interfaces/iurlprocessor.h>
#include <interfaces/ibitsofbinary.h>
#include "boburlhandler.h"

class NetworkReplyPrivate : public QNetworkReply
{
    Q_OBJECT

public:
	NetworkReplyPrivate(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, BobUrlHandler *AHandler, IBitsOfBinary *ABitsObBinary, QObject *AParent = 0);
    virtual bool isSequential() const {return false;}
    virtual qint64 size() const;
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual void abort();

protected slots:
    virtual void onBinaryCached(const QString &AContentId, const QString &AType, const QByteArray &AData, quint64 AMaxAge);
//    virtual void onBinaryError(const QString &AContentId, const QString &AError);
    virtual void readDataChunk();

private:
    BobUrlHandler   *FHandler;
    IBitsOfBinary   *FBitsOfBinary;
    QByteArray      FBuffer;
};

#endif // NETWORKREPLYPRIVATE_H

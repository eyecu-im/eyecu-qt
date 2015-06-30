#include "networkreplyreceipts.h"
#include <QTimer>

NetworkReplyReceipts::NetworkReplyReceipts(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, Receipts *AReceipts, const QByteArray *AImageData, QObject *parent):
    QNetworkReply(parent),
    FReceipts(AReceipts),
    FImageData(AImageData),
    ready(false)
{
	Q_UNUSED(AOutgoingData)

    setOperation(AOperation);
    setRequest(ARequest);
    setUrl(ARequest.url());
    open(ReadOnly); // Open the device as Read Only
    connect(FReceipts, SIGNAL(delivered(QString)), SLOT(onDelivered(QString)));
    QTimer::singleShot(0, this, SLOT(readDataChunk()));
}

qint64 NetworkReplyReceipts::size() const
{
    return ready?FImageData->size():0;
}

qint64 NetworkReplyReceipts::readData(char *data, qint64 maxlen)
{
    if (ready)
    {
        qint64 left=FImageData->size()-pos();
        if (!left)
            return -1;
        qint64 toRead=maxlen?maxlen<left?maxlen:left:left;
        memcpy(data, FImageData->data(), toRead);
        return toRead;
    }
    return 0;
}

void NetworkReplyReceipts::abort()
{
    if (isRunning())
        emit error(QNetworkReply::OperationCanceledError);
    close();
}

void NetworkReplyReceipts::readDataChunk()
{
    if ((ready = FReceipts->isDelivered(url().path())))
    {
        emit readyRead();
        emit readChannelFinished();
        emit finished();
    }
}

void NetworkReplyReceipts::onDelivered(const QString &AId)
{
    if (url().path()==AId)
        readDataChunk();
}

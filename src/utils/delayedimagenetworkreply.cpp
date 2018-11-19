
#include "delayedimagenetworkreply.h"
#include <QTimer>

DelayedImageNetworkReply::DelayedImageNetworkReply(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, const QByteArray *AImageData, QObject *parent):
    QNetworkReply(parent),
    FImageData(AImageData),
    ready(false)
{
    Q_UNUSED(AOutgoingData)

    setOperation(AOperation);
    setRequest(ARequest);
    setUrl(ARequest.url());
    open(ReadOnly); // Open the device as Read Only
}

qint64 DelayedImageNetworkReply::size() const
{
    return ready?FImageData->size():0;
}

qint64 DelayedImageNetworkReply::readData(char *data, qint64 maxlen)
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

void DelayedImageNetworkReply::abort()
{
    if (isRunning())
        emit error(QNetworkReply::OperationCanceledError);
    close();
}

void DelayedImageNetworkReply::onReady(const QString &AId)
{
    if (url().path()==AId)
    {
        emit readyRead();
        emit readChannelFinished();
        emit finished();
    }
}

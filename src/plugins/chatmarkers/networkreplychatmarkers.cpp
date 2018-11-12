#include "networkreplychatmarkers.h"
#include <QTimer>

NetworkReplyChatMarkers::NetworkReplyChatMarkers(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, ChatMarkers *AChatMarkers, const QByteArray *AImageData, QObject *parent):
    QNetworkReply(parent),
    FChatMarkers(AChatMarkers),
    FImageData(AImageData),
    ready(false)
{
	Q_UNUSED(AOutgoingData)

    setOperation(AOperation);
    setRequest(ARequest);
    setUrl(ARequest.url());
    open(ReadOnly); // Open the device as Read Only
	connect(FChatMarkers, SIGNAL(marked(QString)), SLOT(onMarked(QString)));
    QTimer::singleShot(0, this, SLOT(readDataChunk()));
}

qint64 NetworkReplyChatMarkers::size() const
{
    return ready?FImageData->size():0;
}

qint64 NetworkReplyChatMarkers::readData(char *data, qint64 maxlen)
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

void NetworkReplyChatMarkers::abort()
{
    if (isRunning())
        emit error(QNetworkReply::OperationCanceledError);
    close();
}

void NetworkReplyChatMarkers::readDataChunk()
{
	if ((ready = FChatMarkers->isMarked(url().path())))
    {
        emit readyRead();
        emit readChannelFinished();
        emit finished();
    }
}

void NetworkReplyChatMarkers::onMarked(const QString &AId)
{
    if (url().path()==AId)
        readDataChunk();
}

#include "networkreplyprivate.h"
#include <QTimer>

NetworkReplyPrivate::NetworkReplyPrivate(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, BobUrlHandler *AHandler, IBitsOfBinary *ABitsObBinary, QObject *AParent):
	QNetworkReply(AParent), FHandler(AHandler), FBitsOfBinary(ABitsObBinary)
{
	Q_UNUSED(AOutgoingData)

	setOperation(AOperation);
	setRequest(ARequest);
	setUrl(ARequest.url());
	open(ReadOnly); // Open the device as Read Only

	connect(ABitsObBinary->instance(), SIGNAL(binaryCached(QString, QString, QByteArray, quint64)),
									   SLOT(onBinaryCached(QString, QString, QByteArray, quint64)));
/*
	connect(ABitsObBinary->instance(), SIGNAL(binaryError(QString, QString)),
									   SLOT(onBinaryError(QString, QString)));
*/
	QTimer::singleShot(0, this, SLOT(readDataChunk()));
}

qint64 NetworkReplyPrivate::size() const
{
	return FBuffer.size();
}

qint64 NetworkReplyPrivate::readData(char *data, qint64 maxlen)
{
	qint64 left=FBuffer.size()-pos();
	if (!left)
		return -1;
	qint64 toRead=maxlen?maxlen<left?maxlen:left:left;
	memcpy(data, FBuffer.data()+pos(), toRead);
	return toRead;
}

void NetworkReplyPrivate::readDataChunk()
{
	QString cid=url().path();

	if (FBitsOfBinary->hasBinary(cid))
	{
		QString type;
		quint64 maxAge;
		if (FBitsOfBinary->loadBinary(cid, type, FBuffer, maxAge))
		{
			setHeader(QNetworkRequest::ContentTypeHeader, type);
			setHeader(QNetworkRequest::ContentLengthHeader, FBuffer.size());
			emit readyRead();
			emit readChannelFinished();
			emit finished();
		}
	}
}

void NetworkReplyPrivate::abort()
{
	if (isRunning())
		emit error(QNetworkReply::OperationCanceledError);
	close();
}

void NetworkReplyPrivate::onBinaryCached(const QString &AContentId, const QString &AType, const QByteArray &AData, quint64 AMaxAge)
{
	Q_UNUSED(AType)
	Q_UNUSED(AData)
	Q_UNUSED(AMaxAge)

	if (url().path()==AContentId)
		readDataChunk();
}

/*
void NetworkReplyPrivate::onBinaryError(const QString &AContentId, const QString &AError)
{
	if (url().path()==AContentId)
	{
		setError(QNetworkReply::UnknownNetworkError, AError);
		emit error(QNetworkReply::UnknownNetworkError);
		emit finished();
	}
}
*/

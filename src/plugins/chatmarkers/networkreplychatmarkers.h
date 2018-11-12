#ifndef NetworkReplyChatMarkers_H
#define NetworkReplyChatMarkers_H

#include "chatmarkers.h"
#include <QNetworkReply>
#include <utils/jid.h>

class NetworkReplyChatMarkers : public QNetworkReply
{
    Q_OBJECT
public:
	NetworkReplyChatMarkers(QNetworkAccessManager::Operation AOperation,
							const QNetworkRequest &ARequest,
							QIODevice *AOutgoingData,
							ChatMarkers *AChatMarkers,
							const QByteArray *AImageData,
							QObject *parent = nullptr);
    virtual bool isSequential() const {return false;}
    virtual qint64 size() const;
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual void abort();

protected slots:
	void readDataChunk();
	void onMarked(const QString &AId);

private:
	const ChatMarkers   *FChatMarkers;
    const QByteArray    *FImageData;
    bool                ready;
};

#endif // NetworkReplyChatMarkers_H

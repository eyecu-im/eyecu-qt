#ifndef DELAYEDIMAGENETWORKREPLY
#define DELAYEDIMAGENETWORKREPLY

#include <QNetworkReply>
#include "jid.h"

class UTILS_EXPORT DelayedImageNetworkReply :
    public QNetworkReply
{
    Q_OBJECT
public:
    DelayedImageNetworkReply(QNetworkAccessManager::Operation AOperation, const QNetworkRequest &ARequest, QIODevice *AOutgoingData, const QByteArray *AImageData, QObject *parent = 0);
    virtual bool isSequential() const {return false;}
    virtual qint64 size() const;
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual void abort();

protected slots:
    virtual void onReady(const QString &AId);

private:
    const QByteArray    *FImageData;
    bool                ready;
};

#endif // DELAYEDIMAGENETWORKREPLY

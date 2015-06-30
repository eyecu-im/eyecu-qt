#ifndef TUNEIMAGEHTTPQUERY_H
#define TUNEIMAGEHTTPQUERY_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QUuid>
#include <QNetworkProxy>
#include <MapCoordinates>
#include <interfaces/itune.h>

class TuneImageHttpQuery : public QObject
{
    Q_OBJECT

public:
    TuneImageHttpQuery(const QUrl &AUrl, const QString &AArtist, const QString &AAlbum);
    void sendRequest(QNetworkAccessManager *ANetworkAccessManager);

signals:
    void resultReceived(const QByteArray &AResult, const QString &AArtist, const QString &AAlbum);

private slots:
    void httpFinished();

private:
    QUrl        FUrl;
    QString     FArtist;
    QString     FAlbum;
    QByteArray  FResult;
};

#endif // TUNEIMAGEHTTPQUERY

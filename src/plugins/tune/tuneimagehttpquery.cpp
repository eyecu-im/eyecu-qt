#include <QtNetwork>
#include "tuneimagehttpquery.h"

TuneImageHttpQuery::TuneImageHttpQuery(const QUrl &AUrl, const QString &AArtist, const QString &AAlbum):
    FUrl(AUrl), FArtist(AArtist), FAlbum(AAlbum)
{}

void TuneImageHttpQuery::sendRequest(QNetworkAccessManager *ANetworkAccessManager)
{
    QNetworkReply *reply = ANetworkAccessManager->get(QNetworkRequest(FUrl));
    connect(reply, SIGNAL(finished()), SLOT(httpFinished()));
}

void TuneImageHttpQuery::httpFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error()==QNetworkReply::NoError)
        FResult = reply->readAll();
    emit resultReceived(FResult, FArtist, FAlbum);
    reply->deleteLater();
}

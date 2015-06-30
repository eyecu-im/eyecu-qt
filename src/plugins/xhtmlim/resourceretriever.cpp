#include "resourceretriever.h"

#include <utils/logger.h>

ResourceRetriever::ResourceRetriever(QTextEdit *ATextEdit, QNetworkReply *AReply) :
    QObject(AReply), FTextEdit(ATextEdit)
{
    connect(AReply, SIGNAL(finished()), SLOT(onFinished()));
}

void ResourceRetriever::onFinished()
{
    QNetworkReply *reply=qobject_cast<QNetworkReply *>(sender());
    if (reply->error()==QNetworkReply::NoError)
    {
        if (reply->size())
        {
            QByteArray bytes=reply->readAll();
            FTextEdit->document()->addResource(QTextDocument::ImageResource, reply->url(), bytes);
            FTextEdit->setLineWrapColumnOrWidth(FTextEdit->lineWrapColumnOrWidth());
        }
        else
			LOG_ERROR("Zero length data retrieved!");
    }
    else
		LOG_ERROR(QString("Resource retrieving failed: %1 (%2)").arg(reply->errorString()).arg(reply->error()));
    reply->deleteLater();
    deleteLater();
}

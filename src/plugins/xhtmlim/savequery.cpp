#include <QNetworkReply>
#include <QMessageBox>

#include <utils/logger.h>

#include "savequery.h"

SaveQuery::SaveQuery(QNetworkAccessManager *ANetworkAccessManager, QString AFullPath, QString uri):
    FReply(ANetworkAccessManager->get(QNetworkRequest(uri))),
    FFullPath(AFullPath),
    FFile(FFullPath)
{
    QStringList splitDir = FFullPath.split("/");
    QDir dir;

    for (int i=0; i<splitDir.count()-1; i++)
    {
        splitDir[i].append('/');
        if (!dir.exists(splitDir[i]))
            if (!dir.mkdir(splitDir[i]))
			   LOG_WARNING(QString("mkdir[\"%1\"] failed!\n").arg(splitDir[i].toLatin1().data()));
        if (!dir.cd(splitDir[i]))
			LOG_WARNING(QString("cd[\"%1\"] failed!\n").arg(splitDir[i].toLatin1().data()));
    }
    FProgress.setLabelText(tr("Saving image..."));
    FProgress.show();
    connect(&FProgress, SIGNAL(canceled()), SLOT(onCancelled()));

    connect(FReply, SIGNAL(readyRead()), SLOT(onReadyRead()));
//    connect(FReply, SIGNAL(readChannelFinished()), SLOT(onReadChannelFinished()));
    connect(FReply, SIGNAL(finished()), SLOT(onFinished()));
    connect(FReply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(onDownloadProgress(qint64,qint64)));
}

void SaveQuery::onReadyRead()
{
    QByteArray imageData = FReply->readAll();

    if(!FFile.isOpen())
        if(!FFile.open(QIODevice::WriteOnly))
            FReply->abort();
    FFile.write(imageData);
}
/*
void SaveQuery::onReadChannelFinished()
{
    FReply->deleteLater();
    if (FFile.isOpen())
        FFile.close();
    deleteLater();
}
*/

void SaveQuery::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    FProgress.setMaximum(bytesTotal);
    FProgress.setValue(bytesReceived);
}

void SaveQuery::onCancelled()
{
	FReply->abort();
}

void SaveQuery::onFinished()
{
	FReply->deleteLater();
	if (FFile.isOpen())
		FFile.close();
	deleteLater();
}
//-------------------------------------

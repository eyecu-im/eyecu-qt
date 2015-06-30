#ifndef SAVEQUERY_H
#define SAVEQUERY_H

#include <QNetworkAccessManager>
#include <QProgressDialog>
#include <QDir>
#include <QUrl>

class SaveQuery : public QObject
{
    Q_OBJECT

public:
    SaveQuery(QNetworkAccessManager *ANetworkAccessManager, QString AFullPath, QString uri);

private slots:
    void onReadyRead();
//    void onReadChannelFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onCancelled();
	void onFinished();

private:    
    QNetworkReply           *FReply;
    QString                 FFullPath;
    QFile                   FFile;
    QProgressDialog         FProgress;
};

#endif // SAVEQUERY_H

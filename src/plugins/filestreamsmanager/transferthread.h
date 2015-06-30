#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QFile>
#include <QThread>
#include <QMutex>   // *** <<< eyeCU >>> ***
#include <interfaces/ifilestreamsmanager.h>
#include <interfaces/idatastreamsmanager.h>

class TransferThread :
	public QThread
{
	Q_OBJECT;
public:
    TransferThread(IDataStreamSocket *ASocket, QFile *AFile, QList<QIODevice *> *AOutputDevices, QMutex *AOutputMutex, int AKind, qint64 ABytes, QObject *AParent);   // *** <<< eyeCU >>> ***
	~TransferThread();
	void abort();
	bool isAborted() const;
signals:
	void transferProgress(qint64 ABytes);
protected:
	void run();
private:
	int FKind;
	QFile *FFile;
	qint64 FBytesToTransfer;
	IDataStreamSocket *FSocket;
// *** <<< eyeCU <<< ***
    QList<QIODevice *> *FOutputDevices;
    QMutex             *FOutputMutex;
// *** >>> eyeCU >>> ***
private:
	volatile bool FAborted;
};

#endif // TRANSFERTHREAD_H

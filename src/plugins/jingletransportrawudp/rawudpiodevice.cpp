#include "rawudpiodevice.h"
#include <QBuffer>
#include <QThread>

RawUdpIODevice::RawUdpIODevice(QUdpSocket *AInputSocket,
							   QUdpSocket *AOutputSocket,
							   QObject *AParent):
	QIODevice(AParent), FInputSocket(AInputSocket),
	FOutputSocket(AOutputSocket)
{
	qDebug() << "RawUdpIODevice(" << AInputSocket << "," << AOutputSocket
			 << "," << AParent << "); this=" << this;
	qDebug() << "curren thread:" << thread();
	QThread *targetThread = new QThread();
	moveToThread(targetThread);
	qDebug() << "moved to thread:" << thread();

	if (FInputSocket) {
		FInputSocket->moveToThread(targetThread);
		qDebug() << "input socket moved to thread:" << FInputSocket->thread();
//		FInputSocket->setParent(this);
//		qDebug() << "input socket's parent:" << FInputSocket->parent();
	}
	if (FOutputSocket) {
		FOutputSocket->moveToThread(targetThread);
		qDebug() << "output socket moved to thread:" << FInputSocket->thread();
//		FOutputSocket->setParent(this);
//		qDebug() << "input socket's parent:" << FInputSocket->parent();
	}

	qDebug() << "HERE!";

	connect(this, SIGNAL(aboutToClose()), targetThread, SLOT(quit()));
	connect(this, SIGNAL(writeSocket()), SLOT(onWriteSocket()));
	qDebug() << "CONNECTED!";
	targetThread->start();
	qDebug() << "STARTED!";
}

void RawUdpIODevice::setInputSocket(QUdpSocket *ASocket)
{
	qDebug() << "RawUdpIODevice::setInputSocket(" << ASocket << "); this=" << this;
	FInputMutex.lock();
	if (ASocket != FInputSocket)
	{
		if (FInputSocket)
		{
			FInputSocket->disconnect(SIGNAL(readyRead()), this,
									 SLOT(onReadyRead()));
			FInputSocket->close();
			FInputSocket->deleteLater();
		}
		FInputSocket = ASocket;		
		if (FInputSocket)
		{
			FInputSocket->moveToThread(thread());
			qDebug() << "input socket moved to thread:" << FInputSocket->thread();
//			FInputSocket->setParent(this);
//			qDebug() << "input socket's parent:" << FInputSocket->parent();
			if (openMode().testFlag(ReadOnly)) {
				connect(FInputSocket, SIGNAL(readyRead()),
									  SLOT(onReadyRead()));
				onReadyRead();
			}
		}
	}
	FInputMutex.unlock();
}

QUdpSocket *RawUdpIODevice::inputSocket() const
{
	return FInputSocket;
}

void RawUdpIODevice::setOutputSocket(QUdpSocket *ASocket)
{
	qDebug() << "RawUdpIODevice::setOutputSocket(" << ASocket << "); this=" << this;
	bool emitSignal(false);
	FOutputMutex.lock();
	if (ASocket != FOutputSocket)
	{
		if (FOutputSocket)
		{
			FOutputSocket->disconnect(SIGNAL(bytesWritten(qint64)), this,
									  SIGNAL(bytesWritten(qint64)));
			FOutputSocket->disconnectFromHost();
			FOutputSocket->deleteLater();
		}
		FOutputSocket = ASocket;
		if (FOutputSocket)
		{
			FOutputSocket->moveToThread(thread());
			qDebug() << "output socket moved to thread:" << FOutputSocket->thread();
//			FOutputSocket->setParent(this);
//			qDebug() << "output socket's parent:" << FOutputSocket->parent();
			if (openMode().testFlag(WriteOnly)) {
				qDebug() << "CONNECTING SIGNAL...";
				connect(FOutputSocket, SIGNAL(bytesWritten(qint64)),
									   SIGNAL(bytesWritten(qint64)));
				qDebug() << "CONNECTED SIGNAL!";
				emitSignal = !FOutputQueue.isEmpty();
			}
		}
	}
	FOutputMutex.unlock();

	qDebug() << "A!";
	if (emitSignal)
		emit writeSocket();
	qDebug() << "B!";
}

QUdpSocket *RawUdpIODevice::outputSocket() const
{
	return FOutputSocket;
}

bool RawUdpIODevice::open(QIODevice::OpenMode mode)
{
	if ((mode.testFlag(ReadOnly) && !FInputSocket) ||
		(mode.testFlag(WriteOnly) && !FInputSocket))
		return false;

	if (mode.testFlag(ReadOnly))
	{
		connect(FInputSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
		onReadyRead();
	}

	if (mode.testFlag(WriteOnly))
	{
//		FOutputSocket->setParent(this);
		connect(FOutputSocket, SIGNAL(bytesWritten(qint64)), SIGNAL(bytesWritten(qint64)));
	}

	return QIODevice::open(mode);
}

void RawUdpIODevice::close()
{
	QIODevice::close();

	if (openMode().testFlag(ReadOnly))
		FInputSocket->disconnect(SIGNAL(readyRead()), this,
								 SLOT(onReadyRead()));
	if (openMode().testFlag(WriteOnly))
		FOutputSocket->disconnect(SIGNAL(bytesWritten(qint64)), this,
								  SIGNAL(bytesWritten(qint64)));
}

bool RawUdpIODevice::isSequential() const
{
	return true;
}

qint64 RawUdpIODevice::bytesAvailable() const
{	
	qint64 bytes(QIODevice::bytesAvailable());
	if (bytes == 0)
	{
		FInputMutex.lock();
		bytes += (FInputQueue.isEmpty()?0:FInputQueue.first().size());
		FInputMutex.unlock();
	}
	return bytes;
}

qint64 RawUdpIODevice::bytesToWrite() const
{
	qint64 bytes = 0;
	QMutexLocker locker(&FOutputMutex);
	for (QList<QByteArray>::ConstIterator it=FOutputQueue.constBegin();
		 it!=FOutputQueue.constEnd(); ++it)
		bytes+=it->size();
	return bytes;
}

qint64 RawUdpIODevice::readData(char *data, qint64 maxlen)
{
	qDebug() << "RawUdpIODevice::readData(data," << maxlen << ")";
	FInputMutex.lock();
	bool emitSignal(false);
	qint64 size = FInputQueue.isEmpty()?0:qMin(static_cast<qint64>(FInputQueue.first().size()), maxlen);
	if (size>0)
	{
		QBuffer buffer(&FInputQueue.first());
		buffer.open(ReadOnly);
		size = buffer.read(data, size);
		buffer.close();
		if (size < FInputQueue.first().size())
			FInputQueue.first().remove(0, static_cast<int>(size));
		else {
			FInputQueue.removeFirst();			
			emitSignal = !FInputQueue.isEmpty();
		}
	}
	FInputMutex.unlock();
	if (emitSignal)
		emit readyRead();
	qDebug() << "returning" << size;
	return size;
}

qint64 RawUdpIODevice::writeData(const char *data, qint64 len)
{
//	qDebug() << "RawUdpIODevice::writeData(data," << len << "); this=" << this;
	FOutputMutex.lock();
	FOutputQueue.enqueue(QByteArray(data, int(len)));
	FOutputMutex.unlock();
	emit writeSocket();
	return len;
}

void RawUdpIODevice::onWriteSocket()
{
//	qDebug() << "RawUdpIODevice::writeSocket(); this=" << this;
	FOutputMutex.lock();
	while (!FOutputQueue.isEmpty())
	{
		qint64 size = FOutputSocket->write(FOutputQueue.dequeue());
//		qDebug() << size << "bytes sent to" << FOutputSocket->peerAddress()
//						 << ":" << FOutputSocket->peerPort();
		Q_ASSERT(size>0);
	}
	FOutputMutex.unlock();
}

void RawUdpIODevice::onReadyRead()
{
//	qDebug() << "RawUdpIODevice::onReadyRead(); this=" << this;
	if (FInputSocket->hasPendingDatagrams())
	{
		qint64 size = FInputSocket->pendingDatagramSize();
		if (size>0)
		{
			QByteArray data;
			data.resize(int(size));

			QHostAddress addr;
			quint16 port;
			size = FInputSocket->readDatagram(data.data(), size, &addr, &port);
			Q_ASSERT(size==data.size());

			FInputMutex.lock();
			FInputQueue.enqueue(data);
			FInputMutex.unlock();

			qDebug() << "received" << size << "bytes from" << addr << ":" << port;

			emit readyRead();
		}
	}
}

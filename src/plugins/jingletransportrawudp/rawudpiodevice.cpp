#include "rawudpiodevice.h"
#include <QBuffer>
#include <QThread>
#include <QTimer>

RawUdpIODevice::RawUdpIODevice(QUdpSocket *AInputSocket, QObject *AParent):
	QIODevice(AParent), FSocket(AInputSocket)
{

	if (FSocket)
		FSocket->setParent(this);

	QThread *targetThread = new QThread();
	moveToThread(targetThread);

	connect(this, SIGNAL(aboutToClose()), targetThread, SLOT(quit()));
	connect(this, SIGNAL(writeSocket()), SLOT(onWriteSocket()));
	targetThread->start();
}

QUdpSocket *RawUdpIODevice::socket() const
{
	return FSocket;
}

void RawUdpIODevice::setTargetAddress(const QHostAddress &AHostAddress,
									  quint16 APort)
{
	bool emitSignal(false);
	FOutputMutex.lock();
	if (AHostAddress != FTargetAddress ||
		APort != FTargetPort)
	{
		FTargetAddress = AHostAddress;
		FTargetPort = APort;

		if (openMode().testFlag(WriteOnly))
			emitSignal = !FOutputQueue.isEmpty();
	}
	FOutputMutex.unlock();

	if (emitSignal)
		emit writeSocket();
}

bool RawUdpIODevice::open(QIODevice::OpenMode mode)
{
	if (mode.testFlag(ReadOnly))
	{
		connect(FSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
		QTimer::singleShot(0, this, SLOT(onReadyRead())); // This slot must be called from this objet's thread!
	}

	if (mode.testFlag(WriteOnly))
		connect(FSocket, SIGNAL(bytesWritten(qint64)), SIGNAL(bytesWritten(qint64)));

	return QIODevice::open(mode);
}

void RawUdpIODevice::close()
{
	if (openMode().testFlag(ReadOnly))
		FSocket->disconnect(SIGNAL(readyRead()), this,
								 SLOT(onReadyRead()));
	if (openMode().testFlag(WriteOnly))
		FSocket->disconnect(SIGNAL(bytesWritten(qint64)), this,
								  SIGNAL(bytesWritten(qint64)));

	QIODevice::close();
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
	FInputMutex.lock();
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
			QTimer::singleShot(0, this, SLOT(emitReadyRead()));
		}
	}
	FInputMutex.unlock();

	return size;
}

qint64 RawUdpIODevice::writeData(const char *data, qint64 len)
{
	FOutputMutex.lock();
	FOutputQueue.enqueue(QByteArray(data, int(len)));
	FOutputMutex.unlock();
	emit writeSocket();
	return len;
}

void RawUdpIODevice::onWriteSocket()
{
	if (FTargetAddress.isNull() || !FTargetPort)
		return;

	FOutputMutex.lock();
	while (!FOutputQueue.isEmpty())
	{
		qint64 size = FSocket->writeDatagram(QByteArray(FOutputQueue.dequeue()),
												  FTargetAddress, FTargetPort);
		Q_ASSERT(size>0);
	}
	FOutputMutex.unlock();
}

void RawUdpIODevice::onReadyRead()
{
	if (FSocket->hasPendingDatagrams())
	{
		qint64 size = FSocket->pendingDatagramSize();
		if (size>0)
		{
			QByteArray data;
			data.resize(int(size));

			QHostAddress addr;
			quint16 port;
			size = FSocket->readDatagram(data.data(), size, &addr, &port);
			Q_ASSERT(size==data.size());

			FInputMutex.lock();
			FInputQueue.enqueue(data);
			FInputMutex.unlock();

			QTimer::singleShot(0, this, SLOT(emitReadyRead()));
			emit readyRead();
		}
	}
}

void RawUdpIODevice::emitReadyRead()
{
	FInputMutex.lock();
	if (!FInputQueue.isEmpty())
		emit readyRead();
	FInputMutex.unlock();
}

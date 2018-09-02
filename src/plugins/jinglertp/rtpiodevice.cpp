#include "rtpiodevice.h"
#include <QUdpSocket>
#include <QThread>
#include <QBuffer>
#include <QTimer>
#include <QMutexLocker>

RtpIODevice::RtpIODevice(QIODevice *ARtp, QIODevice *ARtcp, QObject *AParent):
	QIODevice(AParent),
	FRtp(ARtp),
	FRtcp(ARtcp)
{
	qDebug() << "RtpIODevice::RtpIODevice(" << ARtp << "," << ARtcp << ")";
}

RtpIODevice::~RtpIODevice()
{
	qDebug() << "~RtpIODevice(); this=" << this;
}

bool RtpIODevice::isSequential() const
{
	return true;
}

qint64 RtpIODevice::bytesAvailable() const
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

bool RtpIODevice::waitForReadyRead(int msecs)
{
	if (bytesAvailable())
		return true;
	FInputMutex.lock();
	bool result = FInputWait.wait(&FInputMutex, msecs);
	FInputMutex.unlock();
	return result;
}

qint64 RtpIODevice::readData(char *data, qint64 maxlen)
{
//	qDebug() << "RtpIODevice::readData(" << (void *)data << "," << maxlen << ")";
	FInputMutex.lock();
	bool emitSignal(false);
	qint64 len = FInputQueue.isEmpty()?0:qMin(qint64(FInputQueue.first().size()), maxlen);
	if (len>0)
	{
		QBuffer buffer(&FInputQueue.first());
		buffer.open(ReadOnly);
		len = buffer.read(data, len);
		buffer.close();
		if (len < FInputQueue.first().size())
			FInputQueue.first().remove(0, int(len));
		else {
			FInputQueue.removeFirst();
			emitSignal = !FInputQueue.isEmpty();
		}
	}
	FInputMutex.unlock();
	if (emitSignal) {
		qDebug() << "A: emitting readyRead()";
		emit readyRead();
	}
//TODO: Check, conditions to return -1
	return len;
}

qint64 RtpIODevice::writeData(const char *data, qint64 len)
{
	qint64 sent = 0;

	if (len >= 2)	// Valid RTP or RTCP datagram
	{
		if (data[1] >= char(200) &&
			data[1] <= char(207)) // RTCP
			sent = FRtcp->write(data, len);
		else	// RTP
			sent = FRtp->write(data, len);
	}

	return sent;
}

void RtpIODevice::readDevice(QIODevice *device, qint64 bytes)
{
	QByteArray data(device->read(bytes));
	FInputMutex.lock();
	FInputQueue.enqueue(data);
	FInputMutex.unlock();
	FInputWait.wakeAll();
	qDebug() << "B: emitting readyRead()";
	emit readyRead();
}

void RtpIODevice::onReadyRead()
{
	QIODevice *device = qobject_cast<QIODevice *>(sender());
	Q_ASSERT(device);

	qint64 bytes = device->bytesAvailable();
	if (bytes)
		readDevice(device, bytes);
}

bool RtpIODevice::open(QIODevice::OpenMode mode)
{
	if (mode.testFlag(ReadOnly))
	{
		if (FRtp){
			connect(FRtp, SIGNAL(readyRead()), SLOT(onReadyRead()));
			qint64 bytes = FRtp->bytesAvailable();
			if (bytes)
				readDevice(FRtp, bytes);
		}
		if (FRtcp) {
			connect(FRtcp, SIGNAL(readyRead()), SLOT(onReadyRead()));
			qint64 bytes = FRtcp->bytesAvailable();
			if (bytes)
				readDevice(FRtcp, bytes);
		}
	}

	if (mode.testFlag(WriteOnly))
	{
		if (FRtp)
			connect(FRtp, SIGNAL(bytesWritten(qint64)),
						  SIGNAL(bytesWritten(qint64)));
		if (FRtcp)
			connect(FRtcp, SIGNAL(bytesWritten(qint64)),
						   SIGNAL(bytesWritten(qint64)));
	}

	return QIODevice::open(mode);
}

void RtpIODevice::close()
{
	QIODevice::close();

	if (FRtp) {
		FRtp->disconnect(SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));
		FRtp->disconnect(SIGNAL(readyRead()), this, SIGNAL(onReadyRead()));
	}
	if (FRtcp) {
		FRtcp->disconnect(SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));
		FRtcp->disconnect(SIGNAL(readyRead()), this, SIGNAL(onReadyRead()));
	}
}

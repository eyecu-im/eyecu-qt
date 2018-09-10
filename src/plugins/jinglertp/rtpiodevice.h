#ifndef RTPIODEVICE_H
#define RTPIODEVICE_H

#include <QIODevice>
#include <QQueue>
#include <QMutex>
#include <QTimer>
#include <QWaitCondition>

class RtpIODevice: public QIODevice
{
	Q_OBJECT

public:
	RtpIODevice(QIODevice *ARtp, QIODevice *ARtcp, QObject *AParent=nullptr);
	virtual ~RtpIODevice();

	// QIODevice interface
	virtual bool open(OpenMode mode) override;
	virtual void close() override;
	virtual bool isSequential() const override;
	virtual qint64 bytesAvailable() const override;
	virtual bool waitForReadyRead(int msecs) override;

protected:
	virtual qint64 readData(char *data, qint64 maxlen) override;
	virtual qint64 writeData(const char *data, qint64 len) override;

	void readDevice(QIODevice *device, qint64 bytes);

protected slots:
	void onReadyRead();

private:
	QIODevice *FRtp;
	QIODevice *FRtcp;

	QQueue<QByteArray> FInputQueue;
	mutable QMutex FInputMutex;
	mutable QWaitCondition FInputWait;
};

#endif // RTPIODEVICE_H

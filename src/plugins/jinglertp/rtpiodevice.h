#ifndef RTPIODEVICE_H
#define RTPIODEVICE_H

#include <QIODevice>
#include <QQueue>
#include <QMutex>
#include <QTimer>

class RtpIODevice: public QIODevice
{
	Q_OBJECT

public:
	RtpIODevice(QIODevice *ARtp, QIODevice *ARtcp, QObject *AParent=nullptr);
	virtual ~RtpIODevice() override;

	// QIODevice interface
	virtual bool open(OpenMode mode) override;
	virtual void close() override;
	virtual bool isSequential() const override;
	virtual qint64 bytesAvailable() const override;

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
//	QQueue<QByteArray> FOutputQueue;

	mutable QMutex FInputMutex;
//	mutable QMutex FOutputMutex;

//	QTimer FSendTimer;
//	QTimer FReadyReadTimer;
};

#endif // RTPIODEVICE_H

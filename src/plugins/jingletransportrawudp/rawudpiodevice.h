#ifndef RAWUDPIODEVICE_H
#define RAWUDPIODEVICE_H

#include <QUdpSocket>
#include <QMutex>
#include <QQueue>
#include <QThread>

//TODO: Maybe inherit QUdpSocket instead of QIODevice?
class RawUdpIODevice: public QIODevice
{
	Q_OBJECT

public:
	RawUdpIODevice(QUdpSocket *AInputSocket, QThread *AThread);

	~RawUdpIODevice();

	QUdpSocket *socket() const;

	void setTargetAddress(const QHostAddress &AHostAddress, quint16 APort);

	// QIODevice interface
	virtual bool open(OpenMode mode) override;
	virtual void close() override;
	virtual bool isSequential() const override;
	virtual qint64 bytesAvailable() const override;
	virtual qint64 bytesToWrite() const override;

protected:
	virtual qint64 readData(char *data, qint64 maxlen) override;
	virtual qint64 writeData(const char *data, qint64 len) override;

protected slots:
	void onWriteSocket();
	void onReadyRead();
	void emitReadyRead();

signals:
	void writeSocket();
	void updateSockets();

private:
	QHostAddress FTargetAddress;
	quint16		 FTargetPort;

	QUdpSocket *FSocket;

	QQueue<QByteArray> FInputQueue;
	QQueue<QByteArray> FOutputQueue;

	mutable QMutex	FInputMutex;
	mutable QMutex	FOutputMutex;
};

#endif // RAWUDPIODEVICE_H

#ifndef RAWUDPIODEVICE_H
#define RAWUDPIODEVICE_H

#include <QUdpSocket>
#include <QMutex>
#include <QQueue>

class RawUdpIODevice: public QIODevice
{
	Q_OBJECT

public:
	RawUdpIODevice(QUdpSocket *AInputSocket=nullptr,
				   QUdpSocket *AOutputSocket=nullptr,
				   QObject *AParent=nullptr);

	void setInputSocket(QUdpSocket *ASocket);
	QUdpSocket *inputSocket() const;

	void setOutputSocket(QUdpSocket *ASocket);
	QUdpSocket *outputSocket() const;

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

signals:
	void writeSocket();

private:
	QUdpSocket *FInputSocket;
	QUdpSocket *FOutputSocket;

	QQueue<QByteArray> FInputQueue;
	QQueue<QByteArray> FOutputQueue;

	mutable QMutex	FInputMutex;
	mutable QMutex	FOutputMutex;
};

#endif // RAWUDPIODEVICE_H

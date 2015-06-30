#ifndef SERIALREAD_H
#define SERIALREAD_H

#include <QByteArray>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "portsetings.h"

class QSerialPortInfo;
class QSerialPortPrivate;
class QSerialPort;

class SerialRead : public QObject
{
    Q_OBJECT
public:
    enum SerialReadError
    {
        PortTimeout = 17,
        PortInvalid,
        PortBusy,
        PortSetupFailed,
        PortOpenFailed
    };

    SerialRead(const ReaderSettings &AReaderSettings, const PortSettings &APortSettings = PortSettings(), QObject *AParent = 0);

public slots:
    void    closeSerialPort();
    void    openSerialPort();

protected:
    void    close();

protected slots:
    void onReadyRead();
    void onSerialPortError(QSerialPort::SerialPortError AError);
    void onTimeout();

signals:
    void portRead(const QByteArray &AData);
    void finished(int AErrorCode);

private:
    QSerialPort     *FSerialPort;
    QByteArray      FComPortData;
    QTimer          FTimer;
    int             FErrorCode;
    int             FTimeout;
    ReaderSettings  FReaderSettings;
    PortSettings    FPortSettings;
};

#endif // SERIALREAD_H

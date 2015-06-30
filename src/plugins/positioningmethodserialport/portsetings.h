#ifndef PORTSETTINGS_H
#define PORTSETTINGS_H

#include <QString>
#include <QSerialPort>

struct ReaderSettings
{
    ReaderSettings():
        FSize(0),FTimeout(0),FTimeTreshold(0),FName(QString())
    {}

    ReaderSettings(int ALength, int ATimeout, int ATimeInterval, const QString &APortName):
        FSize(ALength),FTimeout(ATimeout),FTimeTreshold(ATimeInterval),FName(APortName)
    {}

    int     FSize;              // Buffer size
    int     FTimeout;           // Port read timeout
    int     FTimeTreshold;      // Minimum period between position updates
    float   FDistanceTreshold;  // Minimum distance change for position update
    QString FName;              // Port name
};

struct PortSettings
{
    PortSettings():
        FBaudRate(QSerialPort::UnknownBaud),
        FDataBits(QSerialPort::UnknownDataBits),
        FStopBits(QSerialPort::UnknownStopBits),
        FParity(QSerialPort::UnknownParity),
        FFlowControl(QSerialPort::UnknownFlowControl)
    {}

    PortSettings(QSerialPort::BaudRate ABaudRate, QSerialPort::DataBits ADataBits, QSerialPort::StopBits AStopBits, QSerialPort::Parity AParity, QSerialPort::FlowControl AFlowControl):
        FBaudRate(ABaudRate),FDataBits(ADataBits),FStopBits(AStopBits),FParity(AParity),FFlowControl(AFlowControl)
    {}

    qint32                  FBaudRate;      // Baud rate
    QSerialPort::DataBits   FDataBits;      // 5,6,7,8
    QSerialPort::StopBits   FStopBits;      // 1, 1.5, 2
    QSerialPort::Parity     FParity;        // None,Even,Odd,Mark,Space
    QSerialPort::FlowControl FFlowControl;  // None, RTS/CTS, XON/XOFF
};

#endif // PORTSETTINGS_H

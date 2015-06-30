#include "serialread.h"

SerialRead::SerialRead(const ReaderSettings &AReaderSettings, const PortSettings &APortSettings, QObject *AParent)
    :QObject(AParent)
    ,FSerialPort(NULL)
    ,FErrorCode(QSerialPort::NoError)
    ,FTimeout(5)
    ,FReaderSettings(AReaderSettings)
    ,FPortSettings(APortSettings)
{
    connect(&FTimer,SIGNAL(timeout()),SLOT(onTimeout()));
}

void SerialRead::closeSerialPort()
{
    FErrorCode = QSerialPort::NoError;
    close();
}

void SerialRead::openSerialPort()
{
    QSerialPortInfo info(FReaderSettings.FName);
    if(!info.isValid())
        FErrorCode = PortInvalid;
    else
        if(info.isBusy())
            FErrorCode = PortBusy;
        else
        {
            FSerialPort = new QSerialPort(FReaderSettings.FName);
            if (FSerialPort->open(QIODevice::ReadOnly))
            {
                if (FSerialPort->setBaudRate(FPortSettings.FBaudRate)
                    && FSerialPort->setDataBits(FPortSettings.FDataBits)
                    && FSerialPort->setStopBits(FPortSettings.FStopBits)
                    && FSerialPort->setParity(FPortSettings.FParity)
                    && FSerialPort->setFlowControl(FPortSettings.FFlowControl))
                {
                    connect(FSerialPort,SIGNAL(error(QSerialPort::SerialPortError)),SLOT(onSerialPortError(QSerialPort::SerialPortError)));
                    connect(FSerialPort,SIGNAL(readyRead()),SLOT(onReadyRead()));
                    FTimer.start(1000*FTimeout);
                }
                else
                    FErrorCode = PortSetupFailed;
            }
            else
                FErrorCode = PortOpenFailed;
        }
    if(FErrorCode)
        close();
}

void SerialRead::onReadyRead()
{
    FTimer.start(1000*FTimeout);
    FComPortData.append(FSerialPort->readAll());
    if(FComPortData.size() > FReaderSettings.FSize)
    {
        emit portRead(FComPortData);
        FComPortData.clear();
    }
}

void SerialRead::onSerialPortError(QSerialPort::SerialPortError AError)
{
    FErrorCode = AError;
    switch (AError)
    {
        case QSerialPort::NoError:              break;  // No error occurred
        case QSerialPort::DeviceNotFoundError:  close(); break;  //"No such file or directory"
        case QSerialPort::PermissionError:      break;
        case QSerialPort::OpenError:            close(); break;
        case QSerialPort::ParityError:          close(); break;
        case QSerialPort::FramingError:         break;
        case QSerialPort::BreakConditionError:  break;
        case QSerialPort::WriteError:           break;
        case QSerialPort::ReadError:            close(); break;
        case QSerialPort::ResourceError:        close(); break;
        case QSerialPort::UnsupportedOperationError:  break;
        case QSerialPort::UnknownError:         close(); break; //"Unknown error"
        case QSerialPort::TimeoutError:         close(); break; //"The semaphore timeout period has expired."
        case QSerialPort::NotOpenError:         close(); break;
        default:   break;
    }
}

void SerialRead::onTimeout()
{
    FErrorCode = PortTimeout;
    close();
}

void SerialRead::close()
{
    if (FTimer.isActive())
        FTimer.stop();
    if(FSerialPort)
    {
        if (FSerialPort->isOpen())
            FSerialPort->close();
        disconnect(FSerialPort,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
        disconnect(FSerialPort,SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onSerialPortError(QSerialPort::SerialPortError)));
        FSerialPort->deleteLater();
        FSerialPort = NULL;
    }
    emit finished(FErrorCode);
}

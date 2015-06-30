#include <QTimer>
#include <QSerialPortInfo>
#include "portinfo.h"

void PortInfo::run()
{
    QTimer::singleShot(0, this, SLOT(checkPort()));
    exec();
}

void PortInfo::checkPort()
{
    QMap<QString,QString> serialPortInfo;
    QList<QSerialPortInfo> availablePorts(QSerialPortInfo::availablePorts());
    for (QList<QSerialPortInfo>::ConstIterator it = availablePorts.constBegin(); it != availablePorts.constEnd(); it++)
    {
        QString port = QString("<table><tbody>"
                       "<tr><td>%1: </td><td>%2</td></tr>"
                       "<tr><td>%3: </td><td>%4</td></tr>"
                       "<tr><td>%5: </td><td>%6</td></tr>"
                       "<tr><td>%7: </td><td>%8</td></tr>"
                       "<tr><td>%9: </td><td>%10</td></tr>"
                       "<tr><td>%11: </td><td>%12</td></tr>"
                       "<tr><td>%13: </td><td>%14</td></tr>"
                       "</tbody></table>")
        .arg(tr("Information about port")).arg((*it).portName())
        .arg(tr("Location")).arg((*it).systemLocation())
        .arg(tr("Description")).arg((*it).description())
        .arg(tr("Manufacturer")).arg((*it).manufacturer())
        .arg(tr("Product identifier")).arg((*it).productIdentifier() ? QString::number((*it).productIdentifier(), 16): QString())
        .arg(tr("Vendor identifier")).arg((*it).vendorIdentifier() ? QString::number((*it).vendorIdentifier(), 16): QString())
        .arg(tr("Status")).arg((*it).isBusy() ? tr("Busy") : tr("Free"));
        serialPortInfo.insert((*it).portName(), port);
    }
    emit portsFound(serialPortInfo);
    exit(0);
}

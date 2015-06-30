#ifndef PORTINFO_H
#define PORTINFO_H

#include <QThread>
#include <QMap>

class PortInfo : public QThread
{
    Q_OBJECT

protected:
    void run();

protected slots:
    void checkPort();

signals:
    void portsFound(const QMap<QString,QString> &APortInfo);
};

#endif // PORTINFO_H

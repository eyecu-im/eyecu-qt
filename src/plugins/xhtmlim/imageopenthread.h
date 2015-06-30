#ifndef BOBURLOPENTHREAD_H
#define BOBURLOPENTHREAD_H

#include <QThread>
#include <QUrl>
#include "interfaces/ibitsofbinary.h"

class ImageOpenThread : public QThread
{
    Q_OBJECT
public:
    ImageOpenThread(const QUrl &AUrl, QObject *parent = 0);
    void run();

private:
    const QUrl  FUrl;
};

#endif // BOBURLOPENTHREAD_H

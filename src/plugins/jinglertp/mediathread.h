#ifndef MEDIATHREAD_H
#define MEDIATHREAD_H

#include <QThread>
#include <QIODevice>
#include <QTimer>

class MediaThread : public QThread
{
    Q_OBJECT
public:
    MediaThread(QIODevice *AInputDevice, QIODevice *AOutputDevice, int AComponent, QObject *AParent = 0);
    ~MediaThread();

protected:
    void run();

protected slots:
    void onTimeout();
    void onReadyRead();

private:
    QIODevice *FInputDevice;
    QIODevice *FOutputDevice;
    QTimer    *FTimer;
    int       FCounter;
    int       FComponent;
};

#endif // MEDIATHREAD_H

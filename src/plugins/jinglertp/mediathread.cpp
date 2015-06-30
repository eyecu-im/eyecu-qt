#include <QDebug>
#include "mediathread.h"

MediaThread::MediaThread(QIODevice *AInputDevice, QIODevice *AOutputDevice, int AComponent, QObject *AParent) :
    QThread(AParent), FInputDevice(AInputDevice), FOutputDevice(AOutputDevice), FCounter(10), FComponent(AComponent)
{
    qDebug() << "MediaThread(" << AInputDevice << "," << AOutputDevice << "," << AComponent << ")";
    connect(this,SIGNAL(finished()),SLOT(deleteLater()));
    start();
}

MediaThread::~MediaThread()
{}

void MediaThread::run()
{
    FTimer = new QTimer();
    connect(FTimer,SIGNAL(timeout()),SLOT(onTimeout()));
    connect(FInputDevice,SIGNAL(readyRead()),SLOT(onReadyRead()));
    connect(FInputDevice,SIGNAL(aboutToClose()),SLOT(quit()));
    connect(FOutputDevice,SIGNAL(aboutToClose()),SLOT(quit()));
    FTimer->start(500);
    exec();
    FTimer->stop();
    FTimer->deleteLater();
}

void MediaThread::onTimeout()
{
    if (FCounter--)
		FOutputDevice->write(QString("Test: %1").arg(FComponent).toLatin1().data());
    else
        exit();
}

void MediaThread::onReadyRead()
{
    QByteArray array = FInputDevice->readAll();
    if (array.isEmpty())
        qWarning() << "read error:" << FInputDevice->errorString();
    else
    {
        QString read = QString::fromUtf8(array);
        qDebug() << "read: " << read;
    }
}

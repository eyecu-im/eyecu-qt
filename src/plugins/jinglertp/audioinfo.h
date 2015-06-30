#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#include <QAudioDeviceInfo>
#include <QtCore/qendian.h>
#include <QAudioInput>

class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &AAudioFormat, QObject *AParent=0);
    ~AudioInfo();
    void start();
    void stop();
    qint64 readData(char *AData, qint64 AMaxlen);
    qint64 writeData(const char *AData, qint64 ALen);
private:
    void parser();
    const QAudioFormat FAudioFormat;
    quint16 FMaxAmplitude;
    qreal FVolume; // 0.0 <= curLevel <= 1.0
signals:
    void update(qreal ALevel);
};

#endif //AUDIOINFO_H


#include <QDebug>
#include "audioinfo.h"

#if QT_VERSION < 0x050000
#define channelCount channels
#endif

AudioInfo::AudioInfo(const QAudioFormat &AAudioFormat, QObject *AParent)
    : QIODevice(AParent)
    , FAudioFormat(AAudioFormat)
    , FMaxAmplitude(0)
    , FVolume(0.0)
{
    parser();
}

AudioInfo::~AudioInfo()
{
}

void AudioInfo::parser()
{
    switch (FAudioFormat.sampleSize()) {
    case 8:
        switch (FAudioFormat.sampleType()) {
        case QAudioFormat::UnSignedInt:
            FMaxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            FMaxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (FAudioFormat.sampleType()) {
        case QAudioFormat::UnSignedInt:
            FMaxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            FMaxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void AudioInfo::start() {open(QIODevice::WriteOnly);}
void AudioInfo::stop() {close();}

qint64 AudioInfo::readData(char *AData, qint64 AMaxlen)
{
    Q_UNUSED(AData)
    Q_UNUSED(AMaxlen)
    return 0;
}

qint64 AudioInfo::writeData(const char *AData, qint64 ALen)
{
    if (FMaxAmplitude) {
        Q_ASSERT(FAudioFormat.sampleSize() % 8 == 0);
        const int channelBytes = FAudioFormat.sampleSize() / 8;
        const int sampleBytes = FAudioFormat.channelCount() * channelBytes;
        Q_ASSERT(ALen % sampleBytes == 0);
        const int numSamples = ALen / sampleBytes;

        quint16 maxValue = 0;
        const unsigned char *ptr = reinterpret_cast<const unsigned char *>(AData);

        for (int i = 0; i < numSamples; ++i) {
            for(int j = 0; j < FAudioFormat.channelCount(); ++j) {
                quint16 value = 0;

                if (FAudioFormat.sampleSize() == 8 && FAudioFormat.sampleType() == QAudioFormat::UnSignedInt) {
                    value = *reinterpret_cast<const quint8*>(ptr);
                } else if (FAudioFormat.sampleSize() == 8 && FAudioFormat.sampleType() == QAudioFormat::SignedInt) {
                    value = qAbs(*reinterpret_cast<const qint8*>(ptr));
                } else if (FAudioFormat.sampleSize() == 16 && FAudioFormat.sampleType() == QAudioFormat::UnSignedInt) {
                    if (FAudioFormat.byteOrder() == QAudioFormat::LittleEndian)
                        value = qFromLittleEndian<quint16>(ptr);
                    else
                        value = qFromBigEndian<quint16>(ptr);
                } else if (FAudioFormat.sampleSize() == 16 && FAudioFormat.sampleType() == QAudioFormat::SignedInt) {
                    if (FAudioFormat.byteOrder() == QAudioFormat::LittleEndian)
                        value = qAbs(qFromLittleEndian<qint16>(ptr));
                    else
                        value = qAbs(qFromBigEndian<qint16>(ptr));
                }

                maxValue = qMax(value, maxValue);
                ptr += channelBytes;
            }
        }
        maxValue = qMin(maxValue, FMaxAmplitude);
        FVolume = qreal(maxValue) / FMaxAmplitude;
    }
    emit update(FVolume);
    return ALen;
}

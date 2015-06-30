#ifndef TUNELISTENERMPRIS_H
#define TUNELISTENERMPRIS_H

#include <QDBusConnection>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QVariantMap>
#include <QStringList>

#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>

#define TUNELISTENERMPRIS_UUID "{946bdca3-b4ca-87de-c687-07ba1eda610c}"

struct PlayerStatus
{
    int playStatus;
    int playOrder;
    int playRepeat;
    int stopOnce;
};

Q_DECLARE_METATYPE(PlayerStatus)

class TuneListenerMpris: public QObject,
            public IPlugin,
            public ITuneListener
{
    Q_OBJECT
	Q_INTERFACES(IPlugin ITuneListener)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITuneListenerMpris")
#endif
    static const char *MPRIS_PREFIX;
    static const QString busName;

    enum MPRISVersion
    {
        MPRIS_1 = 1,
        MPRIS_2 = 2
    };

    enum PlayStatus
    {
        StatusPlaying = 0,
        StatusPaused = 1,
        StatusStopped = 2
    };

public:
    TuneListenerMpris();
    ~TuneListenerMpris();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return TUNELISTENERMPRIS_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //ITuneListener
    virtual TuneData currentTune() const;
    virtual bool isPollingType() const {return false;}

protected slots:
    void checkMprisService(const QString &AName, const QString &AOldOwner, const QString &ANewOwner);
    void onTrackChange(const QVariantMap &AMap);
    void onPlayerStatusChange(const PlayerStatus &APlayerStatus);
    void onPropertyChange(const QDBusMessage &AMsg);

protected:
    TuneData getTune(const QVariantMap &AMap) const;
    TuneData getMpris2Tune(const QVariantMap &AMap) const;
    int getMpris2Status(const QString &AStatus) const;
    int version(const QString &AService) const;
    void connectToBus(const QString &AService);
    void disconnectFromBus(const QString &AService);

signals:
    void playing(const TuneData &ATuneData);
    void stopped();

private:
    TuneData FCurrentTune;
    QStringList FPlayers;
    bool FTuneSent;
};

#endif // TUNELISTENERMPRIS_H

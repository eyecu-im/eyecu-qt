#include <QDebug>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusConnectionInterface>
#include "tunelistenermpris.h"

/**
 * \class TuneListenerMpris
 * \brief A controller for MPRIS-compatible players.
 */

const char *TuneListenerMpris::MPRIS_PREFIX = "org.mpris";
const QString TuneListenerMpris::busName = "SessionBus";

QDBusArgument &operator<<(QDBusArgument& arg, const PlayerStatus& ps)
{
    arg.beginStructure();
    arg << ps.playStatus;
    arg << ps.playOrder;
    arg << ps.playRepeat;
    arg << ps.stopOnce;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument& arg, PlayerStatus& ps)
{
    arg.beginStructure();
    arg >> ps.playStatus;
    arg >> ps.playOrder;
    arg >> ps.playRepeat;
    arg >> ps.stopOnce;
    arg.endStructure();
    return arg;
}

TuneListenerMpris::TuneListenerMpris(): FTuneSent(false)
{
    qDBusRegisterMetaType<PlayerStatus>();
    QDBusConnection bus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, busName);
    FPlayers = bus.interface()->registeredServiceNames().value().filter(MPRIS_PREFIX);
    for(QStringList::const_iterator it=FPlayers.constBegin(); it!=FPlayers.constEnd(); it++)
        connectToBus(*it);
    bus.connect(QLatin1String("org.freedesktop.DBus"),
            QLatin1String("/org/freedesktop/DBus"),
            QLatin1String("org.freedesktop.DBus"),
            QLatin1String("NameOwnerChanged"),
            this,
            SLOT(checkMprisService(QString, QString, QString)));
}

TuneListenerMpris::~TuneListenerMpris()
{
    for(QStringList::const_iterator it=FPlayers.constBegin(); it!=FPlayers.constEnd(); it++)
        disconnectFromBus(*it);
    QDBusConnection(busName).disconnect(QLatin1String("org.freedesktop.DBus"),
                        QLatin1String("/org/freedesktop/DBus"),
                        QLatin1String("org.freedesktop.DBus"),
                        QLatin1String("NameOwnerChanged"),
                        this,
                        SLOT(checkMprisService(QString, QString, QString)));
    QDBusConnection::disconnectFromBus(busName);
}

void TuneListenerMpris::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Tune Listener MPRIS");
    APluginInfo->description = tr("Allow User Tune plugin to obtain currently playing tune information from MPRIS-compatible players");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(TUNE_UUID);
}

bool TuneListenerMpris::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(APluginManager)
    Q_UNUSED(AInitOrder)
    return true; //FMessageWidgets!=NULL
}

bool TuneListenerMpris::initObjects()
{
    return true;
}

bool TuneListenerMpris::initSettings()
{    
    return true;
}

// Methods to be implemented
void TuneListenerMpris::checkMprisService(const QString &AName, const QString &AOldOwner, const QString &ANewOwner)
{
    Q_UNUSED(AOldOwner);
    if (AName.startsWith(MPRIS_PREFIX))
    {
        int playerIndex = FPlayers.indexOf(AName);
        if (playerIndex == -1)
        {
            if (!ANewOwner.isEmpty())
            {
                FPlayers.append(AName);
                connectToBus(AName);
            }
        }
        else if (ANewOwner.isEmpty())
        {
            disconnectFromBus(AName);
            FPlayers.removeAt(playerIndex);
        }
    }
}

int TuneListenerMpris::version(const QString &AService) const
{
    return AService.contains("MediaPlayer2") ? MPRIS_2 : MPRIS_1;
}

void TuneListenerMpris::connectToBus(const QString &AService)
{
    QDBusConnection bus = QDBusConnection(busName);
    if (version(AService) != MPRIS_2)
    {
        bus.connect(AService,
                QLatin1String("/Player"),
                QLatin1String("org.freedesktop.MediaPlayer"),
                QLatin1String("StatusChange"),
                QLatin1String("(iiii)"),
                this,
                SLOT(onPlayerStatusChange(PlayerStatus)));
        bus.connect(AService,
                QLatin1String("/Player"),
                QLatin1String("org.freedesktop.MediaPlayer"),
                QLatin1String("TrackChange"),
                QLatin1String("a{sv}"),
                this,
                SLOT(onTrackChange(QVariantMap)));
    }
    else
    {
        bus.connect(AService,
                QLatin1String("/org/mpris/MediaPlayer2"),
                QLatin1String("org.freedesktop.DBus.Properties"),
                QLatin1String("PropertiesChanged"),
                this,
                SLOT(onPropertyChange(QDBusMessage)));
    }
}

void TuneListenerMpris::disconnectFromBus(const QString &AService)
{
    QDBusConnection bus = QDBusConnection(busName);
    if (version(AService) != MPRIS_2)
    {
        bus.disconnect(AService,
                   QLatin1String("/Player"),
                   QLatin1String("org.freedesktop.MediaPlayer"),
                   QLatin1String("StatusChange"),
                   QLatin1String("(iiii)"),
                   this,
                   SLOT(onPlayerStatusChange(PlayerStatus)));
        bus.disconnect(AService,
                   QLatin1String("/Player"),
                   QLatin1String("org.freedesktop.MediaPlayer"),
                   QLatin1String("TrackChange"),
                   QLatin1String("a{sv}"),
                   this,
                   SLOT(onTrackChange(QVariantMap)));
    }
    else
    {
        bus.disconnect(AService,
                   QLatin1String("/org/mpris/MediaPlayer2"),
                   QLatin1String("org.freedesktop.DBus.Properties"),
                   QLatin1String("PropertiesChanged"),
                   this,
                   SLOT(onPropertyChange(QDBusMessage)));
    }
    if (!FCurrentTune.isNull())
    {
        emit stopped();
        FTuneSent = false;
        FCurrentTune = TuneData();
    }
}

void TuneListenerMpris::onPlayerStatusChange(const PlayerStatus &APlayerStatus)
{
    if (!FCurrentTune.isNull())
    {
        if (APlayerStatus.playStatus != StatusPlaying)
        {
            emit stopped();
            FTuneSent = false;
            if (APlayerStatus.playStatus == StatusStopped)
                FCurrentTune.clear();
        }
        else if (!FTuneSent)
        {
            emit playing(FCurrentTune);
            FTuneSent = true;
        }
    }
}

void TuneListenerMpris::onTrackChange(const QVariantMap &AMap)
{
    TuneData tune = getTune(AMap);
    if (tune != FCurrentTune && !tune.isEmpty())
    {
        FCurrentTune = tune;
        emit playing(FCurrentTune);
        FTuneSent = true;
    }
}

void TuneListenerMpris::onPropertyChange(const QDBusMessage &AMsg)
{
    QDBusArgument arg = AMsg.arguments().at(1).value<QDBusArgument>();
    QVariantMap map = qdbus_cast<QVariantMap>(arg);
    QVariant v = map.value(QLatin1String("Metadata"));
    if (v.isValid())
    {
        arg = v.value<QDBusArgument>();
        TuneData tune = getMpris2Tune(qdbus_cast<QVariantMap>(arg));
        if (tune != FCurrentTune && !tune.isEmpty())
        {
            FCurrentTune = tune;
            emit playing(FCurrentTune);
            FTuneSent = true;
        }
    }
    v = map.value(QLatin1String("PlaybackStatus"));
    if (v.isValid())
    {
        PlayerStatus status;
        status.playStatus = getMpris2Status(v.toString());
        onPlayerStatusChange(status);
    }
}

int TuneListenerMpris::getMpris2Status(const QString &AStatus) const
{
    if (AStatus == QLatin1String("Playing"))
        return StatusPlaying;
    else if (AStatus == QLatin1String("Paused"))
        return StatusPaused;
    return StatusStopped;
}

TuneData TuneListenerMpris::currentTune() const
{
    return FCurrentTune;
}

TuneData TuneListenerMpris::getTune(const QVariantMap &AMap) const
{
    TuneData tune;
    tune.title = AMap.value("title").toString();
    tune.artist = AMap.value("artist").toString();
    tune.source = AMap.value("album").toString();
    tune.track = AMap.value("track").toString();
    tune.length = AMap.value("time").toUInt();
    return tune;
}

TuneData TuneListenerMpris::getMpris2Tune(const QVariantMap &AMap) const
{
    TuneData tune;
    tune.title = AMap.value("xesam:title").toString();
    tune.artist = AMap.value("xesam:artist").toString();
    tune.source = AMap.value("xesam:album").toString();
    tune.track = QVariant(AMap.value("xesam:trackNumber").toUInt()).toString();
    tune.length = QVariant(AMap.value("mpris:length").toLongLong() / 1000000).toUInt();
    return tune;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_tunelistenermpris, TuneListenerMpris)
#endif

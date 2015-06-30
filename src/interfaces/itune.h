#ifndef ITUNE_H
#define ITUNE_H

#include <QObject>
#include <QIcon>
#include <QHash>
#include <QUrl>
#include <QNetworkAccessManager>

#define TUNE_UUID "{639EADAA-A684-42e4-A9AD-28FC9BCB8F7C}"

class TuneData
{
public:
    QString artist;
    quint16 length;
    quint8  rating;
    QString source;
    QString title;
    QString track;
    QUrl    uri;

    TuneData(): length(0), rating(0) {}

    bool operator == (const TuneData &AOther) const {return
                rating == AOther.rating && length == AOther.length &&
                artist == AOther.artist && title  == AOther.title &&
                source == AOther.source && track  == AOther.track && uri == AOther.uri;}
    bool operator != (const TuneData &AOther) const {return !operator ==(AOther);}
    bool isNull() const {return
                (rating == 0) && (length == 0) &&
                (artist.isNull()) && (title.isNull()) &&
                (source.isNull()) && (track.isNull()) && (uri.isEmpty());}
    bool isEmpty() const { return
                (rating == 0) && (length == 0) &&
                (artist.isEmpty()) && (title.isEmpty()) &&
                (source.isEmpty()) && (track.isEmpty()) && uri.isEmpty();}
    void clear() {artist = QString(); title = QString();
                  source = QString(); track = QString();
                  rating = 0; length = 0; uri = QUrl();}
};

class ITuneListener
{
public:
    virtual QObject *instance() = 0;
    virtual TuneData currentTune() const = 0;
    virtual bool isPollingType() const = 0;

protected:
	/**
	 * This signal is emitted when the media player started playing a tune.
	 * \param ATuneData currently playing tune data
	 */
    virtual void playing(const TuneData &ATuneData) = 0;

	/**
	 * This signal is emitted when the media player stopped playing tunes.
	 */
    virtual void stopped() = 0;
};

class ITuneInfoRequester
{
public:
    virtual QObject *instance() = 0;
    virtual bool    requestTuneInfo(QNetworkAccessManager *ANetworkAccessManager, const QString &AArtist, const QString &AAlbum = QString(), const QString &ATrack = QString()) = 0;
    virtual QString serviceName() const = 0;
    virtual QIcon   serviceIcon() const = 0;

protected:
    virtual void tuneInfoReceived(const QString &AArtist, const QString &AAlbum, const QString &ATrack, const QHash<QString, QString> &ATuneInfo) = 0;
};

class ITune
{
public:
    virtual QObject *instance() = 0;
    virtual QIcon getIcon() const = 0;
    virtual QString getIconFileName() const = 0;
};

Q_DECLARE_INTERFACE(ITuneListener, "RWS.Plugin.ITuneListener/1.0")
Q_DECLARE_INTERFACE(ITuneInfoRequester, "RWS.Plugin.ITuneInfoRequester/1.0")
Q_DECLARE_INTERFACE(ITune, "RWS.Plugin.ITune/1.0")

#endif	//ITUNE_H

#ifndef TUNEDATAREQUESTERLASTFM_H
#define TUNEINFOREQUESTERLASTFM_H

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/itune.h>
#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>
#include <utils/iconstorage.h>

#define TUNEINFOREQUESTERLASTFM_UUID "{5c9fdac3-bc82-74da-f6d0-173bdcab81ac}"

class TuneInfoRequesterLastFm : public QObject,
                                public IPlugin,
								public IOptionsDialogHolder,
                                public ITuneInfoRequester
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IOptionsDialogHolder ITuneInfoRequester)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.ITuneInfoRequesterLastFm")
#endif
public:
    enum ImageSize
    {
        Small,
        Medium,
        Large,
        ExtraLarge,
        Mega
    };

    TuneInfoRequesterLastFm(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid   pluginUuid() const { return TUNEINFOREQUESTERLASTFM_UUID; }
    virtual void    pluginInfo(IPluginInfo *APluginInfo);
    virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool    initObjects();
    virtual bool    initSettings();
    virtual bool    startPlugin(){return true;}

    //IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //ITuneDataRequester
    virtual bool requestTuneInfo(QNetworkAccessManager *ANetworkAccessManager, const QString &AArtist, const QString &AAlbum = QString(), const QString &ATrack = QString());
    virtual QHash<QString, QString> parseResult(const QByteArray &AResult) const;
    virtual QString serviceName() const {return tr("last.fm");}
	virtual QIcon   serviceIcon() const {return IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_TUNE_REQUESTER_LASTFM);}

signals:
    void tuneInfoReceived(const QString &AArtist, const QString &AAlbum, const QString &ATrack, const QHash<QString, QString> &ATuneInfo);

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AArtist, const QString &AAlbum, const QString &ATrack);

private:
    IOptionsManager *FOptionsManager;

    static const QString api_key;
    static const QStringList image_sizes;
};

#endif // TUNEINFOREQUESTERLASTFM_H

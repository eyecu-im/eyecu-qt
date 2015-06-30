#ifndef MAPSEARCHPROVIDER2GIS_H
#define MAPSEARCHPROVIDER2GIS_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imapsearch.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipoi.h>

#include <utils/options.h>

#define MAPSEARCHPROVIDER2GIS_UUID "{e7b3602a-46ca-df21-0b1c-af531649ca20}"

class MapSearchProvider2gis : public QObject,
                  public IPlugin,
                  public IMapSearchProvider,
				  public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IMapSearchProvider IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSearchProvider2gis")
#endif
public:
    MapSearchProvider2gis(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid   pluginUuid() const { return MAPSEARCHPROVIDER2GIS_UUID; }
    virtual void    pluginInfo(IPluginInfo *APluginInfo);
    virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool    initObjects();
    virtual bool    initSettings();
    virtual bool    startPlugin(){return true;}

    //IMapSearchProvider
    virtual void setHttpRequester(HttpRequester *AHttpRequester) {FHttpRequester=AHttpRequester;}
    virtual bool startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore);
    virtual QString sourceName() const;
    virtual QIcon   sourceIcon() const;
    virtual int     features() const;
    virtual void    getPageValues(int *AMin, int *AMax, int *ADefault) const;

    // IOptionsHolder interface
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

protected:
    QUrl searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int ALimit);
    QUrl searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int APageSize, int AMore);
    void parseResult(QByteArray ASearchResult);

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
    void receivedPoi(const GeolocElement &APoi);
    void searchFinished(bool AMoreResultsAvailable);

private:
	HttpRequester	*FHttpRequester;
	IOptionsManager	*FOptionsManager;
	int				FLastResultFound;
};

#endif // MAPSEARCHPROVIDER2GIS_H

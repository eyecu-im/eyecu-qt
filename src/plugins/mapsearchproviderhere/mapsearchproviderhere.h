#ifndef MAPSEARCHPROVIDERHERE_H
#define MAPSEARCHPROVIDERHERE_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/imapsearch.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipoi.h>

#include <utils/options.h>

#define MAPSEARCHPROVIDERHERE_UUID "{2d94fc7a-b203-15b5-f9a8-247b8df30ab5}"

class MapSearchProviderHere:
		public QObject,
		public IPlugin,
		public IMapSearchProvider,
		public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMapSearchProvider IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSearchProviderHere")
#endif
public:
    MapSearchProviderHere(QObject *parent = 0);
    //IPlugin
	virtual QObject *instance() override { return this; }
	virtual QUuid   pluginUuid() const override { return MAPSEARCHPROVIDERHERE_UUID; }
	virtual void    pluginInfo(IPluginInfo *APluginInfo) override;
	virtual bool    initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	virtual bool    initObjects() override;
	virtual bool    initSettings() override;
	virtual bool    startPlugin() override {return true;}

    //IMapSearchProvider
	virtual void	setHttpRequester(HttpRequester *AHttpRequester) override {FHttpRequester=AHttpRequester;}
	virtual bool	startSearch(const QString &ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, int AZoom, bool ALimitRange, int AMaxResults, bool AMore) override;
	virtual QString	sourceName() const override;
	virtual QIcon	sourceIcon() const override;
	virtual int		features() const override {return FeatureLimitRange|FeatureMaxResults|FeatureNextPage;}
	virtual void	getPageValues(int *AMin, int *AMax, int *ADefault) const override;

	// IOptionsDialogHolder interface
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) override;

	QString country(QLocale::Country aCountry);
	QString country(const QLocale &aLocale);
	QList<QLocale::Country> countries();

protected:
    QUrl searchRequest(QString ASearchString, qreal ALatSouth, qreal ALngWest, qreal ALatNorth, qreal ALngEast, bool ALimitRange, int AMaxResults, bool AMore);
    void parseResult(QByteArray ASearchResult);
    void fillCountryCodes();

	QString lang(const QString &aLocale);
	QString lang(const QLocale &aLocale=QLocale());

protected slots:
    void onResultReceived(const QByteArray &AResult, const QString &AId);

signals:
	void receivedPoi(const GeolocElement &APoi);
	void searchFinished(bool AMoreResultsAvailable);

private:
	IOptionsManager	*FOptionsManager;
	IPoi			*FPoi;
	HttpRequester	*FHttpRequester;
    QHash<QString, QString> FCountryCodes;
	int				FNextPageInformation;

	QHash<QString,QString> fLocales;
	QMap<QLocale::Country,QString> fCountries;
};

#endif // MAPSEARCHPROVIDERHERE_H

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QDir>
#include <QHash>
#include <QPointer>
#include <QTranslator>
#include <QDomDocument>
#include <QApplication>
#include <QPluginLoader>
#include <QSessionManager>
#include <QSplashScreen>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/itraymanager.h>
#include "setuppluginsdialog.h"
#include "aboutbox.h"

enum ShutdownKind {
	SK_WORK,
	SK_QUIT,
	SK_RESTART
};

struct PluginItem {
	IPlugin *plugin;
	IPluginInfo *info;
	QPluginLoader *loader;
	QTranslator *translator;
};

class PluginManager :
	public QObject,
	public IPluginManager
{
	Q_OBJECT;
	Q_INTERFACES(IPluginManager);
public:
	PluginManager(QApplication *AParent);
	~PluginManager();
	virtual QObject *instance() { return this; }
	virtual QString version() const;
	virtual QString revision() const;
	virtual QDateTime revisionDate() const;
	virtual bool isShutingDown() const;
	virtual QString homePath() const;
	virtual void setHomePath(const QString &APath);
	virtual void setLocale(QLocale::Language ALanguage, QLocale::Country ACountry);
	virtual IPlugin* pluginInstance(const QUuid &AUuid) const;
	virtual QList<IPlugin *> pluginInterface(const QString &AInterface = QString()) const;
	virtual const IPluginInfo *pluginInfo(const QUuid &AUuid) const;
	virtual QSet<QUuid> pluginDependencesOn(const QUuid &AUuid) const;
	virtual QSet<QUuid> pluginDependencesFor(const QUuid &AUuid) const;
public slots:
	virtual void quit();
	virtual void restart();
	virtual void delayShutdown();
	virtual void continueShutdown();
signals:
	void aboutToQuit();
	void shutdownStarted();
// *** <<< eyeCU <<< ***
	void showSplash();
	void splashMessage(const QString &AMessage);
	void closeSplash(QWidget *AMainWindow = NULL);
// *** >>> eyeCU >>> ***
protected:
	void loadSettings();
	void saveSettings();
	bool loadPlugins();
	bool initPlugins();
	bool startPlugins();
	void unloadPlugins();
protected:
	void startClose();
	void finishClose();
	void startQuit();
	void finishQuit();
	void closeAndQuit();
	void closeTopLevelWidgets();
protected:
	void removePluginItem(const QUuid &AUuid, const QString &AError);
	void unloadPlugin(const QUuid &AUuid, const QString &AError = QString());
	bool checkDependences(const QUuid &AUuid) const;
	bool checkConflicts(const QUuid &AUuid) const;
	QList<QUuid> getConflicts(const QUuid &AUuid) const;
	void loadCoreTranslations(const QDir &ADir, const QString &ALocaleName);
protected:
	bool isPluginEnabled(const QString &AFile) const;
	QDomElement savePluginInfo(const QString &AFile, const IPluginInfo *AInfo);
	void savePluginError(const QString &AFile, const QString &AEror);
	void removePluginsInfo(const QStringList &ACurFiles);
	void createMenuActions();
	void declareShortcuts();
protected slots:
	void onApplicationAboutToQuit();
	void onApplicationCommitDataRequested(QSessionManager &AManager);
	void onShowSetupPluginsDialog(bool);
	void onSetupPluginsDialogAccepted();
	void onShowAboutBoxDialog();
	void onShutdownTimerTimeout();
private:
	QPointer<AboutBox> FAboutDialog;
	QPointer<SetupPluginsDialog> FPluginsDialog;
private:
	bool FQuitReady;
	bool FQuitStarted;
	bool FCloseStarted;
	int FShutdownKind;
	int FShutdownDelayCount;
	QTimer FShutdownTimer;
private:
	QString FDataPath;
	QDomDocument FPluginsSetup;
	QList<QTranslator *> FTranslators;
	QStringList	FTranslatorNames;

	QList<QString> FBlockedPlugins;
	QHash<QUuid, PluginItem> FPluginItems;
	mutable QMultiHash<QString, IPlugin *> FPlugins;
};

#endif //PLUGINMANAGER_H

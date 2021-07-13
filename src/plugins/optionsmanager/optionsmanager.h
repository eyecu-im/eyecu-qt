#ifndef OPTIONSMANAGER_H
#define OPTIONSMANAGER_H

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QPointer>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/itraymanager.h>
#include <interfaces/iaccountmanager.h>
#include "logindialog.h"
#include "editprofilesdialog.h"
#include "optionsdialogwidget.h"
#include "optionsdialogheader.h"
#include "optionsdialog.h"

#ifdef USE_SYSTEM_QTLOCKEDFILE
#	include <QtSolutions/qtlockedfile.h>
#else
#	include <thirdparty/qtlockedfile/qtlockedfile.h>
#endif

class OptionsManager :
	public QObject,
	public IPlugin,
	public IOptionsManager,
	public IOptionsDialogHolder
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IOptionsManager IOptionsDialogHolder);
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "org.jrudevels.vacuum.IOptionsManager")
#endif
public:
	OptionsManager();
	~OptionsManager();
	// IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return OPTIONSMANAGER_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin();
	// IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	// IOptionsManager
	virtual bool isOpened() const;
	virtual QList<QString> profiles() const;
	virtual QString profilePath(const QString &AProfile) const;
	virtual QString lastActiveProfile() const;
	virtual QString currentProfile() const;
	virtual QByteArray currentProfileKey() const;
	virtual bool setCurrentProfile(const QString &AProfile, const QString &APassword);
	virtual QByteArray profileKey(const QString &AProfile, const QString &APassword) const;
	virtual bool checkProfilePassword(const QString &AProfile, const QString &APassword) const;
	virtual bool changeProfilePassword(const QString &AProfile, const QString &AOldPassword, const QString &ANewPassword);
	virtual bool addProfile(const QString &AProfile, const QString &APassword);
	virtual bool renameProfile(const QString &AProfile, const QString &ANewName);
	virtual bool removeProfile(const QString &AProfile);
	virtual QDialog *showLoginDialog(QWidget *AParent = NULL);
	virtual QDialog *showEditProfilesDialog(QWidget *AParent = NULL);
	// OptionsDialog
	virtual QList<IOptionsDialogHolder *> optionsDialogHolders() const;
	virtual void insertOptionsDialogHolder(IOptionsDialogHolder *AHolder);
	virtual void removeOptionsDialogHolder(IOptionsDialogHolder *AHolder);
	virtual QList<IOptionsDialogNode> optionsDialogNodes() const;
	virtual IOptionsDialogNode optionsDialogNode(const QString &ANodeId) const;
	virtual void insertOptionsDialogNode(const IOptionsDialogNode &ANode);
	virtual void removeOptionsDialogNode(const QString &ANodeId);
	virtual QDialog *showOptionsDialog(const QString &ANodeId = QString(), const QString &ARootId = QString(), QWidget *AParent = NULL);
	// OptionsDialogWidgets
	virtual IOptionsDialogWidget *newOptionsDialogHeader(const QString &ACaption, QWidget *AParent) const;
	virtual IOptionsDialogWidget *newOptionsDialogWidget(const OptionsNode &ANode, const QString &ACaption, QWidget *AParent) const;
	virtual IOptionsDialogWidget *newOptionsDialogWidget(const OptionsNode &ANode, const QString &ACaption, QWidget *AEditor, QWidget *AParent) const;
signals:
	// Profiles
	void profileAdded(const QString &AProfile);
	void profileOpened(const QString &AProfile);
	void profileClosed(const QString &AProfile);
	void profileRenamed(const QString &AProfile, const QString &ANewName);
	void profileRemoved(const QString &AProfile);
	// OptionsDialog
	void optionsDialogHolderInserted(IOptionsDialogHolder *AHolder);
	void optionsDialogHolderRemoved(IOptionsDialogHolder *AHolder);
	void optionsDialogNodeInserted(const IOptionsDialogNode &ANode);
	void optionsDialogNodeRemoved(const IOptionsDialogNode &ANode);
	void optionsModeInitialized(bool AAdvanced); // *** <<< eyeCU >>> ***
protected:
	void closeProfile();
	void openProfile(const QString &AProfile, const QString &APassword);
	QDomDocument profileDocument(const QString &AProfile) const;
	bool saveProfile(const QString &AProfile, const QDomDocument &AProfileDoc) const;
	bool saveCurrentProfileOptions() const;
protected:
	QMap<QString, QVariant> getOptionValues(const OptionsNode &ANode) const;
	QMap<QString, QVariant> loadOptionValues(const QString &AFilePath) const;
	QMap<QString, QVariant> loadAllOptionValues(const QString &AFileName) const;
	void updateOptionValues(const QMap<QString, QVariant> &AOptions) const;
	void updateOptionDefaults(const QMap<QString, QVariant> &AOptions) const;
protected slots:
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
	void onOptionsDialogApplied();
	void onChangeProfileByAction(bool);
	void onShowOptionsDialogByAction(bool);
	void onLoginDialogRejected();
	void onAutoSaveTimerTimeout();
	void onApplicationAboutToQuit();
	void onNewProfileOpened(); // *** <<< eyeCU >>> ***
private:
	ITrayManager *FTrayManager;
	IPluginManager *FPluginManager;
	IMainWindowPlugin *FMainWindowPlugin;
private:
	QDir FProfilesDir;
	QTimer FAutoSaveTimer;
private:
	QString FProfile;
	QByteArray FProfileKey;
	QDomDocument FProfileOptions;
	QtLockedFile *FProfileLocker;
private:
	QPointer<LoginDialog> FLoginDialog;
	QPointer<EditProfilesDialog> FEditProfilesDialog;
	QMap<QString, QPointer<OptionsDialog> > FOptionDialogs;
private:
	Action *FChangeProfileAction;
	Action *FShowOptionsDialogAction;
	QList<IOptionsDialogHolder *> FOptionsHolders;	
	QMap<QString, IOptionsDialogNode> FOptionsDialogNodes;
	bool FAdvanced; // *** <<< eyeCU >>> ***
};

#endif // OPTIONSMANAGER_H

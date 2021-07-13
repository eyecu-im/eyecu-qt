#ifndef IOPTIONSMANAGER_H
#define IOPTIONSMANAGER_H

#include <QList>
#include <QString>
#include <QLayout>
#include <QDialog>
#include <QByteArray>
#include <QDomElement>
#include <QPointer>
#include <utils/options.h>

#define OPTIONSMANAGER_UUID "{d29856c7-8f74-4e95-9aba-b95f4fb42f00}"

struct IOptionsDialogNode {
	int order;
	QString nodeId;
	QString iconkey;
	QString caption;
};

class IOptionsDialogWidget
{
public:
	virtual QWidget *instance() =0;
	virtual void addChildOptionsWidget(IOptionsDialogWidget *AWidget) {
		instance()->layout()->addWidget(AWidget->instance());
		instance()->connect(instance(),SIGNAL(childApply()),AWidget->instance(),SLOT(apply()));
		instance()->connect(instance(),SIGNAL(childReset()),AWidget->instance(),SLOT(reset()));
		instance()->connect(AWidget->instance(),SIGNAL(modified()),instance(),SIGNAL(modified()));
	}
public slots:
	virtual void apply() =0;
	virtual void reset() =0;
protected:
	virtual void modified() =0;
	virtual void childApply() =0;
	virtual void childReset() =0;
};

class IOptionsDialogHolder
{
public:
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) =0;
};

class IOptionsManager
{
public:
	virtual QObject* instance() =0;
	// Profiles
	virtual bool isOpened() const =0;
	virtual QList<QString> profiles() const =0;
	virtual QString profilePath(const QString &AProfile) const =0;
	virtual QString lastActiveProfile() const =0;
	virtual QString currentProfile() const =0;
	virtual QByteArray currentProfileKey() const =0;
	virtual bool setCurrentProfile(const QString &AProfile, const QString &APassword) =0;
	virtual QByteArray profileKey(const QString &AProfile, const QString &APassword) const =0;
	virtual bool checkProfilePassword(const QString &AProfile, const QString &APassword) const =0;
	virtual bool changeProfilePassword(const QString &AProfile, const QString &AOldPassword, const QString &ANewPassword) =0;
	virtual bool addProfile(const QString &AProfile, const QString &APassword) =0;
	virtual bool renameProfile(const QString &AProfile, const QString &ANewName) =0;
	virtual bool removeProfile(const QString &AProfile) =0;
	virtual QDialog *showLoginDialog(QWidget *AParent = NULL) =0;
	virtual QDialog *showEditProfilesDialog(QWidget *AParent = NULL) =0;
	// OptionsDialog
	virtual QList<IOptionsDialogHolder *> optionsDialogHolders() const =0;
	virtual void insertOptionsDialogHolder(IOptionsDialogHolder *AHolder) =0;
	virtual void removeOptionsDialogHolder(IOptionsDialogHolder *AHolder) =0;
	virtual QList<IOptionsDialogNode> optionsDialogNodes() const =0;
	virtual IOptionsDialogNode optionsDialogNode(const QString &ANodeId) const =0;
	virtual void insertOptionsDialogNode(const IOptionsDialogNode &ANode) =0;
	virtual void removeOptionsDialogNode(const QString &ANodeId) =0;
	virtual QDialog *showOptionsDialog(const QString &ANodeId = QString(), const QString &ARootId = QString(), QWidget *AParent = NULL) =0;
	// OptionsDialogWidgets
	virtual IOptionsDialogWidget *newOptionsDialogHeader(const QString &ACaption, QWidget *AParent) const =0;
	virtual IOptionsDialogWidget *newOptionsDialogWidget(const OptionsNode &ANode, const QString &ACaption, QWidget *AParent) const =0;
	virtual IOptionsDialogWidget *newOptionsDialogWidget(const OptionsNode &ANode, const QString &ACaption, QWidget *AEditor, QWidget *AParent) const =0;
protected:
	// Profiles
	virtual void profileAdded(const QString &AProfile) =0;
	virtual void profileOpened(const QString &AProfile) =0;
	virtual void profileClosed(const QString &AProfile) =0;
	virtual void profileRenamed(const QString &AProfile, const QString &ANewName) =0;
	virtual void profileRemoved(const QString &AProfile) =0;
	// OptionsDialog
	virtual void optionsDialogHolderInserted(IOptionsDialogHolder *AHolder) =0;
	virtual void optionsDialogHolderRemoved(IOptionsDialogHolder *AHolder) =0;
	virtual void optionsDialogNodeInserted(const IOptionsDialogNode &ANode) =0;
	virtual void optionsDialogNodeRemoved(const IOptionsDialogNode &ANode) =0;
	virtual void optionsModeInitialized(bool AAdvanced) =0; // *** <<< eyeCU >>> ***
};

Q_DECLARE_INTERFACE(IOptionsDialogWidget,"Vacuum.Plugin.IOptionsDialogWidget/1.1")
Q_DECLARE_INTERFACE(IOptionsDialogHolder,"Vacuum.Plugin.IOptionsDialogWidget/1.1")
Q_DECLARE_INTERFACE(IOptionsManager,"Vacuum.Plugin.IOptionsManager/1.1")

#endif //IOPTIONSMANAGER_H

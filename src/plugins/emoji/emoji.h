#ifndef EMOJI_H
#define EMOJI_H

#include <QHash>
#include <QStringList>
#include <QDir>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iemoji.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/ioptionsmanager.h>
#include "selecticonmenu.h"

struct EmojiTreeItem {
//TODO: Find out what do we need it for
	QString name;
	QMap<QChar, EmojiTreeItem *> childs;
};

class EmojiData : public IEmojiData {
	// IEmojiData interface
public:
	virtual ~EmojiData();
	virtual const QString &id() const override;
	virtual const QString &name() const override;
	virtual const QString &diversity() const override;
	virtual const QString &gender() const override;
	virtual const QStringList &diversities() const override;
	virtual const QStringList &genders() const override;
	virtual bool variation() const override;
	virtual bool present() const override;
	virtual bool display() const override;

	QString FId;
	QString FUnicode;
	QString FName;
	QString FUcs4;
	QString FDiversity;
	QString FGender;
	QStringList FAliases;
	QStringList FDiversities;
	QStringList FGenders;
	int		FCategory;
	bool	FPresent;
	bool	FDisplay;
};


class Emoji:
	public QObject,
	public IPlugin,
	public IEmoji,
	public IMessageWriter,
	public IOptionsDialogHolder,
	public IMessageEditContentsHandler
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IEmoji IMessageWriter IOptionsDialogHolder IMessageEditContentsHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IEmoji")
#endif
public:
	Emoji();
	~Emoji();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return EMOJI_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
	//IMessageWriter
	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang);
	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang);
	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//IMessageEditContentsHandler
	virtual bool messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData);
	virtual bool messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData);
	virtual bool messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument);
	virtual bool messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded);
	virtual QList<QString> activeIconsets() const;	
	//IEmoji
	virtual QString fileByKey(const QString &AKey) const;
	virtual QMap<int, QString> findTextEmoji(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	virtual QMap<int, QString> findImageEmoji(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	virtual QStringList recentIcons(const QString &ASetName) const {Q_UNUSED(ASetName) return FRecent;}
	virtual QMap<uint, QString> setEmoji(const QString &AEmojiSet) const;
	virtual QString categoryName(Category ACategory) const {return FCategoryNames.value(ACategory);}
	virtual QIcon categoryIcon(Category ACategory) const {return FCategoryIcons.value(ACategory);}
	virtual QIcon getIcon(const QString &AEmojiCode, const QSize &ASize=QSize()) const;
	virtual QIcon getIconForSet(const QString &AEmojiSet, const QString &AEmojiText, const QSize &ASize=QSize()) const;
	virtual QMap<uint, IEmojiData*> emojiData(Category ACategory) const;
	virtual const IEmojiData *findData(const QString &AEmojiId, SkinColor ASkinColor=SkinDefault, Gender AGender=GenderDefault) const;
	virtual bool isColored(const QString &AEmojiId) const;
	virtual unsigned categoryCount(Category ACategory) const {return FCategoryCount[ACategory];}
	virtual QStringList emojiSets() const {return FEmojiSets.keys();}
	virtual QList<int> availableSizes(const QString &ASetName) const {return FAvailableSizes.value(ASetName);}
	virtual QString genderSuffix(Gender AGender) const;
	virtual QString skinColorSuffix(SkinColor ASkinColor) const;

protected:
	void findEmojiSets();
	void loadEmojiSet(const QString &AEmojiSet);
	void createTreeItem(const QString &AKey, const QString &AName);
	void clearTreeItem(EmojiTreeItem *AItem) const;
	bool isWordBoundary(const QString &AText) const;
	int replaceTextToImage(QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	int replaceImageToText(QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	SelectIconMenu *createSelectIconMenu(const QString &AIconSet, QWidget *AParent);
	void updateSelectIconMenu(const QString &AIconSet);
	QString getFileName(const EmojiData &AEmojiData, const QDir &ADir) const;
	void updateSize(OptionsNode ANode);
	static int abs(int x) {return x<0?-x:x;}

protected slots:
	void onToolBarWindowLayoutChanged();
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onToolBarWidgetDestroyed(QObject *AObject);
	void onSelectIconMenuSelected(QString AIconId);
	void onSelectIconMenuDestroyed(QObject *AObject);
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
private:
	IMessageWidgets *FMessageWidgets;
	IMessageProcessor *FMessageProcessor;
	IOptionsManager *FOptionsManager;
private:
	EmojiTreeItem FRootTreeItem;
	QMap<int, QHash<QString, QString> > FFileByKey;
	QHash<Category, QMap<uint, IEmojiData*> > FCategories;
	QHash<QString, EmojiData> FEmojiData;
	QHash<QString, QString> FIdByUnicode;
	QList<IMessageToolBarWidget *> FToolBarsWidgets;
	QMap<SelectIconMenu *, IMessageToolBarWidget *> FToolBarWidgetByMenu;

	QStringList FRecent;
	QHash<QString, QMap<uint, QString> > FEmojiSets;
	QHash<QString, QList<int> >	FAvailableSizes;
	mutable QHash<QString, QIcon> FIconHash;
	QMap<int, QString> FCategoryNames;
	QMap<int, QString> FCategoryIDs;
	QMap<int, QIcon> FCategoryIcons;	
	unsigned FCategoryCount[8];
	QDir		FResourceDir;

	const QStringList FGenderSuffixes;
	const QStringList FSkinColorSuffixes;
};

#endif // EMOJI_H

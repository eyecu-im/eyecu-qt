#ifndef EMOJI_H
#define EMOJI_H

#include <QHash>
#include <QStringList>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iemoji.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/ioptionsmanager.h>
#include "selecticonmenu.h"

struct EmoticonTreeItem {
	QUrl url;
	QMap<QChar, EmoticonTreeItem *> childs;
};

class Emoji :
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
	virtual void writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual void writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
	//IMessageEditContentsHandler
	virtual bool messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData);
	virtual bool messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData);
	virtual bool messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument);
	virtual bool messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded);
	//IEmoticons
	virtual QList<QString> activeIconsets() const;
	virtual QUrl urlByKey(const QString &AKey) const;
	virtual QString keyByUrl(const QUrl &AUrl) const;
	virtual QMap<int, QString> findTextEmoticons(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	virtual QMap<int, QString> findImageEmoticons(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	//IEmoji
	virtual bool isColored(const QString &AEmojiText) const;
	virtual const QStringList &colorSuffixes() const {return FColorSuffixes;}
	virtual QStringList recentEmoji(const QString &ASetName) const {return FRecent.value(ASetName);}
protected:
	void createIconsetUrls();
	void createTreeItem(const QString &AKey, const QUrl &AUrl);
	void clearTreeItem(EmoticonTreeItem *AItem) const;
	bool isWordBoundary(const QString &AText) const;
	int replaceTextToImage(QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	int replaceImageToText(QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;
	SelectIconMenu *createSelectIconMenu(const QString &ASubStorage, QWidget *AParent);
	void insertSelectIconMenu(const QString &ASubStorage);
	void removeSelectIconMenu(const QString &ASubStorage);
protected slots:
	void onToolBarWindowLayoutChanged();
	void onToolBarWidgetCreated(IMessageToolBarWidget *AWidget);
	void onToolBarWidgetDestroyed(QObject *AObject);
	void onSelectIconMenuSelected(const QString &ASubStorage, const QString &AIconKey);
	void onSelectIconMenuDestroyed(QObject *AObject);
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
private:
	IMessageWidgets *FMessageWidgets;
	IMessageProcessor *FMessageProcessor;
	IOptionsManager *FOptionsManager;
private:
	EmoticonTreeItem FRootTreeItem;
	QHash<QString, QUrl> FUrlByKey;
	QHash<QString, QString> FKeyByUrl;
	QMap<QString, IconStorage *> FStorages;
	QList<IMessageToolBarWidget *> FToolBarsWidgets;
	QMap<SelectIconMenu *, IMessageToolBarWidget *> FToolBarWidgetByMenu;
	QStringList FColorSuffixes;
	QHash<QString, QStringList> FRecent;
	const QChar FFirst;
};

#endif // EMOJI_H

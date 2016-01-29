#include "emoji.h"
#include <QDebug>
#include <QDir>
#include <QSet>
#include <QChar>
#include <QComboBox>
#include <QMimeData>
#include <QTextBlock>
#include <QDataStream>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditcontentshandlerorders.h>
#include <utils/iconsetdelegate.h>
#include <utils/iconstorage.h>
#include <utils/options.h>
#include <utils/logger.h>
#include <utils/menu.h>
#include <XmlTextDocumentParser>
#include "emojioptions.h"

#define EMOJI_DEFAULT		    "Emoji One"
//#define EMOJI_EXTRA				"vk_extra_small"
//#define EMOJI_FAMILY			"vk_family_small"
//#define EMOJI_SELECTABLE_BIG	"vk_big"
//#define EMOJI_EXTRA_BIG			"vk_extra_big"
//#define EMOJI_FAMILY_BIG		"vk_family_big"

#define EMOJI_DIR			"emojione"

Emoji::Emoji():
	FMessageWidgets(NULL),
	FMessageProcessor(NULL),
	FOptionsManager(NULL)
{}

Emoji::~Emoji()
{
	clearTreeItem(&FRootTreeItem);
}

void Emoji::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Emoji");
	APluginInfo->description = tr("Allows to use emoji in messages");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MESSAGEWIDGETS_UUID);
}

bool Emoji::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
			connect(FMessageWidgets->instance(),SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)),SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
	}

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return FMessageWidgets!=NULL;
}

bool Emoji::initObjects()
{
	FCategoryNames.insert(People, tr("People"));
	FCategoryNames.insert(Symbols, tr("Symbols"));
	FCategoryNames.insert(Flags, tr("Flags"));
	FCategoryNames.insert(Activity, tr("Activity"));
	FCategoryNames.insert(Nature, tr("Nature"));
	FCategoryNames.insert(Travel, tr("Travel"));
	FCategoryNames.insert(Objects, tr("Objects"));
	FCategoryNames.insert(Foods, tr("Foods"));

	FCategoryIDs.insert(People, "people");
	FCategoryIDs.insert(Symbols, "symbols");
	FCategoryIDs.insert(Flags, "flags");
	FCategoryIDs.insert(Activity, "activity");
	FCategoryIDs.insert(Nature, "nature");
	FCategoryIDs.insert(Travel, "travel");
	FCategoryIDs.insert(Objects, "objects");
	FCategoryIDs.insert(Foods, "foods");

	if (FMessageProcessor)
		FMessageProcessor->insertMessageWriter(MWO_EMOTICONS,this);

	if (FMessageWidgets)
		FMessageWidgets->insertEditContentsHandler(MECHO_EMOJI_CONVERT_IMAGE2TEXT,this);

	for (uint i=0x1f3fb; i<=0x1f3ff; ++i)
		FColorSuffixes.append(QString::fromUcs4(&i, 1));

	findEmojiSets();

	return true;
}

bool Emoji::isColored(const QString &AEmojiCode) const
{
	int size = AEmojiCode.size();
	ushort last = AEmojiCode.at(size-1).unicode();
	return (size>1) && (AEmojiCode.at(size-2)==QChar(0xD83C)) && (last>0xDFFA) && (last<0xE000);
}

bool Emoji::initSettings()
{
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_ICONSET, EMOJI_DEFAULT);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SKINCOLOR, 0);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_RECENT, QByteArray());

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
	return true;
}

void Emoji::writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AMessage); Q_UNUSED(ALang);
	if (AOrder == MWO_EMOTICONS)
		replaceImageToText(ADocument);
}

void Emoji::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AMessage); Q_UNUSED(ALang);
	if (AOrder == MWO_EMOTICONS)
		replaceTextToImage(ADocument);
}

QMultiMap<int, IOptionsDialogWidget *> Emoji::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_APPEARANCE)
	{
		widgets.insertMulti(OHO_APPEARANCE_MESSAGES, FOptionsManager->newOptionsDialogHeader(tr("Message windows"), AParent));
		widgets.insertMulti(OWO_APPEARANCE_EMOJI, new EmojiOptions(this,AParent));
	}
	return widgets;
}

bool Emoji::messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData)
{
	Q_UNUSED(AOrder); Q_UNUSED(AWidget); Q_UNUSED(AData);
	return false;
}

bool Emoji::messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData)
{
	Q_UNUSED(AOrder); Q_UNUSED(AWidget); Q_UNUSED(AData);
	return false;
}

bool Emoji::messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument)
{
	Q_UNUSED(AOrder); Q_UNUSED(AData);
	if (AOrder == MECHO_EMOJI_CONVERT_IMAGE2TEXT)
	{
		QList<QUrl> urlList = FUrlByKey.values();
		QTextBlock block = ADocument->firstBlock();
		while (block.isValid())
		{
			for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
			{
				QTextFragment fragment = it.fragment();
				if (fragment.charFormat().isImageFormat())
				{
					QUrl url = fragment.charFormat().toImageFormat().name();
					if (AWidget->document()->resource(QTextDocument::ImageResource,url).isNull() && urlList.contains(url))
					{
						AWidget->document()->addResource(QTextDocument::ImageResource,url,QImage(url.toLocalFile()));
						AWidget->document()->markContentsDirty(fragment.position(),fragment.length());
					}
				}
			}
			block = block.next();
		}
	}
	return false;
}

bool Emoji::messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded)
{
	Q_UNUSED(AOrder); Q_UNUSED(AWidget); Q_UNUSED(APosition); Q_UNUSED(ARemoved); Q_UNUSED(AAdded);
	return false;
}

QList<QString> Emoji::activeIconsets() const
{
/*
	QList<QString> iconsets = FStorages.keys();// = Options::node(OPV_MESSAGES_EMOJI_ICONSETS).value().toStringList();
	for (QList<QString>::iterator it = iconsets.begin(); it != iconsets.end(); )
	{
		if ((*it).at(0)=='@')
			it = iconsets.erase(it);
		else
			++it;
	}
	return iconsets;
*/
	QList<QString> iconsets;
	iconsets << "emojione";
	return iconsets;
}

QUrl Emoji::urlByKey(const QString &AKey) const
{
	return FUrlByKey.value(AKey);
}

QString Emoji::keyByUrl(const QUrl &AUrl) const
{
	return FKeyByUrl.value(AUrl.toString());
}

QMap<int, QString> Emoji::findTextEmoticons(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> emoticons;
	if (!FUrlByKey.isEmpty())
	{
		int stopPos = ALength < 0 ? ADocument->characterCount() : AStartPos+ALength;
		for (QTextBlock block = ADocument->findBlock(AStartPos); block.isValid() && block.position()<stopPos; block = block.next())
			for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
			{
				QTextFragment fragment = it.fragment();
				if (fragment.length()>0 && fragment.position()<stopPos)
				{
					QString searchText = fragment.text();
					for (int keyPos=0; keyPos<searchText.length(); keyPos++)
						if (!searchText.at(keyPos).isSpace())
						{
							int keyLength, keyLengthCurrent = 0;
							QString key;
							for (const EmojiTreeItem *item = &FRootTreeItem; item && keyLengthCurrent<=searchText.length()-keyPos && fragment.position()+keyPos+keyLengthCurrent<=stopPos; ++keyLengthCurrent)
							{
								if (!item->url.isEmpty())
									key = searchText.mid(keyPos, keyLength = keyLengthCurrent);
								if (keyPos+keyLengthCurrent<searchText.length())
									item = item->childs.value(searchText.at(keyPos+keyLengthCurrent));
							}

							if (!key.isEmpty())
							{
								emoticons.insert(fragment.position()+keyPos, key);
								keyPos += keyLength-1;
								key.clear();
							}
						}
				}
			}
	}
	return emoticons;
}

QMap<int, QString> Emoji::findImageEmoticons(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> emoticons;
	if (!FKeyByUrl.isEmpty())
	{
		QTextBlock block = ADocument->findBlock(AStartPos);
		int stopPos = ALength < 0 ? ADocument->characterCount() : AStartPos+ALength;
		while (block.isValid() && block.position()<stopPos)
		{
			for (QTextBlock::iterator it = block.begin(); !it.atEnd() && it.fragment().position()<stopPos; ++it)
			{
				const QTextFragment fragment = it.fragment();
				if (fragment.charFormat().isImageFormat())
				{
					QString key = FKeyByUrl.value(fragment.charFormat().toImageFormat().name());
					if (!key.isEmpty() && fragment.length()==1)
						emoticons.insert(fragment.position(),key);
				}
			}
			block = block.next();
		}
	}
	return emoticons;
}

QIcon Emoji::getIcon(const QString &AEmojiCode, const QSize &ASize) const
{
	QIcon icon;
	if (FIconHash.contains(AEmojiCode))
		icon = FIconHash[AEmojiCode];
	if (!icon.availableSizes().contains(ASize))
	{
		if (FUrlByKey.contains(AEmojiCode))
		{
			QFile file(FUrlByKey[AEmojiCode].toLocalFile());
			if (file.open(QIODevice::ReadOnly))
			{
				QImageReader reader(&file);
				if (!ASize.isNull())
					reader.setScaledSize(ASize);
				icon.addPixmap(QPixmap::fromImageReader(&reader));
				file.close();
				FIconHash.insert(AEmojiCode, icon);
			}
		}
	}
	return icon;
}

QMap<uint, EmojiData> Emoji::emojiData(Category ACategory) const
{
	return FCategories.value(ACategory);
}

EmojiData Emoji::findData(const QString &AEmojiCode) const
{
	return FEmojiData.value(AEmojiCode);
}

void Emoji::findEmojiSets()
{
	QList<QString> resourceDirs = FileStorage::resourcesDirs();
	for (QList<QString>::ConstIterator it=resourceDirs.constBegin(); it!=resourceDirs.constEnd(); ++it)
	{
		FCategories.clear();
		FEmojiData.clear();
		FEmojiSets.clear();
		QDir dir(*it);
		if (dir.cd(EMOJI_DIR))
		{
			QFile file(dir.absoluteFilePath("emoji.json"));
			if (file.open(QIODevice::ReadOnly))
			{
				QByteArray json = file.readAll();
				file.close();
				if (!json.isEmpty())
				{
					QScriptEngine engine;
					QString js("("+QString::fromUtf8(json)+")");

					QScriptValue value = engine.evaluate(js);
					if (value.isValid())
					{
						QScriptValueIterator it(value);
						do
						{
							it.next();
							EmojiData emojiData;

							emojiData.id = it.name();

							QScriptValue val = it.value();
							emojiData.name = val.property("name").toString();

							QString category = val.property("category").toString();
							QString emojiOrder = val.property("emoji_order").toString();
							if (!val.property("aliases").isNull())
							{
								QVariantList list=val.property("aliases").toVariant().toList();
								QList<QString> aliases;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
									aliases.append((*it).toString());
							}

							if (!val.property("aliases_ascii").isNull())
							{
								QVariantList list=val.property("aliases_ascii").toVariant().toList();
								QList<QString> aliases;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
									aliases.append((*it).toString());
								emojiData.aliases = aliases;
							}

							if (!val.property("keywords").isNull())
							{
								QVariantList list=val.property("keywords").toVariant().toList();
								QString out;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
								{
									if (!out.isEmpty())
										out.append(',');
									out.append((*it).toString());
								}
							}

							bool ok;
							uint ucs4[16];
							int  i(0);
							emojiData.ucs4 = val.property("unicode").toString();
							QList<QString> splitted = emojiData.ucs4.split('-');
							for (QList<QString>::ConstIterator it=splitted.constBegin(); it!=splitted.constEnd(); ++it, ++i)
							{
								ucs4[i]=(*it).toInt(&ok, 16);
								if (!ok)
									break;
							}
							if (ok)
							{
								emojiData.unicode = QString::fromUcs4(ucs4, i);
								if (!isColored(emojiData.unicode))
								{
									uint order = emojiOrder.toInt();
									FCategories[(Category)FCategoryIDs.key(category)].insert(order, emojiData);
									FEmojiData.insert(emojiData.unicode, emojiData);
									emojiData.colored=false;
								}
							}
						} while (it.hasNext());
// ------------------- JSON descriptor parsed -------------------
// ----------------- Looking for category icons -----------------
						if (dir.cd("category_icons"))
						{
							for (QMap<int, QString>::ConstIterator ifc=FCategoryIDs.constBegin(); ifc!=FCategoryIDs.constEnd(); ++ifc)
							{
								QFile file(dir.absoluteFilePath(*ifc+".svg"));
								if (file.open(QIODevice::ReadOnly))
								{
									QImageReader reader(&file);
									reader.setScaledSize(QSize(16, 16));
									QPixmap pixmap = QPixmap::fromImageReader(&reader);
									if (!pixmap.isNull())
										FCategoryIcons.insert(ifc.key(), QIcon(pixmap));
									file.close();
								}
							}
							dir.cdUp();
						}
// ------------------ Getting Emoji set list --------------------
						if (dir.cd("assets"))
						{
							FEmojiSets = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
							if (!FEmojiSets.isEmpty())
							{
								FResourceDir = dir;
								break;
							}
							dir.cdUp();
						}
// --------------------------------------------------------------
					}
				}
				else
					qWarning() << "JSON is empty!";
			}
			else
				qWarning() << "failed to open file!";
			dir.cdUp();
		}
	}
}

void Emoji::loadEmojiSet(const QString &AEmojiSet)
{
	QDir dir(FResourceDir);
	dir.cd(AEmojiSet);
	dir.cd("png");
	dir.cd("16");

	FUrlByKey.clear();
	FKeyByUrl.clear();
	clearTreeItem(&FRootTreeItem);

	for (QHash<Category, QMap<uint, EmojiData> >::Iterator itc=FCategories.begin(); itc!=FCategories.end(); ++itc)
		for (QMap<uint, EmojiData>::Iterator it = (*itc).begin(); it!=(*itc).end(); ++it)
		{			
			QFile file(QDir::cleanPath(dir.absoluteFilePath((*it).ucs4+".png")));
			if (file.exists())
			{
				QUrl url(QUrl::fromLocalFile(file.fileName()));
				(*it).colored = false;
				for (uint c=0x1f3fb; c<=0x1f3ff; ++c)
				{
					QFile file(dir.absoluteFilePath(QString("%1-%2.png").arg((*it).ucs4).arg(c, 0, 16)));
					if (file.exists())
					{
						(*it).colored = true;						
						QString unicode = (*it).unicode+QString::fromUcs4(&c, 1);
						QUrl url(QUrl::fromLocalFile(file.fileName()));
						FUrlByKey.insert(unicode, url);
						FKeyByUrl.insert(url.toString(), unicode);
						createTreeItem(unicode, url);
					}
				}
				FUrlByKey.insert((*it).unicode, url);
				FKeyByUrl.insert(url.toString(), (*it).unicode);
				createTreeItem((*it).unicode, url);
			}
		}

	for (uint c=0x1f3fb; c<=0x1f3ff; ++c)
	{
		QFile file(dir.absoluteFilePath(QString("%1.png").arg(c, 0, 16)));
		if (file.exists())
		{
			QString unicode = QString::fromUcs4(&c, 1);
			QUrl url(QUrl::fromLocalFile(file.fileName()));
			FUrlByKey.insert(unicode, url);
			FKeyByUrl.insert(url.toString(), unicode);
		}
	}
}

void Emoji::createTreeItem(const QString &AKey, const QUrl &AUrl)
{
	EmojiTreeItem *item = &FRootTreeItem;
	for (int i=0; i<AKey.size(); i++)
	{
		QChar itemChar = AKey.at(i);
		if (!item->childs.contains(itemChar))
		{
			EmojiTreeItem *childItem = new EmojiTreeItem;
			item->childs.insert(itemChar,childItem);
			item = childItem;
		}
		else
		{
			item = item->childs.value(itemChar);
		}
	}
	item->url = AUrl;
}

void Emoji::clearTreeItem(EmojiTreeItem *AItem) const
{
	foreach(const QChar &itemChar, AItem->childs.keys())
	{
		EmojiTreeItem *childItem = AItem->childs.take(itemChar);
		clearTreeItem(childItem);
		delete childItem;
	}
}

bool Emoji::isWordBoundary(const QString &AText) const
{
	return !AText.isEmpty() ? AText.at(0).isSpace() : true;
}

int Emoji::replaceTextToImage(QTextDocument *ADocument, int AStartPos, int ALength) const
{
	int posOffset = 0;
	QMap<int,QString> emoticons = findTextEmoticons(ADocument,AStartPos,ALength);
	if (!emoticons.isEmpty())
	{
		QTextCursor cursor(ADocument);
		cursor.beginEditBlock();
		for (QMap<int,QString>::const_iterator it=emoticons.constBegin(); it!=emoticons.constEnd(); ++it)
		{
			QUrl url = FUrlByKey.value(it.value());
			if (!url.isEmpty())
			{
				cursor.setPosition(it.key()-posOffset);
				cursor.setPosition(cursor.position()+it->length(), QTextCursor::KeepAnchor);

				QTextImageFormat format;
				format.setName(url.toString());
				format.setToolTip(findData(it.value()).name);
				if (!ADocument->resource(QTextDocument::ImageResource,url).isValid())
					ADocument->addResource(QTextDocument::ImageResource, url, QImage(url.toLocalFile()));
				cursor.insertImage(format);
				posOffset += it->length()-1;
			}
		}
		cursor.endEditBlock();
	}
	return posOffset;
}

int Emoji::replaceImageToText(QTextDocument *ADocument, int AStartPos, int ALength) const
{
	int posOffset = 0;
	QMap<int,QString> emoticons = findImageEmoticons(ADocument,AStartPos,ALength);
	if (!emoticons.isEmpty())
	{
		QTextCursor cursor(ADocument);
		cursor.beginEditBlock();
		for (QMap<int,QString>::const_iterator it=emoticons.constBegin(); it!=emoticons.constEnd(); ++it)
		{
			cursor.setPosition(it.key()+posOffset);
			cursor.deleteChar();
			posOffset--;

			if (cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,1))
				cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);

			cursor.insertText(it.value());
			posOffset += it->length();
		}
		cursor.endEditBlock();
	}
	return posOffset;
}

SelectIconMenu *Emoji::createSelectIconMenu(const QString &ASubStorage, QWidget *AParent)
{
	SelectIconMenu *menu = new SelectIconMenu(ASubStorage, this, AParent);
	connect(menu->instance(),SIGNAL(iconSelected(QString, QString)), SLOT(onSelectIconMenuSelected(QString, QString)));
	connect(menu->instance(),SIGNAL(destroyed(QObject *)),SLOT(onSelectIconMenuDestroyed(QObject *)));
	return menu;
}

//void Emoji::insertSelectIconMenu(const QString &ASubStorage)
//{
//	foreach(IMessageToolBarWidget *widget, FToolBarsWidgets)
//	{
//		SelectIconMenu *menu = createSelectIconMenu(ASubStorage,widget->instance());
//		FToolBarWidgetByMenu.insert(menu,widget);
//		QToolButton *button = widget->toolBarChanger()->insertAction(menu->menuAction(),TBG_MWTBW_EMOTICONS);
//		button->setPopupMode(QToolButton::InstantPopup);
//	}
//}

//void Emoji::removeSelectIconMenu(const QString &ASubStorage)
//{
//	QMap<SelectIconMenu *,IMessageToolBarWidget *>::iterator it = FToolBarWidgetByMenu.begin();
//	while (it != FToolBarWidgetByMenu.end())
//	{
//		SelectIconMenu *menu = it.key();
//		IMessageToolBarWidget *widget = it.value();
//		if (menu->iconset() == ASubStorage)
//		{
//			widget->toolBarChanger()->removeItem(widget->toolBarChanger()->actionHandle(menu->menuAction()));
//			it = FToolBarWidgetByMenu.erase(it);
//			delete menu;
//		}
//		else
//			++it;
//	}
//}

void Emoji::onToolBarWindowLayoutChanged()
{
	IMessageWindow *window = qobject_cast<IMessageWindow *>(sender());
	if (window && window->toolBarWidget())
		foreach(QAction *handle, window->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_EMOTICONS))
			handle->setVisible(window->editWidget()->isVisibleOnWindow());
}

void Emoji::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	if (AWidget->messageWindow()->editWidget())
	{
		FToolBarsWidgets.append(AWidget);
		if (AWidget->messageWindow()->editWidget()->isVisibleOnWindow())
			foreach(const QString &substorage, activeIconsets())
			{
				SelectIconMenu *menu = createSelectIconMenu(substorage,AWidget->instance());
				FToolBarWidgetByMenu.insert(menu,AWidget);
				QToolButton *button = AWidget->toolBarChanger()->insertAction(menu->menuAction(),TBG_MWTBW_EMOTICONS);
				button->setPopupMode(QToolButton::InstantPopup);
			}
		connect(AWidget->instance(),SIGNAL(destroyed(QObject *)),SLOT(onToolBarWidgetDestroyed(QObject *)));
		connect(AWidget->messageWindow()->instance(),SIGNAL(widgetLayoutChanged()),SLOT(onToolBarWindowLayoutChanged()));
	}
}

void Emoji::onToolBarWidgetDestroyed(QObject *AObject)
{
	QList<IMessageToolBarWidget *>::iterator it = FToolBarsWidgets.begin();
	while (it != FToolBarsWidgets.end())
	{
		if (qobject_cast<QObject *>((*it)->instance()) == AObject)
			it = FToolBarsWidgets.erase(it);
		else
			++it;
	}
}

void Emoji::onSelectIconMenuSelected(QString AIconKey, const QString &AIconText)
{
	SelectIconMenu *menu = qobject_cast<SelectIconMenu *>(sender());
	if (FToolBarWidgetByMenu.contains(menu))
	{
		IMessageEditWidget *widget = FToolBarWidgetByMenu.value(menu)->messageWindow()->editWidget();
		if (widget)
		{
			QUrl url = FUrlByKey.value(AIconKey);
			if (!url.isEmpty())
			{
				QTextEdit *editor = widget->textEdit();
				QTextCursor cursor = editor->textCursor();
				cursor.beginEditBlock();

				if (cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,1))
					cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);
				
				if (!editor->document()->resource(QTextDocument::ImageResource,url).isValid())
					editor->document()->addResource(QTextDocument::ImageResource,url,QImage(url.toLocalFile()));
				QTextImageFormat imageFormat;
				imageFormat.setName(url.toString());
				imageFormat.setToolTip(AIconText);
				cursor.insertImage(imageFormat);

				cursor.endEditBlock();
				editor->setFocus();

				if (isColored(AIconKey))
					AIconKey.chop(2);

				QStringList recent = FRecent;

				if (recent.isEmpty() || recent.first()!=AIconKey)
				{
					if (recent.contains(AIconKey))
						recent.removeOne(AIconKey);
					recent.insert(0, AIconKey);
					if (recent.size()>10)
						recent.removeLast();
					FRecent = recent;
					QByteArray array;
					QDataStream stream(&array, QIODevice::WriteOnly);
					stream << recent;
					Options::node(OPV_MESSAGES_EMOJI_RECENT).setValue(array);
				}
			}
		}
	}
}

void Emoji::onSelectIconMenuDestroyed(QObject *AObject)
{
	foreach(SelectIconMenu *menu, FToolBarWidgetByMenu.keys())
		if (qobject_cast<QObject *>(menu) == AObject)
			FToolBarWidgetByMenu.remove(menu);
}

void Emoji::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MESSAGES_EMOJI_ICONSET));
	QByteArray array(Options::node(OPV_MESSAGES_EMOJI_RECENT).value().toByteArray());
	if (!array.isEmpty())
	{
		QDataStream stream(&array, QIODevice::ReadOnly);
		stream >> FRecent;
	}
}

void Emoji::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_EMOJI_ICONSET)
	{
/*
		QList<QString> oldStorages = FStorages.keys();
		QList<QString> availStorages = IconStorage::availSubStorages(RSR_STORAGE_EMOJI);

		foreach(const QString &substorage, Options::node(OPV_MESSAGES_EMOJI_ICONSETS).value().toStringList())
		{
			QString ss = substorage;
			if (ss.at(0)=='@')
				ss.remove(0,1);
			if (availStorages.contains(ss))
			{
				if (!FStorages.contains(substorage))
				{
					LOG_DEBUG(QString("Creating emoji storage=%1").arg(substorage));
					if (substorage.at(0)!='@')
						insertSelectIconMenu(substorage);
					FStorages.insert(substorage, new IconStorage(RSR_STORAGE_EMOJI,ss,this));
				}
				oldStorages.removeAll(substorage);
			}
			else
			{
				LOG_WARNING(QString("Selected emoji storage=%1 not available").arg(substorage));
			}
		}

		foreach (const QString &substorage, oldStorages)
		{
			LOG_DEBUG(QString("Removing emoji storage=%1").arg(substorage));
			if (substorage.at(0)!='@')
				removeSelectIconMenu(substorage);
			delete FStorages.take(substorage);
		}
*/
		loadEmojiSet(ANode.value().toString());
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_emoji, Emoji)
#endif

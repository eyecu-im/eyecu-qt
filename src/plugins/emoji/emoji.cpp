#include "emoji.h"

#include <QDebug>
#include <QSet>
#include <QChar>
#include <QComboBox>
#include <QMimeData>
#include <QTextBlock>
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

#define EMOJI_SELECTABLE		"vk_small"
#define EMOJI_EXTRA				"vk_extra_small"
#define EMOJI_SELECTABLE_BIG	"vk_bog"
#define EMOJI_EXTRA_BIG			"vk_extra_big"

Emoji::Emoji()
{
	FMessageWidgets = NULL;
	FMessageProcessor = NULL;
	FOptionsManager = NULL;
}

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
	{
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return FMessageWidgets!=NULL;
}

bool Emoji::initObjects()
{
	if (FMessageProcessor)
		FMessageProcessor->insertMessageWriter(MWO_EMOTICONS,this);

	if (FMessageWidgets)
		FMessageWidgets->insertEditContentsHandler(MECHO_EMOJI_CONVERT_IMAGE2TEXT,this);

	return true;
}

bool Emoji::initSettings()
{
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_ICONSETS, QStringList() << EMOJI_SELECTABLE	<< "@"EMOJI_EXTRA);

	if (FOptionsManager)
	{
		FOptionsManager->insertOptionsDialogHolder(this);
	}
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

//		if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
			widgets.insertMulti(OWO_APPEARANCE_EMOJI, new EmojiOptions(this,AParent));
//		else
//		{
//		QComboBox *cmbEmoticons = new QComboBox(AParent);
//		cmbEmoticons->setItemDelegate(new IconsetDelegate(cmbEmoticons));

//		int index = 1;
//		cmbEmoticons->addItem(tr("Do not convert text smileys to images"),QStringList());
//		foreach(const QString &iconset, IconStorage::availSubStorages(RSR_STORAGE_EMOJI))
//		{
//			IconStorage *storage = new IconStorage(RSR_STORAGE_EMOJI,iconset);
//			cmbEmoticons->addItem(storage->getIcon(storage->fileKeys().value(0)),storage->storageProperty(FILE_STORAGE_NAME,iconset),QStringList()<<iconset);

//			cmbEmoticons->setItemData(index,storage->storage(),IconsetDelegate::IDR_STORAGE);
//			cmbEmoticons->setItemData(index,storage->subStorage(),IconsetDelegate::IDR_SUBSTORAGE);
//			cmbEmoticons->setItemData(index,true,IconsetDelegate::IDR_HIDE_STORAGE_NAME);

//			index++;

//			delete storage;
//		}
//		widgets.insertMulti(OWO_APPEARANCE_EMOJI, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_EMOJI_ICONSET),tr("Emoji:"),cmbEmoticons,AParent));
//		}
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
		if (AWidget->isRichTextEnabled())
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
						if (AWidget->document()->resource(QTextDocument::ImageResource,url).isNull())
						{
							if (urlList.contains(url))
							{
								AWidget->document()->addResource(QTextDocument::ImageResource,url,QImage(url.toLocalFile()));
								AWidget->document()->markContentsDirty(fragment.position(),fragment.length());
							}
						}
					}
				}
				block = block.next();
			}
		}
		else
		{
			replaceImageToText(ADocument);
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
	QList<QString> iconsets = Options::node(OPV_MESSAGES_EMOJI_ICONSETS).value().toStringList();
	for (QList<QString>::iterator it = iconsets.begin(); it != iconsets.end(); )
	{
		if (!FStorages.contains(*it))
			it = iconsets.erase(it);
		else
			++it;
	}
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
		QTextBlock block = ADocument->findBlock(AStartPos);
		int stopPos = ALength < 0 ? ADocument->characterCount() : AStartPos+ALength;
		while (block.isValid() && block.position()<stopPos)
		{
			for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
			{
				QTextFragment fragment = it.fragment();
				if (fragment.length()>0 && fragment.position()<stopPos)
				{
					bool searchStarted = true;
					QString searchText = fragment.text();
					for (int keyPos=0; keyPos<searchText.length(); keyPos++)
					{
						searchStarted = searchStarted || searchText.at(keyPos).isSpace();
						if (searchStarted && !searchText.at(keyPos).isSpace())
						{
							int keyLength = 0;
							const EmoticonTreeItem *item = &FRootTreeItem;
							QString key;
							while (item && keyLength<=searchText.length()-keyPos && fragment.position()+keyPos+keyLength<=stopPos)
							{
//								qDebug() << "item=" << item;
//								qDebug() << "url=" << item->url;
//								qDebug() << "keyLength=" << keyLength;

								const QChar nextChar = keyPos+keyLength<searchText.length() ? searchText.at(keyPos+keyLength) : QChar();
//								qDebug() << "nextChar=" << nextChar << " (0x" << QString::number(nextChar.unicode(), 16) << ")";
								if (!item->url.isEmpty())
								{
									key = searchText.mid(keyPos,keyLength);
									qDebug() << "key=" << key;
								}
								else
								{
									if (!key.isEmpty())
										item = NULL;
								}
//								if (!nextChar.isNull())
								if (item)
									item = item->childs.value(nextChar);
								keyLength++;
							}

							if (!key.isEmpty())
							{
								qDebug() << "inserting:" << fragment.position() << "," << key;
								emoticons.insert(fragment.position()+keyPos, key);
								keyPos += keyLength-1;
								key.clear();
							}
							else
								keyPos ++;
							searchStarted = false;
						}
					}
				}
			}
			block = block.next();
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

void Emoji::createIconsetUrls()
{
	FUrlByKey.clear();
	FKeyByUrl.clear();
	clearTreeItem(&FRootTreeItem);

	foreach(const QString &substorage, Options::node(OPV_MESSAGES_EMOJI_ICONSETS).value().toStringList())
	{
		IconStorage *storage = FStorages.value(substorage);
		if (storage)
		{
			QHash<QString, QString> fileFirstKey;
			foreach(const QString &key, storage->fileFirstKeys())
				fileFirstKey.insert(storage->fileFullName(key), key);

			foreach(const QString &key, storage->fileKeys())
			{
				if (!FUrlByKey.contains(key)) 
				{
					QString file = storage->fileFullName(key);
					QUrl url = QUrl::fromLocalFile(file);
					FUrlByKey.insert(key,url);
					FKeyByUrl.insert(url.toString(),fileFirstKey.value(file));
					createTreeItem(key,url);
				}
			}
		}
	}
}

void Emoji::createTreeItem(const QString &AKey, const QUrl &AUrl)
{
	EmoticonTreeItem *item = &FRootTreeItem;
	for (int i=0; i<AKey.size(); i++)
	{
		QChar itemChar = AKey.at(i);
		if (!item->childs.contains(itemChar))
		{
			EmoticonTreeItem *childItem = new EmoticonTreeItem;
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

void Emoji::clearTreeItem(EmoticonTreeItem *AItem) const
{
	foreach(const QChar &itemChar, AItem->childs.keys())
	{
		EmoticonTreeItem *childItem = AItem->childs.take(itemChar);
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
				if (!ADocument->resource(QTextDocument::ImageResource,url).isValid())
					cursor.insertImage(QImage(url.toLocalFile()),url.toString());
				else
					cursor.insertImage(url.toString());
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
			{
				bool space = !isWordBoundary(cursor.selectedText());
				cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);
				if (space)
				{
					posOffset++;
					cursor.insertText(" ");
				}
			}

			cursor.insertText(it.value());
			posOffset += it->length();

			if (cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,1))
			{
				bool space = !isWordBoundary(cursor.selectedText());
				cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,1);
				if (space)
				{
					posOffset++;
					cursor.insertText(" ");
				}
			}

		}
		cursor.endEditBlock();
	}
	return posOffset;
}

SelectIconMenu *Emoji::createSelectIconMenu(const QString &ASubStorage, QWidget *AParent)
{
	SelectIconMenu *menu = new SelectIconMenu(ASubStorage, AParent);
	connect(menu->instance(),SIGNAL(iconSelected(const QString &, const QString &)), SLOT(onSelectIconMenuSelected(const QString &, const QString &)));
	connect(menu->instance(),SIGNAL(destroyed(QObject *)),SLOT(onSelectIconMenuDestroyed(QObject *)));
	return menu;
}

void Emoji::insertSelectIconMenu(const QString &ASubStorage)
{
	qDebug() << "Emoji::insertSelectIconMenu(" << ASubStorage << ")";
	foreach(IMessageToolBarWidget *widget, FToolBarsWidgets)
	{
		SelectIconMenu *menu = createSelectIconMenu(ASubStorage,widget->instance());
		FToolBarWidgetByMenu.insert(menu,widget);
		QToolButton *button = widget->toolBarChanger()->insertAction(menu->menuAction(),TBG_MWTBW_EMOTICONS);
		button->setPopupMode(QToolButton::InstantPopup);
	}
}

void Emoji::removeSelectIconMenu(const QString &ASubStorage)
{
	QMap<SelectIconMenu *,IMessageToolBarWidget *>::iterator it = FToolBarWidgetByMenu.begin();
	while (it != FToolBarWidgetByMenu.end())
	{
		SelectIconMenu *menu = it.key();
		IMessageToolBarWidget *widget = it.value();
		if (menu->iconset() == ASubStorage)
		{
			widget->toolBarChanger()->removeItem(widget->toolBarChanger()->actionHandle(menu->menuAction()));
			it = FToolBarWidgetByMenu.erase(it);
			delete menu;
		}
		else
		{
			++it;
		}
	}
}

void Emoji::onToolBarWindowLayoutChanged()
{
	IMessageWindow *window = qobject_cast<IMessageWindow *>(sender());
	if (window && window->toolBarWidget())
	{
		foreach(QAction *handle, window->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_EMOTICONS))
			handle->setVisible(window->editWidget()->isVisibleOnWindow());
	}
}

void Emoji::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	if (AWidget->messageWindow()->editWidget())
	{
		FToolBarsWidgets.append(AWidget);
		if (AWidget->messageWindow()->editWidget()->isVisibleOnWindow())
		{
			foreach(const QString &substorage, activeIconsets())
			{
				SelectIconMenu *menu = createSelectIconMenu(substorage,AWidget->instance());
				FToolBarWidgetByMenu.insert(menu,AWidget);
				QToolButton *button = AWidget->toolBarChanger()->insertAction(menu->menuAction(),TBG_MWTBW_EMOTICONS);
				button->setPopupMode(QToolButton::InstantPopup);
			}
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

void Emoji::onSelectIconMenuSelected(const QString &ASubStorage, const QString &AIconKey)
{
	Q_UNUSED(ASubStorage);
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
				{
					bool space = !isWordBoundary(cursor.selectedText());
					cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);
					if (space)
						cursor.insertText(" ");
				}
				
				if (widget->isRichTextEnabled())
				{
					if (!editor->document()->resource(QTextDocument::ImageResource,url).isValid())
						editor->document()->addResource(QTextDocument::ImageResource,url,QImage(url.toLocalFile()));
                    QTextImageFormat imageFormat;
                    imageFormat.setName(url.toString());
                    imageFormat.setProperty(XmlTextDocumentParser::ImageAlternativeText, AIconKey);
                    imageFormat.setProperty(QTextCharFormat::TextToolTip, AIconKey);
                    cursor.insertImage(imageFormat);
				}
				else
				{
					cursor.insertText(AIconKey);
				}

				if (cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,1))
				{
					bool space = !isWordBoundary(cursor.selectedText());
					cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,1);
					if (space)
						cursor.insertText(" ");
				}

				cursor.endEditBlock();
				editor->setFocus();
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
	onOptionsChanged(Options::node(OPV_MESSAGES_EMOJI_ICONSETS));
}

void Emoji::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_EMOJI_ICONSETS)
	{
		QList<QString> oldStorages = FStorages.keys();
		QList<QString> availStorages = IconStorage::availSubStorages(RSR_STORAGE_EMOJI);

		foreach(const QString &substorage, Options::node(OPV_MESSAGES_EMOJI_ICONSETS).value().toStringList())
		{
			qDebug() << "substorage=" << substorage;
			if (availStorages.contains(substorage))
			{
				if (!FStorages.contains(substorage))
				{
					LOG_DEBUG(QString("Creating emoji storage=%1").arg(substorage));
					FStorages.insert(substorage, new IconStorage(RSR_STORAGE_EMOJI,substorage,this));
					insertSelectIconMenu(substorage);
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
			removeSelectIconMenu(substorage);
			delete FStorages.take(substorage);
		}

		createIconsetUrls();
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_emoji, Emoji)
#endif

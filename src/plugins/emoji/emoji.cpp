#include <QDebug>
#include <QDir>
#include <QSet>
#include <QChar>
#include <QComboBox>
#include <QMimeData>
#include <QTextBlock>
#include <QDataStream>
#include <qmath.h>
#if QT_VERSION < 0x050000
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#else
#include <QJsonDocument>
#include <QJsonArray>
#endif
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

#include "emojioptions.h"
#include "emoji.h"

#define EMOJI_DEFAULT	"Emoji One"
#define EMOJI_DIR		"emoji"

Emoji::Emoji():
	FMessageWidgets(nullptr),
	FMessageProcessor(nullptr),
	FOptionsManager(nullptr)
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

	IPlugin *plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,nullptr);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
			connect(FMessageWidgets->instance(),SIGNAL(toolBarWidgetCreated(IMessageToolBarWidget *)),SLOT(onToolBarWidgetCreated(IMessageToolBarWidget *)));
	}

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,nullptr);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,nullptr);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return FMessageWidgets!=nullptr;
}

bool Emoji::initObjects()
{
	FCategoryNames.insert(People, tr("Smileys & People"));
	FCategoryNames.insert(Nature, tr("Animals & Nature"));
	FCategoryNames.insert(Foods, tr("Food & Drink"));
	FCategoryNames.insert(Travel, tr("Travel & Places"));
	FCategoryNames.insert(Symbols, tr("Symbols"));
	FCategoryNames.insert(Activity, tr("Activities"));
	FCategoryNames.insert(Objects, tr("Objects"));
	FCategoryNames.insert(Flags, tr("Flags"));

	FCategoryIDs.insert(People, "people");
	FCategoryIDs.insert(Nature, "nature");
	FCategoryIDs.insert(Foods, "food");
	FCategoryIDs.insert(Travel, "travel");
	FCategoryIDs.insert(Symbols, "symbols");
	FCategoryIDs.insert(Activity, "activity");
	FCategoryIDs.insert(Objects, "objects");
	FCategoryIDs.insert(Flags, "flags");

	if (FMessageProcessor)
		FMessageProcessor->insertMessageWriter(MWO_EMOJI,this);

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
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SIZE_CHAT, 32);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SIZE_MENU, 16);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SKINCOLOR, SkinDefault);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_CATEGORY, People);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_RECENT, QByteArray());

	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
	return true;
}

bool Emoji::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ALang);

	return false;
}

bool Emoji::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ALang);

	return replaceImageToText(ADocument);
}

bool Emoji::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ALang);

	return replaceTextToImage(ADocument);
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
		QList<QUrl> urlList = FUrlByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt()).values();
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
	QList<QString> iconsets;
	iconsets << Options::node(OPV_MESSAGES_EMOJI_ICONSET).value().toString();
	return iconsets;
}

QUrl Emoji::urlByKey(const QString &AKey) const
{
	return FUrlByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt()).value(AKey);
}

QString Emoji::keyByUrl(const QUrl &AUrl) const
{
	return FKeyByUrl.value(AUrl.toString());
}

QMap<int, QString> Emoji::findTextEmoticons(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> emoji;
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
							int keyLength = 0, keyLengthCurrent = 0;
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
								emoji.insert(fragment.position()+keyPos, key);
								keyPos += keyLength-1;
								key.clear();
							}
						}
				}
			}
	}
	return emoji;
}

QMap<int, QString> Emoji::findImageEmoticons(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> emoji;
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
					if (!key.isEmpty())
					{
						int i = 0;
						QString text = fragment.text();
						for (QString::ConstIterator itc=text.constBegin(); itc!=text.constEnd(); ++itc, ++i)
							if (*itc==QChar::ObjectReplacementCharacter)
								emoji.insert(fragment.position()+i,key);
					}
				}
			}
			block = block.next();
		}
	}
	return emoji;
}

QMap<uint, QString> Emoji::setEmoji(const QString &AEmojiSet) const
{
	return FEmojiSets.value(AEmojiSet);
}

QIcon Emoji::getIcon(const QString &AEmojiCode, const QSize &ASize) const
{
	QIcon icon;
	if (FIconHash.contains(AEmojiCode))
		icon = FIconHash[AEmojiCode];
	if (!icon.availableSizes().contains(ASize))
	{
		QHash<QString, QUrl> urlByKey = FUrlByKey.value(ASize.height());
		if (urlByKey.contains(AEmojiCode))
		{
			QFile file(urlByKey[AEmojiCode].toLocalFile());
			if (file.open(QIODevice::ReadOnly))
			{
				QImageReader reader(&file);
				if (!ASize.isNull())
					reader.setScaledSize(ASize);
				icon.addPixmap(QPixmap::fromImage(reader.read()));
				file.close();
				FIconHash.insert(AEmojiCode, icon);
			}
		}
	}
	return icon;
}

QIcon Emoji::getIconForSet(const QString &AEmojiSet, const QString &AEmojiText, const QSize &ASize) const
{
	if (FEmojiSets.contains(AEmojiSet) && FEmojiData.contains(AEmojiText))
	{
		QDir dir(FResourceDir);
		if (dir.cd(AEmojiSet) && dir.cd("png") && dir.cd(QString::number(ASize.height())))
		{
			QFile file(getFileName(FEmojiData.value(AEmojiText), dir));
			if (file.exists() && file.open(QIODevice::ReadOnly))
			{
				QImageReader reader(&file);
				reader.setScaledSize(ASize);
				QPixmap pixmap = QPixmap::fromImage(reader.read());
				if (!pixmap.isNull())
					return QIcon(pixmap);
			}
		}
	}
	return QIcon();
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
#if QT_VERSION < 0x050000
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
							emojiData.ucs4alt = val.property("unicode_alternates").toString();
							emojiData.ucs4 = val.property("unicode").toString();
							QList<QString> splitted = (emojiData.ucs4alt.isEmpty()?emojiData.ucs4:emojiData.ucs4alt).split('-');
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
#else
					QJsonDocument jsonDoc = QJsonDocument::fromJson(json);
					if (jsonDoc.isObject())
					{
						QJsonObject object = jsonDoc.object();
						for (QJsonObject::ConstIterator it = object.constBegin(); it!=object.constEnd(); ++it)
						{
							EmojiData emojiData;

							emojiData.id = it.key();
							QJsonObject object = (*it).toObject();

							emojiData.name = object.value("name").toString();
							QString category = object.value("category").toString();
							int order = object.value("order").toInt();

							if (!object.value("ascii").isNull())
							{
								QVariantList list = object.value("ascii").toVariant().toList();
								QList<QString> aliases;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
									aliases.append((*it).toString());
								emojiData.aliases = aliases;
							}

							if (!object.value("keywords").isNull())
							{
								QVariantList list = object.value("keywords").toVariant().toList();
								QString out;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
								{
									if (!out.isEmpty())
										out.append(',');
									out.append((*it).toString());
								}
							}

							uint ucs4[16];
							int  i(0);

							QJsonObject codePoints = object.value("code_points").toObject();
							emojiData.ucs4 = codePoints.value("fully_qualified").toString();

							qDebug() << "ucs4=" << emojiData.ucs4;

							bool ok;
							QJsonValue diversity = object.value("diversity");
							emojiData.diversity = diversity.isNull()?0:diversity.toString().toInt(&ok, 16);
							if (emojiData.diversity)
								qDebug() << "diversity=" << QString::number(emojiData.diversity, 16);

							QJsonValue gender = object.value("gender");
							emojiData.gender = gender.isNull()?0:gender.toString().toInt(&ok, 16);
							if (emojiData.gender)
								qDebug() << "gender=" << QString::number(emojiData.gender, 16);

							if (!emojiData.diversity && !emojiData.gender)
							{
								QJsonArray diversities = object.value("diversities").toArray();
								for (QJsonArray::ConstIterator it = diversities.constBegin();
									 it != diversities.constEnd(); ++it)
									emojiData.diversities.append((*it).toString());

								bool ok(false);
								QList<QString> splitted = emojiData.id.split('-');
								for (QList<QString>::ConstIterator it=splitted.constBegin(); it!=splitted.constEnd(); ++it, ++i)
								{
									ucs4[i]=(*it).toInt(&ok, 16);
									if (!ok)
										break;
								}
								if (ok)
								{
									emojiData.unicode = QString::fromUcs4(ucs4, i);
									FCategories[(Category)FCategoryIDs.key(category)].insert(order, emojiData);
									FEmojiData.insert(emojiData.unicode, emojiData);
								}
							}
						}
#endif
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
									QPixmap pixmap = QPixmap::fromImage(reader.read());
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
							QStringList emojiSets = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
							for (QStringList::ConstIterator its = emojiSets.constBegin(); its!=emojiSets.constEnd(); ++its)
							{
								QDir set(dir);
								if (set.cd(*its) && set.cd("png"))
								{
									QStringList entries = set.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
									QList<int> sizes;
									QMap<uint, QString> setEmoji;
									for (QStringList::ConstIterator it=entries.constBegin(); it!=entries.constEnd(); ++it)
									{
										int size = (*it).toInt();
										if (size && set.cd(*it))
										{
											QStringList fileNames = set.entryList(QDir::Files);
											if (!fileNames.isEmpty())
											{
												sizes.append(size);
												if (setEmoji.isEmpty())
													for (QHash<Category, QMap<uint, EmojiData> >::Iterator itc=FCategories.begin(); itc!=FCategories.end(); ++itc)
													{
														FCategoryCount[itc.key()]=0;
														for (QMap<uint, EmojiData>::Iterator it = (*itc).begin(); it!=(*itc).end(); ++it)
														{
															QFile file(getFileName(*it, set));
															if (file.exists())
																setEmoji.insert(it.key(), (*it).unicode);
														}
													}
											}
											set.cdUp();
										}
									}
									if (!sizes.isEmpty() && !setEmoji.isEmpty())
									{
										FEmojiSets.insert(*its, setEmoji);
										FAvailableSizes.insert(*its, sizes);
									}
								}
							}
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
					LOG_WARNING("JSON is empty!");
			}
			else
				LOG_WARNING("failed to open file!");
			dir.cdUp();
		}
	}
}

void Emoji::loadEmojiSet(const QString &AEmojiSet)
{
	QDir dir(FResourceDir);
	dir.cd(AEmojiSet);
	dir.cd("png");

	FIconHash.clear();
	FUrlByKey.clear();
	FKeyByUrl.clear();
	clearTreeItem(&FRootTreeItem);

	if (!AEmojiSet.isEmpty())
	{
		QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
		for (QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it)
			if (!(*it).toInt())
				it = dirs.erase(it);

		if (!dirs.isEmpty())
		{
			for (QStringList::ConstIterator itd = dirs.constBegin(); itd!=dirs.constEnd(); ++itd)
				if (dir.cd(*itd))
				{
					for (QHash<Category, QMap<uint, EmojiData> >::Iterator itc=FCategories.begin(); itc!=FCategories.end(); ++itc)
					{
						FCategoryCount[itc.key()]=0;
						for (QMap<uint, EmojiData>::Iterator it = (*itc).begin(); it!=(*itc).end(); ++it)
						{
							QFile file(getFileName(*it, dir));
							if (file.exists())
							{
								QUrl url(QUrl::fromLocalFile(file.fileName()));
//								(*it).colored = false;
								for (uint c=0x1f3fb; c<=0x1f3ff; ++c)
								{
									QFile file(dir.absoluteFilePath(QString("%1-%2.png").arg((*it).ucs4).arg(c, 0, 16)));
									if (!file.exists())
										file.setFileName(dir.absoluteFilePath(QString("%1-%2.png").arg((*it).id).arg(c, 0, 16)));
									if (file.exists())
									{
//										(*it).colored = true;
										QString unicode = (*it).unicode+QString::fromUcs4(&c, 1);
										QUrl url(QUrl::fromLocalFile(file.fileName()));
										FUrlByKey[(*itd).toInt()].insert(unicode, url);
										FKeyByUrl.insertMulti(url.toString(), unicode);
										createTreeItem(unicode, url);
									}
								}
								FCategoryCount[itc.key()]++;
								FUrlByKey[(*itd).toInt()].insert((*it).unicode, url);
								FKeyByUrl.insertMulti(url.toString(), (*it).unicode);
								createTreeItem((*it).unicode, url);
								(*it).present = true;
							}
							else
								(*it).present = false;
						}

						for (uint c=0x1f3fb; c<=0x1f3ff; ++c)
						{
							QFile file(dir.absoluteFilePath(QString("%1.png").arg(c, 0, 16)));
							if (file.exists())
							{
								QString unicode = QString::fromUcs4(&c, 1);
								QUrl url(QUrl::fromLocalFile(file.fileName()));
								FUrlByKey[(*itd).toInt()].insert(unicode, url);
								FKeyByUrl.insertMulti(url.toString(), unicode);
							}
						}
					}
					dir.cdUp();
				}
		}
		updateSize(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT));
		updateSize(Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU));
	}
	updateSelectIconMenu(AEmojiSet);
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
	QMap<int,QString> emoji = findTextEmoticons(ADocument,AStartPos,ALength);
	if (!emoji.isEmpty())
	{
		QTextCursor cursor(ADocument);
		cursor.beginEditBlock();
		for (QMap<int,QString>::const_iterator it=emoji.constBegin(); it!=emoji.constEnd(); ++it)
		{
			QUrl url = FUrlByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt()).value(it.value());
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

SelectIconMenu *Emoji::createSelectIconMenu(const QString &AIconSet, QWidget *AParent)
{
	SelectIconMenu *menu = new SelectIconMenu(AIconSet, this, AParent);
	connect(menu->instance(),SIGNAL(iconSelected(QString, QString)), SLOT(onSelectIconMenuSelected(QString, QString)));
	connect(menu->instance(),SIGNAL(destroyed(QObject *)),SLOT(onSelectIconMenuDestroyed(QObject *)));
	return menu;
}

void Emoji::updateSelectIconMenu(const QString &AIconSet)
{
	for(QList<IMessageToolBarWidget *>::ConstIterator it=FToolBarsWidgets.constBegin(); it!=FToolBarsWidgets.constEnd(); ++it)
	{
		Action *action = nullptr;
		ToolBarChanger *changer  = (*it)->toolBarChanger();
		QList<QAction *> actions = changer->groupItems(TBG_MWTBW_EMOJI);
		if (!actions.isEmpty())
			action = changer->handleAction(actions.first());
		if (action)
		{
			SelectIconMenu *menu = qobject_cast<SelectIconMenu *>(action->menu());
			if (menu)
			{
				changer->removeItem(actions.first());
				FToolBarWidgetByMenu.remove(menu);
			}
		}

		if (!AIconSet.isEmpty())
		{
			SelectIconMenu *menu = createSelectIconMenu(AIconSet, (*it)->instance());
			FToolBarWidgetByMenu.insert(menu, (*it));
			QToolButton *button = changer->insertAction(menu->menuAction(),TBG_MWTBW_EMOJI);
			button->setPopupMode(QToolButton::InstantPopup);
		}
	}
}

QString Emoji::getFileName(const EmojiData &AEmojiData, const QDir &ADir) const
{
	QFile file;
	if (AEmojiData.id.isNull())
		file.setFileName(ADir.absoluteFilePath(AEmojiData.ucs4+".png"));
	else
	{
		file.setFileName(ADir.absoluteFilePath(AEmojiData.id+".png"));
		if (!file.exists())
			file.setFileName(ADir.absoluteFilePath(AEmojiData.ucs4+".png"));
	}
	return QDir::cleanPath(file.fileName());
}

void Emoji::updateSize(OptionsNode ANode)
{
	bool ok;
	int value = ANode.value().toInt(&ok);
	if (ok)
	{
		if (!FUrlByKey.contains(value))
		{
			QList<int> keys = FUrlByKey.keys();
			int shift = abs(keys.first()-value);
			int val;
			for (QList<int>::ConstIterator it=keys.constBegin(); it!=keys.constEnd(); ++it)
			{
				if (abs(*it-value)<shift)
				{
					shift = abs(*it-value);
					val = *it;
				}
			}
			ANode.setValue(val);
		}
	}
}

void Emoji::onToolBarWindowLayoutChanged()
{
	IMessageWindow *window = qobject_cast<IMessageWindow *>(sender());
	if (window && window->toolBarWidget())
		foreach(QAction *handle, window->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_EMOJI))
			handle->setVisible(window->editWidget()->isVisibleOnWindow());
}

void Emoji::onToolBarWidgetCreated(IMessageToolBarWidget *AWidget)
{
	if (AWidget->messageWindow()->editWidget())
	{
		QString iconSet = Options::node(OPV_MESSAGES_EMOJI_ICONSET).value().toString();
		if (!iconSet.isEmpty())
		{
			FToolBarsWidgets.append(AWidget);
			if (AWidget->messageWindow()->editWidget()->isVisibleOnWindow())
			{
				SelectIconMenu *menu = createSelectIconMenu(iconSet, AWidget->instance());
				FToolBarWidgetByMenu.insert(menu,AWidget);
				QToolButton *button = AWidget->toolBarChanger()->insertAction(menu->menuAction(), TBG_MWTBW_EMOJI);
				button->setPopupMode(QToolButton::InstantPopup);
			}
			connect(AWidget->instance(),SIGNAL(destroyed(QObject *)),SLOT(onToolBarWidgetDestroyed(QObject *)));
			connect(AWidget->messageWindow()->instance(),SIGNAL(widgetLayoutChanged()),SLOT(onToolBarWindowLayoutChanged()));
		}
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
	Q_UNUSED(AIconText)

	SelectIconMenu *menu = qobject_cast<SelectIconMenu *>(sender());
	if (FToolBarWidgetByMenu.contains(menu))
	{
		IMessageEditWidget *widget = FToolBarWidgetByMenu.value(menu)->messageWindow()->editWidget();
		if (widget)
		{
			QUrl url = FUrlByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt()).value(AIconKey);
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
	OptionsNode iconset = Options::node(OPV_MESSAGES_EMOJI_ICONSET);
	if (FEmojiSets.contains(iconset.value().toString()))
		onOptionsChanged(iconset);
	else if (FEmojiSets.isEmpty())
		iconset.setValue(QString(""));
	else
		iconset.setValue(emojiSets().first());

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
		QString emojiSet(ANode.value().toString());
		if (FEmojiSets.contains(emojiSet))
			loadEmojiSet(emojiSet);
	}
	else if (ANode.path() == OPV_MESSAGES_EMOJI_SIZE_MENU)
		updateSelectIconMenu(Options::node(OPV_MESSAGES_EMOJI_ICONSET).value().toString());
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_emoji, Emoji)
#endif

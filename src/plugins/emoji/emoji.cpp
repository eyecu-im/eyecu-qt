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

//
// EmojiData class
//
EmojiData::~EmojiData()
{}

const QString &EmojiData::id() const
{
	return FId;
}

const QString &EmojiData::name() const
{
	return FName;
}

const QString &EmojiData::diversity() const
{
	return FDiversity;
}

const QString &EmojiData::gender() const
{
	return FGender;
}

const QStringList &EmojiData::diversities() const
{
	return FDiversities;
}

const QStringList &EmojiData::genders() const
{
	return FGenders;
}

bool EmojiData::variation() const
{
	return !(FDiversity.isNull() && FGender.isNull());
}

bool EmojiData::present() const
{
	return FPresent;
}

bool EmojiData::display() const
{
	return FDisplay;
}

//
// Emoji class
//
Emoji::Emoji():
	FMessageWidgets(nullptr),
	FMessageProcessor(nullptr),
	FOptionsManager(nullptr),
	FGenderSuffixes(QStringList() << "2642" << "2640"),
	FSkinColorSuffixes(QStringList() << "1f3fb" << "1f3fc"
									 << "1f3fd" << "1f3fe"
									 << "1f3ff")
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

	findEmojiSets();

	return true;
}

bool Emoji::isColored(const QString &AEmojiId) const
{
	if (FEmojiData.contains(AEmojiId))
		return !FEmojiData[AEmojiId].FDiversities.isEmpty();
	return false;
}

QString Emoji::genderSuffix(Gender AGender) const
{
	return FGenderSuffixes.value(AGender-1);
}

QString Emoji::skinColorSuffix(SkinColor ASkinColor) const
{
	return FSkinColorSuffixes.value(ASkinColor-1);
}


bool Emoji::initSettings()
{
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_ICONSET, EMOJI_DEFAULT);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SIZE_CHAT, 32);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SIZE_MENU, 16);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_SKINCOLOR, SkinDefault);
	Options::setDefaultValue(OPV_MESSAGES_EMOJI_GENDER, GenderMale);
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
		QList<QString> urlList = FFileByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt()).values();
		QTextBlock block = ADocument->firstBlock();
		while (block.isValid())
		{
			for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
			{
				QTextFragment fragment = it.fragment();
				if (fragment.charFormat().isImageFormat())
				{
					QUrl url = fragment.charFormat().toImageFormat().name();
					if (AWidget->document()->resource(QTextDocument::ImageResource,url).isNull() && FEmojiData.contains(url.path()))
					{
						AWidget->document()->addResource(QTextDocument::ImageResource,url,QImage(fileByKey(url.path())));
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

QString Emoji::fileByKey(const QString &AKey) const
{
	return FFileByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt()).value(AKey);
}

QMap<int, QString> Emoji::findTextEmoji(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> emoji;

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
						for (const EmojiTreeItem *item = &FRootTreeItem;
							 item && keyLengthCurrent<=searchText.length()-keyPos &&
							 fragment.position()+keyPos+keyLengthCurrent<=stopPos;
							 ++keyLengthCurrent)
						{
							if (!item->name.isEmpty())
								key = searchText.mid(keyPos, keyLength = keyLengthCurrent);
							if (keyPos+keyLengthCurrent<searchText.length())
								item = item->children.value(searchText.at(keyPos+keyLengthCurrent));
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
	return emoji;
}

QMap<int, QString> Emoji::findImageEmoji(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> emoji;
	QTextBlock block = ADocument->findBlock(AStartPos);
	int stopPos = ALength < 0 ? ADocument->characterCount() : AStartPos+ALength;
	while (block.isValid() && block.position()<stopPos)
	{
		for (QTextBlock::iterator it = block.begin(); !it.atEnd() && it.fragment().position()<stopPos; ++it)
		{
			const QTextFragment fragment = it.fragment();
			if (fragment.charFormat().isImageFormat())
			{
				QUrl url(fragment.charFormat().toImageFormat().name());
				if (url.isValid() && url.scheme()=="emoji" && FEmojiData.contains(url.path()))
				{
					int i = 0;
					QString text = fragment.text();
					for (QString::ConstIterator itc=text.constBegin();
						 itc!=text.constEnd(); ++itc, ++i)
						if (*itc==QChar::ObjectReplacementCharacter)
							emoji.insert(fragment.position()+i,
										 FEmojiData[url.path()].FUnicode);
				}
			}
		}
		block = block.next();
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
		if (AEmojiCode.isEmpty()) // Create empty icon
		{
			QPixmap pixmap(ASize);
			pixmap.fill(QColor(0, 0, 0, 0));
			icon.addPixmap(pixmap);
			FIconHash.insert(AEmojiCode, icon);
		}
		else
		{
			QHash<QString, QString> fileByKey = FFileByKey.value(ASize.height());
			if (fileByKey.contains(AEmojiCode))
			{
				QFile file(fileByKey[AEmojiCode]);
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

QMap<uint, IEmojiData *> Emoji::emojiData(Category ACategory) const
{
	return FCategories.value(ACategory);
}

const IEmojiData *Emoji::findData(const QString &AEmojiId, SkinColor ASkinColor, Gender AGender) const
{
	if (FEmojiData.contains(AEmojiId))
	{
		QHash<QString, EmojiData> &emojiData = const_cast<QHash<QString, EmojiData>&>(FEmojiData);
		EmojiData *data = &emojiData[AEmojiId];

		if (ASkinColor && !data->diversities().isEmpty())
		{
			QString colorSuffix = skinColorSuffix(ASkinColor);
			QStringList diversities = data->diversities();
			for(QStringList::ConstIterator itc = diversities.constBegin();
				itc != diversities.constEnd(); ++itc)
				if (emojiData.contains(*itc) && emojiData[*itc].diversity() == colorSuffix)
				{
					if (emojiData[*itc].FPresent)
						data = &emojiData[*itc];
					break;
				}
		}

		if (AGender && !data->genders().isEmpty())
		{
			QString _genderSuffix = genderSuffix(AGender);
			QStringList genders = data->genders();
			for(QStringList::ConstIterator itg = genders.constBegin();
				itg != genders.constEnd(); ++itg)
				if (emojiData.contains(*itg) && emojiData[*itg].gender() == _genderSuffix)
				{
					if (emojiData[*itg].FPresent)
						data = &emojiData[*itg];
					break;
				}
		}
		return data;
	}
	return nullptr;
}

void Emoji::findEmojiSets()
{
	QList<QString> resourceDirs = FileStorage::resourcesDirs();	
	for (QList<QString>::ConstIterator it=resourceDirs.constBegin();
		 it!=resourceDirs.constEnd(); ++it)
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
							emojiData.FId = it.name();
							QScriptValue val = it.value();
							emojiData.FName = val.property("name").toString();
							emojiData.FDisplay = val.property("display").toInt32() > 0; // Do not process symbols without "display" flag
							QString category = val.property("category").toString();
							emojiData.FCategory = FCategoryIDs.key(category);
							qint32 order = val.property("order").toInt32();

							if (!val.property("ascii").isNull())
							{
								QVariantList list=val.property("ascii").toVariant().toList();
								QList<QString> aliases;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
									aliases.append((*it).toString());
								emojiData.FAliases = aliases;
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

							emojiData.FUcs4 = val.property("code_points").property("fully_qualified").toString();

							if (val.property("diversity").isNull()) // May have diversities
							{
								QVariantList diversities = val.property("diversities").toVariant().toList();
								for (QVariantList::ConstIterator it = diversities.constBegin();
									 it != diversities.constEnd(); ++it)
									emojiData.FDiversities.append((*it).toString());
							}
							else
								emojiData.FDiversity = val.property("diversity").toString();

							if (val.property("gender").isNull()) // May have genders
							{
								QVariantList genders = val.property("genders").toVariant().toList();
								for (QVariantList::ConstIterator it = genders.constBegin();
									 it != genders.constEnd(); ++it)
									emojiData.FGenders.append((*it).toString());
							}
							else
								emojiData.FGender = val.property("gender").toString();
#else
					QJsonDocument jsonDoc = QJsonDocument::fromJson(json);
					if (jsonDoc.isObject())
					{
						QJsonObject object = jsonDoc.object();
						for (QJsonObject::ConstIterator it = object.constBegin();
							 it!=object.constEnd(); ++it)
						{
							QJsonObject object = (*it).toObject();								
							EmojiData emojiData;
							emojiData.FId = it.key();
							emojiData.FName = object.value("name").toString();
							emojiData.FDisplay = object.value("display").toInt() > 0; // Do not process symbols without "display" flag
							QString category = object.value("category").toString();
							emojiData.FCategory = FCategoryIDs.key(category);
							uint order = uint(object.value("order").toInt());

							if (!object.value("ascii").isNull())
							{
								QVariantList list = object.value("ascii").toVariant().toList();
								QList<QString> aliases;
								for (QVariantList::ConstIterator it=list.constBegin(); it!=list.constEnd(); it++)
									aliases.append((*it).toString());
								emojiData.FAliases = aliases;
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

							emojiData.FUcs4 = object.value("code_points").toObject().value("fully_qualified").toString();

							if (object.value("diversity").isNull()) // May have diversities
							{
								QJsonArray diversities = object.value("diversities").toArray();
								for (QJsonArray::ConstIterator it = diversities.constBegin();
									 it != diversities.constEnd(); ++it)
									emojiData.FDiversities.append((*it).toString());
							}
							else
								emojiData.FDiversity = object.value("diversity").toString();

							if (object.value("gender").isNull()) // May have genders
							{
								QJsonArray genders = object.value("genders").toArray();
								for (QJsonArray::ConstIterator it = genders.constBegin();
									 it != genders.constEnd(); ++it)
									emojiData.FGenders.append((*it).toString());
							}
							else
								emojiData.FGender = object.value("gender").toString();
#endif
							uint ucs4[16];
							bool ok(false);
							int  i(0);
							QList<QString> splitted = emojiData.FUcs4.split('-');
							for (QList<QString>::ConstIterator it=splitted.constBegin(); it!=splitted.constEnd(); ++it, ++i)
							{
								ucs4[i]=uint(it->toInt(&ok, 16));
								if (!ok)
									break;
							}
							if (ok)
							{								
								emojiData.FUnicode = QString::fromUcs4(ucs4, i);
								if (!emojiData.FDisplay && !emojiData.FGender.isEmpty())
									emojiData.FDisplay = true;
								FEmojiData.insert(emojiData.id(), emojiData);
								FIdByUnicode.insert(emojiData.FUnicode, emojiData.FId);
								if (emojiData.FDisplay && !emojiData.variation())
									FCategories[Category(emojiData.FCategory)]
											.insert(uint(order), &FEmojiData[emojiData.FId]);
							}
						}
#if QT_VERSION < 0x050000
						while (it.hasNext());
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
													for (QHash<Category, QMap<uint, IEmojiData*> >::Iterator itc=FCategories.begin();
														 itc!=FCategories.end(); ++itc)
													{
														FCategoryCount[itc.key()]=0;
														for (QMap<uint, IEmojiData*>::Iterator it = itc->begin();
															 it!=itc->end(); ++it)
														{
															const EmojiData &emojiData = *static_cast<EmojiData*>(*it);
															QFile file(getFileName(emojiData, set));
															if (file.exists())
																setEmoji.insert(it.key(), emojiData.FUnicode);
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
	FFileByKey.clear();
	clearTreeItem(&FRootTreeItem);

	if (!AEmojiSet.isEmpty())
	{
		QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
		for (QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it)
			if (!(*it).toInt())
				it = dirs.erase(it);

		if (!dirs.isEmpty())
		{
			for (QStringList::ConstIterator itd = dirs.constBegin();itd!=dirs.constEnd(); ++itd)
				if (dir.cd(*itd))
				{
					for (int c = People; c < Last; ++c)
						FCategoryCount[c] = 0;

					for (QHash<QString, EmojiData>::Iterator it = FEmojiData.begin();
						 it != FEmojiData.end(); ++it)
					{
						QFile file(getFileName(*it, dir));
						if (file.exists())
						{
							QString fileName(file.fileName());
							if (!it->variation())
								FCategoryCount[it->FCategory]++;
							FFileByKey[itd->toInt()].insert(it.key(), fileName);
							createTreeItem(it->FUnicode, fileName);
							it->FPresent = true;
						}
						else
							it->FPresent = false;
					}

					dir.cdUp();
				}
		}
		updateSize(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT));
		updateSize(Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU));
	}
	updateSelectIconMenu(AEmojiSet);
}

void Emoji::createTreeItem(const QString &AKey, const QString &AName)
{
	EmojiTreeItem *item = &FRootTreeItem;
	for (int i=0; i<AKey.size(); i++)
	{
		QChar itemChar = AKey.at(i);
		if (!item->children.contains(itemChar))
		{
			EmojiTreeItem *childItem = new EmojiTreeItem;
			item->children.insert(itemChar,childItem);
			item = childItem;
		}
		else
		{
			item = item->children.value(itemChar);
		}
	}
	item->name = AName;
}

void Emoji::clearTreeItem(EmojiTreeItem *AItem) const
{
	foreach(const QChar &itemChar, AItem->children.keys())
	{
		EmojiTreeItem *childItem = AItem->children.take(itemChar);
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
	QMap<int,QString> emoji = findTextEmoji(ADocument,AStartPos,ALength);
	if (!emoji.isEmpty())
	{
		QTextCursor cursor(ADocument);
		cursor.beginEditBlock();
		for (QMap<int,QString>::const_iterator it=emoji.constBegin(); it!=emoji.constEnd(); ++it)
		{
			QString id = FIdByUnicode[*it];
			if (FEmojiData[id].FDisplay)
			{
				QString fileName = FFileByKey.value(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT)
													.value().toInt()).value(id);
				if (!fileName.isEmpty())
				{
					cursor.setPosition(it.key()-posOffset);
					cursor.setPosition(cursor.position()+it->length(), QTextCursor::KeepAnchor);

					QTextImageFormat format;
					QUrl url(QUrl::fromLocalFile(fileName));
					format.setName(url.toString());
					format.setToolTip(findData(id)->name());
					if (!ADocument->resource(QTextDocument::ImageResource,url).isValid())
						ADocument->addResource(QTextDocument::ImageResource, url, QImage(fileName));
					cursor.insertImage(format);
					posOffset += it->length()-1;
				}
			}
		}
		cursor.endEditBlock();
	}
	return posOffset;
}

int Emoji::replaceImageToText(QTextDocument *ADocument, int AStartPos, int ALength) const
{
	int posOffset = 0;
	QMap<int,QString> emoji = findImageEmoji(ADocument,AStartPos,ALength);
	if (!emoji.isEmpty())
	{
		QTextCursor cursor(ADocument);
		cursor.beginEditBlock();

		for (QMap<int,QString>::const_iterator it=emoji.constBegin(); it!=emoji.constEnd(); ++it)
		{
			cursor.setPosition(it.key()+posOffset);
			cursor.deleteChar();
			posOffset--;
			if (cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor, 1))
				cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor, 1);
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
	connect(menu,SIGNAL(iconSelected(QString)),SLOT(onSelectIconMenuSelected(QString)));
	connect(menu,SIGNAL(destroyed(QObject*)),SLOT(onSelectIconMenuDestroyed(QObject*)));
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
	if (AEmojiData.FId.isNull())
		file.setFileName(ADir.absoluteFilePath(AEmojiData.FUcs4+".png"));
	else
	{
		file.setFileName(ADir.absoluteFilePath(AEmojiData.FId+".png"));
		if (!file.exists())
			file.setFileName(ADir.absoluteFilePath(AEmojiData.FUcs4+".png"));
	}
	return QDir::cleanPath(file.fileName());
}

void Emoji::updateSize(OptionsNode ANode)
{
	bool ok;
	int value = ANode.value().toInt(&ok);
	if (ok)
	{
		if (!FFileByKey.contains(value))
		{
			QList<int> keys = FFileByKey.keys();
			int shift = abs(keys.first()-value);
			int val(0);
			for (QList<int>::ConstIterator it=keys.constBegin(); it!=keys.constEnd(); ++it)
				if (abs(*it-value)<shift)
				{
					shift = abs(*it-value);
					val = *it;
				}
			if (val)
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

void Emoji::onSelectIconMenuSelected(QString AIconId)
{
	SelectIconMenu *menu = qobject_cast<SelectIconMenu *>(sender());
	if (FToolBarWidgetByMenu.contains(menu))
	{
		IMessageEditWidget *widget = FToolBarWidgetByMenu.value(menu)->messageWindow()->editWidget();
		if (widget)
		{
			// Calculate correct icon id for current skin color
			QString iconId = findData(AIconId,
									  SkinColor(Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt()),
									  Gender(Options::node(OPV_MESSAGES_EMOJI_GENDER).value().toInt()))->id();

			QString fileName = FFileByKey.value(
						Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT)
						.value().toInt()).value(iconId);

			if (!fileName.isEmpty())
			{
				QTextEdit *editor = widget->textEdit();
				QTextCursor cursor = editor->textCursor();
				cursor.beginEditBlock();

				if (cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,1))
					cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);

				QUrl url;
				url.setScheme("emoji");
				url.setPath(iconId);
				if (!editor->document()->resource(QTextDocument::ImageResource,url).isValid())
					editor->document()->addResource(QTextDocument::ImageResource,url,QImage(fileName));
				QTextImageFormat imageFormat;
				imageFormat.setName(url.toString());
				cursor.insertImage(imageFormat);

				cursor.endEditBlock();
				editor->setFocus();

				QStringList recent = FRecent;

				if (recent.isEmpty() || recent.first()!=AIconId)
				{
					if (recent.contains(AIconId))
						recent.removeOne(AIconId);
					recent.insert(0, AIconId);
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

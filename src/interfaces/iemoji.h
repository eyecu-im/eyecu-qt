#ifndef IEMOJI_H
#define IEMOJI_H

#include "iemoticons.h"

#define EMOJI_UUID "{3FCAB06C-B748-BFAC-839B-24F8B6A75D91}"

struct EmojiData {
	QString id;
	QString unicode;
	QString name;
	QString ucs4;
//	QString ucs4alt;
	QList<QString> aliases;
	QList<QString> diversities;
	int		diversity;
	int		gender;
//	bool	colored;
	bool	present;
};

class IEmoji: public IEmoticons
{
public:
	enum Category {
		People,
		Nature,
		Foods,
		Travel,
		Symbols,
		Activity,
		Objects,
		Flags,
		Last
	};

	enum SkinColor {
		SkinDefault,
		SkinTone1,
		SkinTone2,
		SkinTone3,
		SkinTone4,
		SkinTone5
	};

	virtual QString categoryName(Category ACategory) const = 0;
	virtual QIcon categoryIcon(Category ACategory) const = 0;
	virtual QIcon getIcon(const QString &AEmojiCode, const QSize &ASize=QSize()) const = 0;
	virtual QIcon getIconForSet(const QString &AEmojiSet, const QString &AEmojiText, const QSize &ASize=QSize()) const = 0;
	virtual QMap<uint, EmojiData> emojiData(Category ACategory) const = 0;
	virtual EmojiData findData(const QString &AEmojiCode) const = 0;
	virtual bool isColored(const QString &AEmojiText) const = 0;
	virtual const QStringList &colorSuffixes() const = 0;
	virtual int categoryCount(Category ACategory) const = 0;
	virtual QStringList emojiSets() const = 0;
	virtual QList<int> availableSizes(const QString &ASetName) const = 0;
	virtual QMap<uint, QString> setEmoji(const QString &AEmojiSet) const = 0;
};

Q_DECLARE_INTERFACE(IEmoji,"RWS.Plugin.IEmoji/1.0")

#endif

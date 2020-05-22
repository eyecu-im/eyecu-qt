#ifndef IEMOJI_H
#define IEMOJI_H

#include <QUrl>
#include <QIcon>
#include <QTextDocument>

#define EMOJI_UUID "{3FCAB06C-B748-BFAC-839B-24F8B6A75D91}"

class IEmojiData {
public:
	virtual const QString &id() const =0;
	virtual const QString &name() const =0;
	virtual const QString &diversity() const =0;
	virtual const QString &gender() const =0;
	virtual const QStringList &diversities() const =0;
	virtual const QStringList &genders() const =0;	

	virtual bool variation() const =0;
	virtual bool present() const =0;
	virtual bool display() const =0;
};

class IEmoji // : public IEmoticons
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

	enum Gender {
		GenderDefault,
		GenderMale,
		GenderFemale
	};

	virtual QString fileByKey(const QString &AKey) const = 0;
	virtual QMap<int, QString> findTextEmoji(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const = 0;
	virtual QMap<int, QString> findImageEmoji(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const = 0;
	virtual QStringList recentIcons(const QString &ASetName) const = 0;
	virtual QMap<uint, QString> setEmoji(const QString &AEmojiSet) const = 0;
	virtual QString categoryName(Category ACategory) const = 0;
	virtual QIcon categoryIcon(Category ACategory) const = 0;
	virtual QIcon getIcon(const QString &AEmojiCode, const QSize &ASize=QSize()) const = 0;
	virtual QIcon getIconForSet(const QString &AEmojiSet, const QString &AEmojiText, const QSize &ASize=QSize()) const = 0;
	virtual QMap<uint, IEmojiData*> emojiData(Category ACategory) const = 0;
	virtual const IEmojiData *findData(const QString &AEmojiId, SkinColor ASkinColor=SkinDefault, Gender AGender=GenderDefault) const = 0;
	virtual bool isColored(const QString &AEmojiId) const = 0;
	virtual unsigned categoryCount(Category ACategory) const = 0;
	virtual QStringList emojiSets() const = 0;
	virtual QList<int> availableSizes(const QString &ASetName) const = 0;
	virtual QString genderSuffix(Gender AGender) const =0;
	virtual QString skinColorSuffix(SkinColor ASkinColor) const =0;
};

Q_DECLARE_INTERFACE(IEmoji,"RWS.Plugin.IEmoji/1.0")

#endif

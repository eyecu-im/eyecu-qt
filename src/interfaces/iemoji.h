#ifndef IEMOJI_H
#define IEMOJI_H

#include "iemoticons.h"

#define EMOJI_UUID "{3FCAB06C-B748-BFAC-839B-24F8B6A75D91}"

struct EmojiData {
	QString id;
	QString unicode;
	QString name;
	QList<QString> aliases;
	bool	colored;
};

class IEmoji: public IEmoticons
{
public:
	virtual QIcon getIcon(const QString &AEmojiCode, const QSize &ASize=QSize()) const = 0;
	virtual QList<QString> categories() const = 0;
	virtual QMap<uint, EmojiData> emojiData(const QString &ACategory) const = 0;
	virtual EmojiData findData(const QString &AEmojiCode) const = 0;
	virtual bool isColored(const QString &AEmojiText) const = 0;
	virtual const QStringList &colorSuffixes() const = 0;
};

Q_DECLARE_INTERFACE(IEmoji,"RWS.Plugin.IEmoji/1.0")

#endif

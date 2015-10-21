#ifndef IEMOJI_H
#define IEMOJI_H

#include "iemoticons.h"

#define EMOJI_UUID "{3FCAB06C-B748-BFAC-839B-24F8B6A75D91}"

class IEmoji: public IEmoticons
{
public:
	virtual bool isColored(const QString &AEmojiText) const = 0;
	virtual const QStringList &colorSuffixes() const = 0;
};

Q_DECLARE_INTERFACE(IEmoji,"RWS.Plugin.IEmoji/1.0")

#endif

#ifndef IAVATARS_H
#define IAVATARS_H

#include <QImage>
#include <utils/jid.h>

#define AVATARTS_UUID "{22F84EAF-683E-4a20-B5E5-1FE363FD206C}"

class IAvatars
{
public:
	enum SizeType {
		AvatarSmall,
		AvatarNormal,
		AvatarLarge
	};
// *** <<< eyeCU <<< ***
	enum ShowAvatar
	{
		Right,
		Left
	};
// *** >>> eyeCU >>> ***
public:
	virtual QObject *instance() =0;
	virtual quint8 avatarSize(int AType) const =0;
	virtual bool hasAvatar(const QString &AHash) const =0;
	virtual QString avatarFileName(const QString &AHash) const =0;
	virtual QString avatarHash(const Jid &AContactJid, bool AExact=false) const =0;
	virtual QString saveAvatarData(const QByteArray &AData) const =0;
	virtual QByteArray loadAvatarData(const QString &AHash) const =0;
	virtual bool setAvatar(const Jid &AStreamJid, const QByteArray &AData) =0;
	virtual QString setCustomPictire(const Jid &AContactJid, const QByteArray &AData) =0;
	virtual QImage emptyAvatarImage(quint8 ASize, bool AGray=false) const =0;
	virtual QImage cachedAvatarImage(const QString &AHash, quint8 ASize, bool AGray=false) const =0;
	virtual QImage loadAvatarImage(const QString &AHash, quint8 ASize, bool AGray=false) const =0;
	virtual QImage visibleAvatarImage(const QString &AHash, quint8 ASize, bool AGray=false, bool ADummy=false, qreal ARound=0) const =0;
protected:
	virtual void avatarChanged(const Jid &AContactJid) =0;
};

Q_DECLARE_INTERFACE(IAvatars,"Vacuum.Plugin.IAvatars/1.3")

#endif // IAVATARS_H

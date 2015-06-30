#ifndef ICLIENTICONS_H
#define ICLIENTICONS_H

#include <QString>
#include <QIcon>

#include <utils/jid.h>

#define CLIENTICONS_UUID "{8357ed60-b69b-11e2-9e96-0800200c9a66}"

struct Client
{
	QString name;
	QIcon icon;
};

class IClientIcons
{
public:
	virtual QObject *instance() = 0;
	virtual quint32 rosterLabelId() const = 0;
	virtual QIcon iconByKey(const QString &key) const = 0;
	virtual QString clientByKey(const QString &key) const = 0;
	virtual QString contactClient(const Jid &contactJid) const = 0;
	virtual QIcon contactIcon(const Jid &contactJid) const = 0;
};

Q_DECLARE_INTERFACE(IClientIcons,"RWS.Plugin.IClientIcons/0.1")

#endif // ICLIENTICONS_H

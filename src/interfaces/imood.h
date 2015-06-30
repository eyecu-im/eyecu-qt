#ifndef IMOOD_H
#define IMOOD_H

#include <QObject>
#include <QIcon>
#include <utils/jid.h>

#define MOOD_UUID "{C3B6DBC9-20C8-3A90-B0BD-588AB71903DC}"

class IMood {
public:
    virtual QObject *instance() =0;
    virtual QIcon getIcon(const QString &moodName) const =0;
    virtual QString getIconFileName(const QString &moodName) const =0;
    virtual QString getIconName(const Jid &AContactJid) const =0;
    virtual QString getText(const Jid &AContactJid) const =0;
    virtual QString getLabel(const Jid &AContactJid) const =0;
};

Q_DECLARE_INTERFACE(IMood, "RWS.Plugin.IMood/1.0")

#endif	//IMOOD_H

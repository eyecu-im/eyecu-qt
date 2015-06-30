#ifndef IACTIVITY_H
#define IACTIVITY_H

#include <QObject>
#include <QIcon>
#include <utils/jid.h>

#define ACTIVITY_UUID "{11917DD1-537C-3104-A7E0-244DEDF953FD}"
 
class IActivity {
public:
	virtual QObject *instance() =0;
    virtual QIcon getIcon(const QString &AActivityName) const =0;
    virtual QString getIconFileName(const QString &AActivityName) const =0;
    virtual QString getIconName(const Jid &AContactJid) const =0;
    virtual QString getText(const Jid &AContactJid) const =0;
    virtual QString getLabel(const Jid &AContactJid) const =0;
    virtual QString getActivityText(const QString &AActivityName) const=0;
};

Q_DECLARE_INTERFACE(IActivity, "RWS.Plugin.IActivity/1.0")

#endif	//IACTIVITY_H

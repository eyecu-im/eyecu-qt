#ifndef IMAPCONTACTS_H
#define IMAPCONTACTS_H

#include <QObject>

#define MAPCONTACTS_UUID "{9f72c3d8-ab45-998e-b2d4-9803b5c877af}"

class IMapContacts
{
public:
    virtual QObject *instance() =0;
    virtual QStringList getFullJidList(const QString &bareJid) const =0;
    virtual bool hasGeoloc(QString &AJid) const =0;

    virtual void showContact(QString AJid, bool AShowMap=true) const =0;
    virtual void onGeolocActionTriggered() const =0;

protected:
    virtual void contactShowedOnTheMap(const QString &AJid) = 0;
};

Q_DECLARE_INTERFACE(IMapContacts, "eyeCU.Plugin.IMapContacts/1.0")

#endif	//IMAPCONTACTS_H

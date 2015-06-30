#ifndef IPOI_H
#define IPOI_H

#include <QDomElement>
#include <QIcon>
#include <QHash>
#include <QTreeWidget>
#include <utils/jid.h>
#include <utils/action.h>
#include <QtGeo/geolocelement.h>

#define POI_UUID "{d2a856c7-8f74-4e95-9aba-b95f4fb42f00}"

// typedef QHash<QString, QString> GeolocElement;
// Q_DECLARE_METATYPE(GeolocElement)

typedef QHash<QString, GeolocElement> PoiHash;

class IPoi
{
public:
    enum ActionDataRoles
    {
        ADR_STREAM_JID=Action::DR_StreamJid,
        ADR_CONTACT_JID=Action::DR_Parametr4,
        ADR_MESSAGE_TYPE=Action::DR_UserDefined,
        ADR_ID=Action::DR_Parametr2,
        ADR_ACTION=Action::DR_Parametr1
    };

    enum PoiAction
    {
        PA_NULL,
        PA_SHOW,
        PA_EDIT,
        PA_OPENURL,
        PA_DELETE,
        PA_REMOVE,
        PA_SAVE
    };

    enum ActionGroup
    {
        AG_TRANSIENT = AG_DEFAULT,
        AG_PERSISTENT
    };

    enum PoiModificationType
    {
        PMT_ADDED,
        PMT_REMOVED,
        PMT_MODIFIED
    };

    enum PoiDataRoles
    {
        PDR_ID=Qt::UserRole
    };


    virtual QObject *instance() =0;
    virtual QIcon   getIcon(const QString &AName) const =0;
    virtual QString getIconFileName(const QString &AName) const =0;
    virtual QIcon   getTypeIcon(const QString &AType) const =0;
    virtual QString getTypeIconFileName(const QString &AType) const =0;
    virtual QString getFullType(const QString &AType, const QString &AClass=QString()) const =0;
    virtual GeolocElement getPoi(const QString &AId) const =0;
    virtual bool    putPoi(const QString &AId, const GeolocElement &APoiData, const QString &ABareJid) =0;
    virtual bool    putPoi(const QString &AId, const GeolocElement &APoiData, bool AShow=false) =0;
    virtual void    insertMessagePoi(const Jid &AStreamJid, const Jid &AContactJid, int AMessageType, const GeolocElement &APoi) =0;
    virtual void    showSinglePoi(QString AId) =0;
    virtual void    hideOnePoi(QString AId) =0;
    virtual bool    removePoi(const QString &AId) =0;
    virtual Action *addMenuAction(QString text, QString icon, QString keyIcon) =0;
    virtual void    removeMenuAction(Action *action) =0;
	virtual bool    contextMenu(const QString &AId, Menu *AMenu) =0;
    virtual bool    insertPoiShortcut(const QString &AShortcutId) =0;
    virtual void    setTreeWidgetShortcuts(QTreeWidget *ATreeWidget, bool ATemporary=false) =0;
    virtual QString getCoordString(const GeolocElement &APoiData) const =0;
    virtual void    poiShow(const QString &APoiId) const =0;

protected:
    virtual void poisLoaded(const QString &AStreamBareJid, const PoiHash &APoiHash) =0;
    virtual void poisSaved(const QString &AStreamBareJid, const PoiHash &APoiHash) =0;
    virtual void poisRemoved(const QString &AStreamBareJid) =0;
    virtual void poiModified(const QString &AId, int AType) =0;
};

Q_DECLARE_INTERFACE(IPoi,"RWS.Plugin.IPoi/1.0")

#endif	//IPOI_H

#ifndef MESSAGEPOILIST_H
#define MESSAGEPOILIST_H

#include <QTreeWidget>
#include <utils/jid.h>
#include <utils/message.h>
#include <utils/iconstorage.h>
#include <utils/action.h>

#include <interfaces/imaplocationselector.h>
#include <interfaces/iaccountmanager.h>

#include "poi.h"

class MessagePoiList : public QTreeWidget
{
    Q_OBJECT
public:
    enum ItemDataRoles
    {
        IDR_POI=Qt::UserRole
    };

    MessagePoiList(Poi *APoi, IMapLocationSelector *AMapLocationSelector, QList<IAccount *> *AAccounts, IconStorage *AIconStorage, QWidget *parent = 0);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    void    addPoi(const GeolocElement &APoi);

protected:
    void setItemData(QTreeWidgetItem *AItem, const GeolocElement &APoi);

protected slots:
    void    onItemClicked(QTreeWidgetItem *AItem, int AColumn);
    void    onShortcutActivated(const QString &AId, QWidget *AWidget);

private:    
    Poi                  *FPoi;
    IMapLocationSelector *FMapLocationSelector;
    QList<IAccount *>    *FAccounts;
    IconStorage          *FIconStorage;
};

#endif // MESSAGEPOILIST_H

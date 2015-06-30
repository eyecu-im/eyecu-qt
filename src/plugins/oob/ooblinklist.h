#ifndef OOBLINKLIST_H
#define OOBLINKLIST_H

#include <QTreeWidget>
#include <utils/iconstorage.h>

#include "newlink.h"

class OobLinkList : public QTreeWidget
{
    Q_OBJECT
public:
    enum ItemDataRoles
    {
        IDR_URL=Qt::UserRole,
        IDR_DESCRIPTION
    };

    OobLinkList(IconStorage *AIconStorage, QWidget *parent = 0);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    void    addLink(const QUrl &AUrl, const QString &ADescription=QString());

protected:
    static void setItemData(QTreeWidgetItem *AItem, const QUrl &AUrl, const QString &ADescription=QString());

protected slots:
    void    onItemClicked(QTreeWidgetItem *AItem, int AColumn);
    void    onShortcutActivated(const QString &AId, QWidget *AWidget);

private:
    IconStorage *FIconStorage;
};

#endif // OOBLINKLIST_H

#include <QHeaderView>
#include <QVBoxLayout>

#include <interfaces/imessagewidgets.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/messagechatwindowwidgets.h>
#include <definitions/messagenormalwindowwidgets.h>
#include "utils/qt4qt5compat.h"

#include "messagepoilist.h"
#include "newpoi.h"

MessagePoiList::MessagePoiList(Poi *APoi, IMapLocationSelector *AMapLocationSelector, QList<IAccount *> *AAccounts, IconStorage *AIconStorage, QWidget *parent) :
    QTreeWidget(parent), FPoi(APoi), FMapLocationSelector(AMapLocationSelector), FAccounts(AAccounts), FIconStorage(AIconStorage)
{
    setRootIsDecorated(false);
    setColumnCount(5);
    setSortingEnabled(false);
    setHeaderHidden(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QHeaderView *headerView=header();
    headerView->setStretchLastSection(false);
	headerView->SETRESIZEMODE(QHeaderView::Fixed);
	headerView->SETRESIZEMODE(0, QHeaderView::ResizeToContents);
	headerView->SETRESIZEMODE(1, QHeaderView::ResizeToContents);
	headerView->SETRESIZEMODE(2, QHeaderView::Stretch);
    headerView->resizeSection(3, 18);
    headerView->resizeSection(4, 18);

    IMessageChatWindow *chatWindow=qobject_cast<IMessageChatWindow *>(parent);
    if (chatWindow)   // Chat Window
        chatWindow->messageWidgetsBox()->insertWidget(MCWW_POILISTWIDGET, this);
    else              // Normal Window
    {
        IMessageNormalWindow *normalWindow=qobject_cast<IMessageNormalWindow *>(parent);
        if (normalWindow)
            normalWindow->messageWidgetsBox()->insertWidget(MNWW_POILISTWIDGET, this);
    }
    // Initialize shortcuts
    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_POI_DELETE, this);
    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_POI_EDIT, this);
    connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(onItemClicked(QTreeWidgetItem*, int)));    
    hide();
}

QSize MessagePoiList::sizeHint() const
{
    QMargins margins=contentsMargins();
    QSize s=QTreeWidget::sizeHint();
    int h=margins.top()+margins.bottom();
    int count=topLevelItemCount();
    for (int i=0; i<count; i++)
    {
        QRect rect=visualItemRect(topLevelItem(i));
        h+=rect.height();
    }
    s.setHeight(h);
    return s;
}

QSize MessagePoiList::minimumSizeHint() const
{
    return sizeHint();
}

void MessagePoiList::addPoi(const GeolocElement &APoi)
{
    QTreeWidgetItem *item;
    if (topLevelItemCount())
        item=topLevelItem(0);
    else
    {
        item= new QTreeWidgetItem();
        QFont font;
        font.setBold(true);
        item->setFont(0, font);
        font.setBold(false);
        font.setItalic(true);
        item->setFont(1, font);
		item->setIcon(3, FIconStorage->getIcon(MNI_EDIT));
        item->setSizeHint(3, QSize(18,18));        
        item->setToolTip(3, tr("Edit"));
        item->setIcon(4, FIconStorage->getIcon(MNI_EDIT_DELETE));
        item->setSizeHint(4, QSize(18,18));
        item->setToolTip(4, tr("Delete"));
        addTopLevelItem(item);
    }

    setItemData(item, APoi);

    if (isHidden())
        setVisible(true);
    updateGeometry();
}

void MessagePoiList::onItemClicked(QTreeWidgetItem *AItem, int AColumn)
{
    if (AItem)
        switch (AColumn)
        {
            case 3: // Edit
            {
                NewPoi *newPoi = new NewPoi(FPoi, FMapLocationSelector, *FAccounts, tr("Edit point of interest"), AItem->data(1, IDR_POI).value<GeolocElement>(), parentWidget());
                if(newPoi->exec())
                {
                    setItemData(AItem, newPoi->getPoi());
                    updateGeometry();
                }
                newPoi->deleteLater();
                break;
            }

            case 4: // Delete
            {
                takeTopLevelItem(indexOfTopLevelItem(AItem));
                if (!topLevelItemCount())
                    hide();
                updateGeometry();
                break;
            }
        }
}

void MessagePoiList::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AId==SCT_MESSAGEWINDOWS_POI_EDIT)
        onItemClicked(currentItem(), 3);
    else if (AId==SCT_MESSAGEWINDOWS_POI_DELETE)
        onItemClicked(currentItem(), 4);
}

void MessagePoiList::setItemData(QTreeWidgetItem *AItem, const GeolocElement &APoi)
{
	QIcon icon=FPoi->getTypeIcon(APoi.type());
    if (icon.isNull())
        icon=FPoi->getIcon(MNI_POI_NONE);
    AItem->setIcon(0, icon);
	AItem->setText(0, APoi.text());
    AItem->setIcon(1, FPoi->getIcon(MNI_GEOLOC));
    AItem->setText(1, FPoi->getCoordString(APoi));
	AItem->setText(2, APoi.description());
    QVariant poi;
    poi.setValue(APoi);
    AItem->setData(1, IDR_POI, poi);
}

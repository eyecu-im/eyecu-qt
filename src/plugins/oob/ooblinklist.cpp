#include "ooblinklist.h"
#include <interfaces/imessagewidgets.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/messagechatwindowwidgets.h>
#include <definitions/messagenormalwindowwidgets.h>
#include "utils/qt4qt5compat.h"

OobLinkList::OobLinkList(IconStorage *AIconStorage, QWidget *parent) :
    QTreeWidget(parent), FIconStorage(AIconStorage)
{
    setRootIsDecorated(false);
    setColumnCount(4);
    setSortingEnabled(false);
    setHeaderHidden(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QHeaderView *headerView=header();
    headerView->setStretchLastSection(false);
	headerView->SETRESIZEMODE(QHeaderView::Fixed);
	headerView->SETRESIZEMODE(1, QHeaderView::Stretch);
    headerView->resizeSection(0, 18);
    headerView->resizeSection(2, 18);
    headerView->resizeSection(3, 18);

    IMessageChatWindow *chatWindow=qobject_cast<IMessageChatWindow *>(parent);
    if (chatWindow)   // Chat Window
        chatWindow->messageWidgetsBox()->insertWidget(MCWW_OOBLINKLISTWIDGET, this);
    else        // Message Window
    {
        IMessageNormalWindow *normalWindow=qobject_cast<IMessageNormalWindow *>(parent);
        if (normalWindow)
            normalWindow->messageWidgetsBox()->insertWidget(MNWW_OOBLINKLISTWIDGET, this);
    }

    // Initialize shortcuts
    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_OOB_DELETELINK, this);
    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_OOB_EDITLINK, this);
    connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(onItemClicked(QTreeWidgetItem*, int)));
    hide();
}

QSize OobLinkList::sizeHint() const
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

QSize OobLinkList::minimumSizeHint() const
{
    return sizeHint();
}

void OobLinkList::addLink(const QUrl &AUrl, const QString &ADescription)
{
    QTreeWidgetItem *item= new QTreeWidgetItem();
    item->setIcon(0, FIconStorage->getIcon(MNI_LINK));
    item->setSizeHint(0, QSize(18,18));

    item->setIcon(2, FIconStorage->getIcon(MNI_EDIT_EDIT));
    item->setSizeHint(2, QSize(18,18));
    item->setToolTip(2, tr("Edit"));

    item->setIcon(3, FIconStorage->getIcon(MNI_EDIT_DELETE));
    item->setSizeHint(3, QSize(18,18));
    item->setToolTip(3, tr("Delete"));

    setItemData(item, AUrl, ADescription);
    addTopLevelItem(item);

    if (isHidden())
        show();
    updateGeometry();
}

void OobLinkList::onItemClicked(QTreeWidgetItem *AItem, int AColumn)
{
    if (AItem)
        switch (AColumn)
        {
            case 2: // Edit
            {
                QUrl url=AItem->data(1, IDR_URL).toUrl();
                QString description=AItem->data(1, IDR_DESCRIPTION).toString();

                NewLink *newLink = new NewLink(tr("Edit link"),
                                               FIconStorage->getIcon(MNI_LINK),
                                               url, description, parentWidget());
                if(newLink->exec())
                {
                    setItemData(AItem, newLink->getUrl(), newLink->getDescription());
                    updateGeometry();
                }
                newLink->deleteLater();
                break;
            }

            case 3: // Delete
            {
                takeTopLevelItem(indexOfTopLevelItem(AItem));
                if (!topLevelItemCount())
                    hide();
                updateGeometry();
                break;
            }
        }
}

void OobLinkList::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AId==SCT_MESSAGEWINDOWS_OOB_EDITLINK)
        onItemClicked(currentItem(), 2);
    else if (AId==SCT_MESSAGEWINDOWS_OOB_DELETELINK)
        onItemClicked(currentItem(), 3);
}

void OobLinkList::setItemData(QTreeWidgetItem *AItem, const QUrl &AUrl, const QString &ADescription)
{
    QString text(ADescription.isEmpty()?"%1":ADescription+" (%1)");
    AItem->setText(1, text.arg(AUrl.toString()));
    AItem->setData(1, IDR_URL, AUrl);
    AItem->setData(1, IDR_DESCRIPTION, ADescription);
}

#include <definitions/shortcuts.h>
#include "poilist.h"
#include "utils/qt4qt5compat.h"

PoiList::PoiList(IPoi *APoi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PoiList),
    FPoi(APoi)
{
    ui->setupUi(this);

    ui->poiList->setColumnWidth(0, 28);
	ui->poiList->header()->SETRESIZEMODE(0, QHeaderView::Fixed);
	ui->poiList->header()->SETRESIZEMODE(1, QHeaderView::ResizeToContents);
    ui->poiList->sortItems(0, Qt::AscendingOrder);
    ui->poiList->setAlternatingRowColors(true);

    FPoi->setTreeWidgetShortcuts(ui->poiList);

    connect(FPoi->instance(), SIGNAL(poiModified(QString,int)), SLOT(onPoiModified(QString,int)));
    connect(FPoi->instance(), SIGNAL(poisLoaded(QString,PoiHash)), SLOT(onPoisLoaded(QString,PoiHash)));
    connect(FPoi->instance(), SIGNAL(poisRemoved(QString)), SLOT(onPoisRemoved(QString)));
}

PoiList::~PoiList()
{
    delete ui;
}

void PoiList::onItemActivated(QTreeWidgetItem *selectPitem, int index)
{
    Q_UNUSED(selectPitem);
    Q_UNUSED(index);
    accept();
}

void PoiList::onCustomContextMenuRequested(const QPoint &APos)
{
    QTreeWidgetItem *item=ui->poiList->itemAt(APos);
    if (item)
    {
        Menu *menu=new Menu(ui->poiList);
        FPoi->contextMenu(item->data(0, IPoi::PDR_ID).toString(), menu);
        menu->exec(ui->poiList->viewport()->mapToGlobal(APos));
        menu->deleteLater();
    }
}

void PoiList::onPoiModified(const QString &AId, int AType)
{
    if (AId.contains(':'))
    {
        QStringList splitted=AId.split(':');
        if (FBareStreamJid.isEmpty() || FBareStreamJid==splitted[0])
            switch (AType)
            {
                case IPoi::PMT_MODIFIED:
                {
                    int count=ui->poiList->topLevelItemCount();
                    for (int i=0; i<count; i++)
                    {
                        QTreeWidgetItem *item=ui->poiList->topLevelItem(i);
                        if (item->data(0, IPoi::PDR_ID).toString()==AId)
                        {
                            GeolocElement poi=FPoi->getPoi(AId);
							QIcon icon=FPoi->getTypeIcon(poi.type());
                            if (icon.isNull())
                                icon=FPoi->getIcon(MNI_POI_NONE);
                            item->setIcon(0, icon);
							item->setText(0, poi.type());
							item->setText(1, poi.text());
							item->setText(2, poi.description());
                            break;
                        }
                    }
                    break;
                }

                case IPoi::PMT_ADDED:
                {
                    GeolocElement poi=FPoi->getPoi(AId);
                    QTreeWidgetItem *item=new QTreeWidgetItem(ui->poiList);
					QIcon icon=FPoi->getIcon(poi.type());
                    if (icon.isNull())
                        icon=FPoi->getIcon(MNI_POI_NONE);
                    item->setIcon(0, icon);
					item->setText(1, poi.text());
					item->setText(2, poi.description());
                    item->setData(0, IPoi::PDR_ID, AId);
                    break;
                }

                case IPoi::PMT_REMOVED:
                {
                    int count=ui->poiList->topLevelItemCount();
                    for (int i=0; i<count; i++)
                        if (ui->poiList->topLevelItem(i)->data(0, IPoi::PDR_ID).toString()==AId)
                        {
                            delete ui->poiList->takeTopLevelItem(i);
                            break;
                        }
                    break;
                }
            }
    }
}

void PoiList::onPoisLoaded(const QString &ABareSteamJid, const PoiHash &APoiHash)
{
    if (FBareStreamJid.isEmpty() || FBareStreamJid==ABareSteamJid)
        appendStreamPois(APoiHash, ABareSteamJid);
}

void PoiList::onPoisRemoved(const QString &ABareSteamJid)
{
    int count=ui->poiList->topLevelItemCount();
    for (int i=0; i<count; )
    {
        QStringList splitted=ui->poiList->topLevelItem(i)->data(0, IPoi::PDR_ID).toString().split(':');
        if (splitted[0]==ABareSteamJid)
        {
            delete ui->poiList->takeTopLevelItem(i);
            count--;
        }
        else
            i++;
    }
}

void PoiList::appendStreamPois(const PoiHash &APoiHash, QString ABareJid)
{
    QSize size(18,18);
    ABareJid.append(":%1");    

    for(PoiHash::const_iterator it=APoiHash.constBegin(); it!=APoiHash.constEnd(); it++)
        if(!(*it).isEmpty())
        {
            QTreeWidgetItem* item= new QTreeWidgetItem(ui->poiList);
			QIcon icon=FPoi->getTypeIcon((*it).type());
            if (icon.isNull())
                icon=FPoi->getIcon(MNI_POI_NONE);            
            item->setData(0, IPoi::PDR_ID, ABareJid.arg(it.key())); // Remember item key
            item->setIcon(0, icon);
			item->setText(0, (*it).type());
            item->setSizeHint(0, size);
			item->setText(1, (*it).hasProperty(GeolocElement::Text) ? (*it).text() : "");
			item->setText(2, (*it).hasProperty(GeolocElement::Description) ? (*it).description() : "");
        }
}

void PoiList::fillTable(const PoiHash &APoiHash, QString ABareJid)
{
    FBareStreamJid=ABareJid;
    ui->poiList->clear();
    appendStreamPois(APoiHash, ABareJid);
}

void PoiList::fillTable(QHash<QString, PoiHash> &AGeolocHash)
{
    ui->poiList->clear();
    FBareStreamJid.clear();

    for (QHash<QString, PoiHash>::const_iterator it=AGeolocHash.constBegin(); it!=AGeolocHash.constEnd(); it++)
        appendStreamPois(*it, it.key());
}

QString PoiList::selectedId() const
{
    QList<QTreeWidgetItem *>selection = ui->poiList->selectedItems();
    return selection.size()==1?selection[0]->data(0, IPoi::PDR_ID).toString():QString();
}

void PoiList::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

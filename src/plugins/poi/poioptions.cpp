#include <QFontDialog>
#include <QColorDialog>
#include <QGraphicsDropShadowEffect>

#include <definitions/menuicons.h>
#include "poioptions.h"
#include "ui_poioptions.h"

PoiOptions::PoiOptions(Poi *APoi,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::poiOptions),
    FPoi(APoi)
{
    ui->setupUi(this);
    createTypeWidget();

    connect(ui->pbFont, SIGNAL(clicked()), SLOT(modifyFont()));
    connect(ui->pbColor, SIGNAL(clicked()), SLOT(modifyColor()));
    connect(ui->pbShadowColor, SIGNAL(clicked()), SLOT(modifyShadowColor()));
    connect(ui->pbTempColor, SIGNAL(clicked()), SLOT(modifyTempColor()));
    connect(ui->rbMap, SIGNAL(clicked()), SLOT(onBackgroundSelected()));
    connect(ui->rbSatellite, SIGNAL(clicked()), SLOT(onBackgroundSelected()));
    connect(ui->pbCheckAll, SIGNAL(clicked()), SLOT(onCheckAll()));
    connect(ui->pbUncheckAll, SIGNAL(clicked()), SLOT(onUncheckAll()));



    connect(ui->treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(onItemChanged(QTreeWidgetItem*,int)));

    ui->rbMap->click();
    reset();
}

PoiOptions::~PoiOptions()
{
    delete ui;
}

void PoiOptions::createTypeWidget()
{
    FSubTypes        = FPoi->getTypeMap();
    FTranslatedTypes = FPoi->getTranslatedTypes();
    FRootTypes       = FSubTypes.keys();
    FRootTypes.removeDuplicates();

    ui->treeWidget->sortItems(0,Qt::AscendingOrder);
    ui->treeWidget->setHeaderLabel(tr("POI filter"));

    // Add "None" item
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, QString("<%1>").arg(FTranslatedTypes.value("none")));//Qt::DisplayRole
    item->setData(0, Qt::UserRole, "none");
    item->setIcon(0, FPoi->getIcon(MNI_POI_NONE));

    for (QStringList::const_iterator it=FRootTypes.constBegin(); it!=FRootTypes.constEnd(); it++)
    {
        item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, FTranslatedTypes.value(*it));//Qt::DisplayRole
        item->setData(0, Qt::UserRole, *it);
        item->setIcon(0, FPoi->getTypeIcon(*it));
        item->setExpanded(false);
        QStringList subTypes=FSubTypes.values(*it);
        for (QStringList::const_iterator itp=subTypes.constBegin(); itp!=subTypes.constEnd(); itp++)
        {
            QString id=*it+':'+*itp;
            QTreeWidgetItem* subItem = new QTreeWidgetItem(item);
            subItem->setData(0, Qt::DisplayRole, FTranslatedTypes.value(id));
            subItem->setData(0, Qt::UserRole, id);
            subItem->setIcon(0, FPoi->getTypeIcon(id));
        }
    }
}

void PoiOptions::updateTypeWidget()
{
    int count=ui->treeWidget->topLevelItemCount();
    for (int i=0; i<count; i++)
    {
        QTreeWidgetItem *topLevelItem=ui->treeWidget->topLevelItem(i);
        QString id=topLevelItem->data(0, Qt::UserRole).toString();
        if (FPoiFilter.contains(id))
            topLevelItem->setCheckState(0, Qt::Checked);
        else
            topLevelItem->setCheckState(0, Qt::Unchecked);
        int childCount=topLevelItem->childCount();
        for (int i=0; i<childCount; i++)
        {            
            QTreeWidgetItem *item=topLevelItem->child(i);
            QString id=item->data(0, Qt::UserRole).toString();
            if (FPoiFilter.contains(id))
                item->setCheckState(0, Qt::Checked);
            else
                item->setCheckState(0, Qt::Unchecked);
        }
    }
}

void PoiOptions::onItemChanged(QTreeWidgetItem *item, int index)
{
    Q_UNUSED(index);    
    QString type = item->data(0, Qt::UserRole).toString();
    switch(item->checkState(0))
    {
        case Qt::Unchecked: if(FPoiFilter.contains(type)) FPoiFilter.removeAll(type); break;
        case Qt::Checked: if(!FPoiFilter.contains(type))  FPoiFilter.append(type);    break;
        case Qt::PartiallyChecked: break;
    }
    emit modified();
}

void PoiOptions::groupItemChanged(QTreeWidgetItem *item)
{
    QString newPar = item->data(0, Qt::UserRole).toString();
}

void PoiOptions::onCheckAll()
{
    QList<QTreeWidgetItem *> selected=ui->treeWidget->selectedItems();
    QString type="none";
    if (!selected.isEmpty())
    {
        type=selected.first()->data(0, Qt::UserRole).toString();
        if(type.contains(':'))
            type=type.split(':').first();
    }

    if (type=="none")
        FPoiFilter=FPoi->getAllTypes();
    else
    {
        if (!FPoiFilter.contains(type))
            FPoiFilter.append(type);
        QMap<QString, QString> types=FPoi->getTypeMap();
        QStringList subTypes=types.values(type);
        for (QStringList::const_iterator it=subTypes.constBegin(); it!=subTypes.constEnd(); it++)
        {
            QString id=type+':'+*it;
            if (!FPoiFilter.contains(id))
                FPoiFilter.append(id);
        }
    }

    updateTypeWidget();
    emit modified();
}

void PoiOptions::onUncheckAll()
{
    QList<QTreeWidgetItem *> selected=ui->treeWidget->selectedItems();
    QString type="none";
    if (!selected.isEmpty())
    {
        type=selected.first()->data(0, Qt::UserRole).toString();
        if(type.contains(':'))
            type=type.split(':').first();
    }

    if (type=="none")
        FPoiFilter.clear();
    else
    {
        if (FPoiFilter.contains(type))
            FPoiFilter.removeAll(type);
        QMap<QString, QString> types=FPoi->getTypeMap();
        QStringList subTypes=types.values(type);
        for (QStringList::const_iterator it=subTypes.constBegin(); it!=subTypes.constEnd(); it++)
        {
            QString id=type+':'+*it;
            if (FPoiFilter.contains(id))
                FPoiFilter.removeAll(id);
        }
    }

    updateTypeWidget();
    emit modified();
}


QStringList PoiOptions::getFilter(){return FPoiFilter;}

void PoiOptions::modifyFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, FCurrentFont, this);
    if(ok)
    {
        ui->txtLabel->setFont(font);
        ui->txtTemporary->setFont(font);
//        ui->viewShad->setFont(font);
        FCurrentFont = font;
        emit modified();
    }
}

void PoiOptions::modifyColor()
{
    QColor color = QColorDialog::getColor(FCurrentTextColor,this,tr("Select POI label color")); //"color:#0066ff;"
    if(color.isValid())
    {
        ui->txtLabel->setPalette(QColor(color));
        FCurrentTextColor = color;
        emit modified();
    }
}

void PoiOptions::modifyTempColor()
{
    QColor color = QColorDialog::getColor(FCurrentTempTextColor, 0, tr("Select temporary POI label color"));
    if(color.isValid())
    {
        ui->txtTemporary->setPalette(color);
        FCurrentTempTextColor = color;
        emit modified();
    }
}

void PoiOptions::modifyShadowColor()
{
    QColor color = QColorDialog::getColor(FCurrentShadowColor, 0, tr("Set shadow color"));
    if(color.isValid())
    {
        ui->txtLabelShadow->setPalette(QColor(color.name()));
        ui->txtTemporaryShadow->setPalette(QColor(color.name()));
        FCurrentShadowColor = color;
        emit modified();
    }
}

void PoiOptions::onBackgroundSelected()
{
    ui->frame->setStyleSheet(
                QString("QFrame { background-image: url(%1) }; QLabel { background-image: none }")
                .arg(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)
                     ->fileFullName(sender()==ui->rbMap?"bgrmap":"bgrsat")));
}

void PoiOptions::apply()
{
    Options::node(OPV_POI_PNT_TEXTCOLOR).setValue(FCurrentTextColor);
    Options::node(OPV_POI_PNT_TEMPTEXTCOLOR).setValue(FCurrentTempTextColor);
    Options::node(OPV_POI_PNT_SHADOWCOLOR).setValue(FCurrentShadowColor);
    Options::node(OPV_POI_PNT_FONT).setValue(FCurrentFont);
    Options::node(OPV_POI_FILTER).setValue(FPoiFilter);
    emit childApply();
}

void PoiOptions::reset()
{
    FCurrentTextColor   = Options::node(OPV_POI_PNT_TEXTCOLOR).value().value<QColor>();
    FCurrentShadowColor = Options::node(OPV_POI_PNT_SHADOWCOLOR).value().value<QColor>();
    FCurrentTempTextColor = Options::node(OPV_POI_PNT_TEMPTEXTCOLOR).value().value<QColor>();
    FCurrentFont        = Options::node(OPV_POI_PNT_FONT).value().value<QFont>();
    FPoiFilter          = Options::node(OPV_POI_FILTER).value().toStringList();

    updateTypeWidget();

    ui->txtLabel->setFont(FCurrentFont);
    ui->txtTemporary->setFont(FCurrentFont);
    ui->txtLabelShadow->setFont(FCurrentFont);
    ui->txtTemporaryShadow->setFont(FCurrentFont);

    QPalette palette;
    palette.setColor(QPalette::WindowText, FCurrentTextColor);
    ui->txtLabel->setPalette(palette); //"color:cyan;"
    palette.setColor(QPalette::WindowText, FCurrentTempTextColor);
    ui->txtTemporary->setPalette(palette); //"color:cyan;"
    palette.setColor(QPalette::WindowText, FCurrentShadowColor);
    ui->txtLabelShadow->setPalette(palette); //"color:red;"
    ui->txtTemporaryShadow->setPalette(palette); //"color:red;"

    emit childReset();
}


void PoiOptions::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

#include <definitions/menuicons.h>
#include "activityselect.h"

ActivitySelect::ActivitySelect(Activity *AActivity, const QHash<QString, QStringList> &AActivityList, const QHash<QString, QStringList> &AActivityTexts, const ActivityData &AActivityData, QWidget *parent) :
    QDialog(parent),
    FActivity(AActivity),
    FActivityData(AActivityData),
    FActivityNames(AActivityList),
    FActivityTexts(AActivityTexts),
	ui(new Ui::ActivitySelect)
{
    ui->setupUi(this);
    ui->lstActivity->sortItems(0,Qt::AscendingOrder);

	setWindowIcon(FActivity->getIcon(MNI_ACTIVITY));
    fillActivityTree();    
    fillTextList(AActivityData.iconFileName());

    connect(ui->lstActivity,SIGNAL(itemActivated(QTreeWidgetItem*,int)),SLOT(accept()));
    connect(ui->lstActivity,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),SLOT(onCurItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->cmbText,SIGNAL(editTextChanged(QString)),SLOT(onEditTextChanged(QString)));

	for (QTreeWidgetItemIterator it(ui->lstActivity, QTreeWidgetItemIterator::All) ; *it; ++it)
		if ((*it)->data(0,Qt::UserRole).toString()==AActivityData.nameDetailed && (*it)->text(1)==AActivityData.nameBasic)
		{
			ui->lstActivity->setCurrentItem(*it);
			(*it)->setSelected(true);
			break;
		}

	ui->cmbText->setEditText(AActivityData.text);
}

ActivitySelect::~ActivitySelect()
{
    delete ui;
}

void ActivitySelect::changeEvent(QEvent *e)
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

void ActivitySelect::onCurItemChanged(QTreeWidgetItem *AItemNew, QTreeWidgetItem *AItemOld)
{
    Q_UNUSED(AItemOld);    
    ui->lblIcon->setPixmap(AItemNew->icon(0).pixmap(16));    
    FActivityData.nameBasic = AItemNew->text(1);
    FActivityData.nameDetailed = AItemNew->data(0, Qt::UserRole).toString();
    fillTextList(FActivityData.iconFileName());
}

void ActivitySelect::onEditTextChanged(const QString &ANewText)
{
    if (!FActivityData.isEmpty())
        FActivityData.text = ANewText;
}

void ActivitySelect::fillTextList(const QString &AActivity)
{
    ui->cmbText->clear();
    ui->cmbText->addItems(FActivityTexts.value(AActivity));
    ui->cmbText->setEditable(true);
}

void ActivitySelect::fillActivityTree()
{
    QTreeWidgetItem* pItem;
    QTreeWidgetItem* pItemDir;
    ui->lstActivity->sortItems(0, Qt::AscendingOrder);
    for (QHash<QString, QStringList>::const_iterator it=FActivityNames.constBegin(); it!=FActivityNames.constEnd(); it++)
    {
        ActivityData data(it.key());
        pItem = new QTreeWidgetItem(ui->lstActivity);
		pItem->setText(1, data.nameBasic);
		pItem->setText(0, (data.isEmpty()?QString("<%1>").arg(FActivity->getActivityText("no_activity")):FActivity->getActivityText(data.nameBasic)));
        pItem->setIcon(0, FActivity->getIcon(data));
        for (QStringList::const_iterator itn=(*it).constBegin(); itn!=(*it).constEnd(); itn++)
        {
            data.nameDetailed = *itn;
            pItemDir = new QTreeWidgetItem(pItem);
            pItemDir->setText(1, data.nameBasic);                       // dir-name
            pItemDir->setText(0, FActivity->getActivityText(data.nameDetailed)); // transl-name
            pItemDir->setData(0, Qt::UserRole, data.nameDetailed);            // engl-name
            pItemDir->setIcon(0, FActivity->getIcon(data));
        }
        pItem->setExpanded(false);
    }
}



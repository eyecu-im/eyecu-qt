#include <definitions/menuicons.h>
#include "moodselect.h"

MoodSelect::MoodSelect(Mood *AMood, const QStringList &AMoodList, const QHash<QString, QStringList> &ATextList, const QHash<QString, QString> &AMoodKeys, const MoodData &AMoodData, QWidget *parent):
    QDialog(parent),
    ui(new Ui::MoodSelect),
    FMood(AMood),
    FMoodTree(NULL),
    FMoodList(AMoodList),    
    FTextList(ATextList),
    FMoodKeys(AMoodKeys)
{
    ui->setupUi(this);	
    ui->listActiv->sortItems(0,Qt::AscendingOrder);

    connect(ui->listActiv,SIGNAL(itemActivated(QTreeWidgetItem*,int)),SLOT(accept()));
    connect(ui->listActiv,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),SLOT(onCurItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(ui->comboBox,SIGNAL(editTextChanged(QString)),SLOT(onEditTextChanged(QString)));

	setWindowIcon(AMood->getIcon(QString(MNI_MOOD)));
    fillMoodList();
    setCurrentItem(AMoodData);
}

MoodSelect::~MoodSelect()
{
    delete ui;
}

MoodData MoodSelect::moodData() const
{
    return FMoodData;
}

void MoodSelect::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//-------------------
void MoodSelect::setCurrentItem(const MoodData &AMoodData)
{
    for(QTreeWidgetItemIterator it(ui->listActiv, QTreeWidgetItemIterator::All); *it; it++)
        if((*it)->text(1) == AMoodData.name)
        {
            ui->listActiv->setCurrentItem(*it);
            (*it)->setSelected(true);
            ui->comboBox->setEditText(AMoodData.text);
            break;
        }    
}

void MoodSelect::fillTextList(const QString &AMoodName)
{
    ui->comboBox->clear();
    ui->comboBox->addItems(FTextList.value(AMoodName));
    ui->comboBox->setEditable(true);
}

void MoodSelect::onEditTextChanged(const QString &ANewText)
{
    if (!FMoodData.isEmpty())
        FMoodData.text = ANewText;
}

void MoodSelect::onCurItemChanged(QTreeWidgetItem *ANewItem, QTreeWidgetItem *AOldItem)
{
    Q_UNUSED(AOldItem);
    ui->lblIcon->setPixmap(ANewItem->icon(0).pixmap(16));
    if (ANewItem->text(1)=="no_mood")
        FMoodData.clear();
    else
    {
        FMoodData.name = ANewItem->text(1);
		fillTextList(FMoodData.name);
        FMoodData.text = ui->comboBox->currentText();
    }
}

void MoodSelect::fillMoodList()
{
    ui->listActiv->setSortingEnabled(true);
    ui->listActiv->setIconSize(QSize(16,16));
    ui->listActiv->setAlternatingRowColors(true);

    for (QHash<QString, QString>::const_iterator it = FMoodKeys.constBegin(); it!=FMoodKeys.constEnd(); it++)
    {
        FMoodTree = new QTreeWidgetItem(ui->listActiv);
        FMoodTree->setText(0, (it.key()=="no_mood"?QString("<%1>"):QString("%1")).arg(*it));
        FMoodTree->setText(1, it.key()=="no_mood"?QString():it.key());
        FMoodTree->setIcon(0, FMood->getIcon(it.key()));
    }    
}


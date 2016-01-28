#include "emojioptions.h"
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <utils/iconsetdelegate.h>
#include <utils/options.h>

EmojiOptions::EmojiOptions(IEmoji *AEmoji, QWidget *AParent):
	QWidget(AParent),
	FEmoji(AEmoji)
{
	ui.setupUi(this);
	QStyle *style = QApplication::style();
	ui.tbtUp->setIcon(style->standardIcon(QStyle::SP_ArrowUp));
	ui.tbtDown->setIcon(style->standardIcon(QStyle::SP_ArrowDown));
	ui.tbtSelectable->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_PEPMANAGER));

	QStringList filter(AEmoji->colorSuffixes());
	filter.append("default");
	ui.lwtEmoticons->setItemDelegate(new IconsetDelegate(filter, ui.lwtEmoticons));
	connect(ui.lwtEmoticons,SIGNAL(itemChanged(QListWidgetItem *)),SIGNAL(modified()));
	connect(ui.tbtUp,SIGNAL(clicked()),SLOT(onUpButtonClicked()));
	connect(ui.tbtDown,SIGNAL(clicked()),SLOT(onDownButtonClicked()));
	connect(ui.tbtSelectable,SIGNAL(toggled(bool)),SLOT(onMakeSelectableButtonToggled(bool)));
	connect(ui.lwtEmoticons,SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(ui.lwtEmoticons,SIGNAL(itemChanged(QListWidgetItem*)),SLOT(onItemChanged(QListWidgetItem*)));
	reset();
	onCurrentItemChanged(NULL, NULL);
}

EmojiOptions::~EmojiOptions()
{
}

void EmojiOptions::apply()
{
	QString iconset="Emoji One";
//	for (int i = 0; i<ui.lwtEmoticons->count(); i++)
//		if (ui.lwtEmoticons->item(i)->checkState() == Qt::Checked || ui.lwtEmoticons->item(i)->checkState() == Qt::PartiallyChecked)
//		{
//			QString iconset = ui.lwtEmoticons->item(i)->data(IconsetDelegate::IDR_SUBSTORAGE).toString();
//			if (ui.lwtEmoticons->item(i)->checkState() == Qt::PartiallyChecked)
//				iconset.prepend('@');
//			iconsets.append(iconset);
//		}

	Options::node(OPV_MESSAGES_EMOJI_ICONSET).setValue(iconset);

	emit childApply();
}

void EmojiOptions::reset()
{
	ui.lwtEmoticons->clear();
	QString iconset = Options::node(OPV_MESSAGES_EMOJI_ICONSET).value().toString();
//	for (int i = 0; i < storages.count(); i++)
//	{
//		bool	active;
//		if (storages[i].at(0)=='@')
//		{
//			storages[i].remove(0, 1);
//			active = false;
//		}
//		else
//			active = true;
//		QString storage = storages.at(i);
//		QListWidgetItem *item = new QListWidgetItem(RSR_STORAGE_EMOJI"/"+storage,ui.lwtEmoticons);
//		item->setData(IconsetDelegate::IDR_STORAGE,RSR_STORAGE_EMOJI);
//		item->setData(IconsetDelegate::IDR_SUBSTORAGE, storage);
//		item->setData(IconsetDelegate::IDR_ICON_ROW_COUNT,2);
//		item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsTristate|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
//		item->setCheckState(active?Qt::Checked:Qt::PartiallyChecked);
//	}

//	QList<QString> allstorages = FileStorage::availSubStorages(RSR_STORAGE_EMOJI);
//	for (int i = 0; i < allstorages.count(); i++)
//	{
//		if (!storages.contains(allstorages.at(i)))
//		{
//			QListWidgetItem *item = new QListWidgetItem(allstorages.at(i),ui.lwtEmoticons);
//			item->setData(IconsetDelegate::IDR_STORAGE,RSR_STORAGE_EMOJI);
//			item->setData(IconsetDelegate::IDR_SUBSTORAGE,allstorages.at(i));
//			item->setData(IconsetDelegate::IDR_ICON_ROW_COUNT,2);
//			item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
//			item->setCheckState(Qt::Unchecked);
//		}
//	}
}

void EmojiOptions::onUpButtonClicked()
{
	if (ui.lwtEmoticons->currentRow() > 0)
	{
		int row = ui.lwtEmoticons->currentRow();
		ui.lwtEmoticons->insertItem(row-1, ui.lwtEmoticons->takeItem(ui.lwtEmoticons->currentRow()));
		ui.lwtEmoticons->setCurrentRow(row-1);
		emit modified();
	}
}

void EmojiOptions::onDownButtonClicked()
{
	if (ui.lwtEmoticons->currentRow() < ui.lwtEmoticons->count()-1)
	{
		int row = ui.lwtEmoticons->currentRow();
		ui.lwtEmoticons->insertItem(row+1, ui.lwtEmoticons->takeItem(ui.lwtEmoticons->currentRow()));
		ui.lwtEmoticons->setCurrentRow(row+1);
		emit modified();
	}
}

void EmojiOptions::onMakeSelectableButtonToggled(bool ASelectable)
{
	ui.lwtEmoticons->currentItem()->setCheckState(ASelectable?Qt::Checked:Qt::PartiallyChecked);
}

void EmojiOptions::onCurrentItemChanged(QListWidgetItem *ACurrent, QListWidgetItem *APrevious)
{
	Q_UNUSED(APrevious)
	ui.tbtSelectable->blockSignals(true);
	ui.tbtSelectable->setDisabled(!ACurrent || ACurrent->checkState()==Qt::Unchecked);
	ui.tbtSelectable->setChecked(ACurrent && ACurrent->checkState()==Qt::Checked);
	ui.tbtSelectable->blockSignals(false);
}

void EmojiOptions::onItemChanged(QListWidgetItem *AItem)
{
	if (AItem == ui.lwtEmoticons->currentItem())
		onCurrentItemChanged(AItem, NULL);
}

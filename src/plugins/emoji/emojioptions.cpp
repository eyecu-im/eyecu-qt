#include "emojioptions.h"
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <utils/iconsetdelegate.h>
#include <utils/options.h>

EmojiOptions::EmojiOptions(IEmoticons *AEmoticons, QWidget *AParent) : QWidget(AParent)
{
	ui.setupUi(this);
//	IconStorage *storage = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	QStyle *style = QApplication::style();
	ui.tbtUp->setIcon(style->standardIcon(QStyle::SP_ArrowUp));
	ui.tbtDown->setIcon(style->standardIcon(QStyle::SP_ArrowDown));

	FEmoticons = AEmoticons;
	ui.lwtEmoticons->setItemDelegate(new IconsetDelegate(ui.lwtEmoticons));
	connect(ui.lwtEmoticons,SIGNAL(itemChanged(QListWidgetItem *)),SIGNAL(modified()));
	connect(ui.tbtUp,SIGNAL(clicked()),SLOT(onUpButtonClicked()));
	connect(ui.tbtDown,SIGNAL(clicked()),SLOT(onDownButtonClicked()));

	reset();
}

EmojiOptions::~EmojiOptions()
{
}

void EmojiOptions::apply()
{
	QStringList iconsets;
	for (int i = 0; i<ui.lwtEmoticons->count(); i++)
		if (ui.lwtEmoticons->item(i)->checkState() == Qt::Checked)
			iconsets.append(ui.lwtEmoticons->item(i)->data(IconsetDelegate::IDR_SUBSTORAGE).toString());

	Options::node(OPV_MESSAGES_EMOJI_ICONSETS).setValue(iconsets);

	emit childApply();
}

void EmojiOptions::reset()
{
	ui.lwtEmoticons->clear();
	QStringList storages = Options::node(OPV_MESSAGES_EMOJI_ICONSETS).value().toStringList();
	for (int i = 0; i < storages.count(); i++)
	{
		QString storage = storages.at(i);
		bool	active;
		if (storage.at(0)=='@')
		{
			storage.remove(0, 1);
			active = false;
		}
		else
			active = true;
		QListWidgetItem *item = new QListWidgetItem(RSR_STORAGE_EMOJI"/"+storage,ui.lwtEmoticons);
		item->setData(IconsetDelegate::IDR_STORAGE,RSR_STORAGE_EMOJI);
		item->setData(IconsetDelegate::IDR_SUBSTORAGE, storage);
		item->setData(IconsetDelegate::IDR_ICON_ROW_COUNT,2);
		item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsTristate|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		item->setCheckState(active?Qt::Checked:Qt::PartiallyChecked);
	}

	QList<QString> allstorages = FileStorage::availSubStorages(RSR_STORAGE_EMOJI);
	for (int i = 0; i < allstorages.count(); i++)
	{
		if (!storages.contains(allstorages.at(i)))
		{
			QListWidgetItem *item = new QListWidgetItem(allstorages.at(i),ui.lwtEmoticons);
			item->setData(IconsetDelegate::IDR_STORAGE,RSR_STORAGE_EMOJI);
			item->setData(IconsetDelegate::IDR_SUBSTORAGE,allstorages.at(i));
			item->setData(IconsetDelegate::IDR_ICON_ROW_COUNT,2);
			item->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
			item->setCheckState(Qt::Unchecked);
		}
	}
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

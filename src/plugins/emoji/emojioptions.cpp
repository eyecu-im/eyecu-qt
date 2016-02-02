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
	ui.cmbEmojiSet->addItem(tr("Do not use emoji"), QString(""));
	QStringList emojiSets = FEmoji->emojiSets();
	for (QStringList::ConstIterator it=emojiSets.constBegin(); it!=emojiSets.constEnd(); ++it)
		ui.cmbEmojiSet->addItem(*it, *it);
	connect(ui.cmbEmojiSet, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui.spbChatIconSize, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	connect(ui.spbMenuIconSize, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	reset();
}

EmojiOptions::~EmojiOptions()
{
}

void EmojiOptions::apply()
{
	Options::node(OPV_MESSAGES_EMOJI_ICONSET).setValue(ui.cmbEmojiSet->itemData(ui.cmbEmojiSet->currentIndex()).toString());
	Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).setValue(ui.spbChatIconSize->currentSize());
	Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).setValue(ui.spbMenuIconSize->currentSize());

	emit childApply();
}

void EmojiOptions::reset()
{
	ui.cmbEmojiSet->setCurrentIndex(ui.cmbEmojiSet->findData(Options::node(OPV_MESSAGES_EMOJI_ICONSET).value().toString()));
	ui.spbChatIconSize->setCurrentSize(Options::node(OPV_MESSAGES_EMOJI_SIZE_CHAT).value().toInt());
	ui.spbMenuIconSize->setCurrentSize(Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt());
}

void EmojiOptions::onListBoxCurrentIndexChanged(int AIndex)
{
	if (AIndex)
	{
		QList<int> avaliableSizes = FEmoji->availableSizes(ui.cmbEmojiSet->itemData(AIndex).toString());
		ui.spbChatIconSize->setSizes(avaliableSizes);
		ui.spbMenuIconSize->setSizes(avaliableSizes);
	}
	else
	{
		ui.spbChatIconSize->setSizes(QList<int>());
		ui.spbMenuIconSize->setSizes(QList<int>());
	}
}

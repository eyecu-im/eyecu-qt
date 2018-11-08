#include "selectsubiconwidget.h"

SelectSubIconWidget::SelectSubIconWidget(const QStringList &AEmojiIds, IEmoji *AEmoji, QWidget *AParent):
	QWidget(AParent),
	FEmoji(AEmoji),
	FEmojiIds(AEmojiIds)
{

}

SelectSubIconWidget::~SelectSubIconWidget()
{}

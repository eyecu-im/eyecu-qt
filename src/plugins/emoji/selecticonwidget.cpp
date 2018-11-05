#include "selecticonwidget.h"
#include "emoji.h"
#include <QCursor>
#include <QToolTip>
#include <QTextDocument>
#include <definitions/optionvalues.h>
#include <utils/qt4qt5compat.h>

SelectIconWidget::SelectIconWidget(IEmoji::Category ACategory, uint AColumns, uint ARows, IEmoji *AEmoji, QWidget *AParent):
	QWidget(AParent),
	FEmoji(AEmoji),
	FPressed(nullptr),
	FEmojiMap((AEmoji->emojiData(ACategory))),
	FColor(IEmoji::SkinDefault),
	FHasColored(false),
	FGender(IEmoji::GenderDefault),
	FHasGendered(false),
	FNotReady(true),
	FColumns(AColumns),
	FRows(ARows)
{
	FLayout = new QGridLayout(this);
	FLayout->setMargin(2);
	FLayout->setHorizontalSpacing(3);
	FLayout->setVerticalSpacing(3);
	createLabels();
}

SelectIconWidget::~SelectIconWidget()
{}

void SelectIconWidget::updateLabels()
{
	int color = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	int gender = Options::node(OPV_MESSAGES_EMOJI_GENDER).value().toInt();
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize size(extent, extent);
	for (QMap<QLabel*, QString>::Iterator it=FKeyByLabel.begin(); it!=FKeyByLabel.end(); ++it)
	{
		QString key(*it);
		EmojiData data = FEmoji->findData(key);
		bool changed(false);

		if (!data.diversities.isEmpty())
		{
			if (color != FColor)
				changed = true;
			if (color && data.diversities.size() >= color)
				key = data.diversities[color-1];
		}

		data = FEmoji->findData(key);
		if (data.genders.size() == 2)
		{
			if (FGender != gender)
				changed = true;
			if (gender)
				key = data.genders[gender-1];
		}

		if (FNotReady || changed)
		{
			QIcon icon = FEmoji->getIcon(key, size);
			it.key()->setPixmap(icon.pixmap(size));
		}
	}

	if (FNotReady)
		FNotReady = false;

	if (FColor != color)
		FColor = color;

	if (FGender != gender)
		FGender = gender;
}

void SelectIconWidget::createLabels()
{
	uint row =0;
	uint column = 0;
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize iconSize(extent, extent);
	for (QMap<uint, EmojiData>::ConstIterator it=FEmojiMap.constBegin(); it!=FEmojiMap.constEnd(); ++it)
		if ((*it).present)
		{
			QLabel *label; //(NULL);
			label = new QLabel(this);
			label->setMargin(2);
			label->setAlignment(Qt::AlignCenter);
			label->setFrameShape(QFrame::Box);
			label->setFrameShadow(QFrame::Sunken);
			label->installEventFilter(this);
			label->setPixmap(QPixmap(iconSize));
			label->setToolTip((*it).name);
			FKeyByLabel.insert(label, (*it).id);
			FLayout->addWidget(label, int(row), int(column));
			if (!it->diversities.isEmpty())
				FHasColored=true;
			if (it->genders.size() == 2)
				FHasGendered=true;
			column = (column+1) % FColumns;
			row += column==0 ? 1 : 0;
			FLayout->setRowStretch(int(row), 0);
		}
	if (row < FRows)
		FLayout->setRowStretch(int(row+1), 1);
}

bool SelectIconWidget::eventFilter(QObject *AWatched, QEvent *AEvent)
{
	QLabel *label = qobject_cast<QLabel *>(AWatched);
	if (AEvent->type() == QEvent::Enter)
	{
		label->setFrameShadow(QFrame::Plain);
		QToolTip::showText(QCursor::pos(),label->toolTip());
	}
	else if (AEvent->type() == QEvent::Leave)
	{
		label->setFrameShadow(QFrame::Sunken);
	}
	else if (AEvent->type() == QEvent::MouseButtonPress)
	{
		FPressed = label;
	}
	else if (AEvent->type() == QEvent::MouseButtonRelease)
	{
		if (FPressed == label)
			emit iconSelected(FKeyByLabel.value(label));
		FPressed = nullptr;
	}
	return QWidget::eventFilter(AWatched,AEvent);
}

void SelectIconWidget::showEvent(QShowEvent *AShowEvent)
{
	Q_UNUSED(AShowEvent)
	int color = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	int gender = Options::node(OPV_MESSAGES_EMOJI_GENDER).value().toInt();
	if ((FHasColored && FColor!=color) ||
		(FHasGendered && FGender!=gender) ||
		FNotReady)
		updateLabels();
	emit hasColoredChanged(FHasColored);
	emit hasGenderedChanged(FHasGendered);
}

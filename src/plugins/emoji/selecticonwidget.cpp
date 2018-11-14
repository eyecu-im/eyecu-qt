#include "selecticonwidget.h"
#include "emoji.h"
#include <QCursor>
#include <QToolTip>
#include <QTextDocument>
#include <definitions/optionvalues.h>
#include <utils/qt4qt5compat.h>

#define ADR_EMOJI_ID Action::DR_Parametr1
#define ADR_EMOJI_ID_CURRENT Action::DR_Parametr2

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

	QLayout *l = layout();
	int count = l->count();
	for (int i=0; i<count; ++i)
	{
		QToolButton *button = qobject_cast<QToolButton *>(l->itemAt(i)->widget());
		if (button)
		{
			Action *action = qobject_cast<Action*>(button->defaultAction());
			if (action)
			{
				QString key = FEmoji->findData(action->data(ADR_EMOJI_ID).toString(),
															IEmoji::SkinColor(color),
															IEmoji::Gender(gender))->id();

				if (action->data(ADR_EMOJI_ID_CURRENT).toString() != key)
				{
					action->setData(ADR_EMOJI_ID_CURRENT, key);
					action->setIcon(FEmoji->getIcon(key, size));
				}

			}
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

	for (QMap<uint, IEmojiData*>::ConstIterator it=FEmojiMap.constBegin();
		 it!=FEmojiMap.constEnd(); ++it)
	{
		if ((*it)->present())
		{
			QToolButton *button; //(NULL);
			button = new QToolButton(this);
			button->setAutoRaise(true);
			Action *action = new Action(button);
			action->setIcon(QPixmap(iconSize));
			action->setIconText((*it)->name());
			action->setData(ADR_EMOJI_ID, (*it)->id());
			button->setDefaultAction(action);
			button->setPopupMode(QToolButton::DelayedPopup);
			connect(button, SIGNAL(triggered(QAction*)), SLOT(onActionTriggered(QAction*)));
			FLayout->addWidget(button, int(row), int(column));

			if (!(*it)->diversities().isEmpty())
				FHasColored=true;
			if ((*it)->genders().size() == 2)
				FHasGendered=true;
			column = (column+1) % FColumns;
			row += column==0 ? 1 : 0;
			FLayout->setRowStretch(int(row), 0);
		}
	}
	if (row < FRows)
		FLayout->setRowStretch(int(row+1), 1);
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

void SelectIconWidget::onActionTriggered(QAction *AAction)
{
	Action *action = qobject_cast<Action*>(AAction);
	if (action)
		emit iconSelected(action->data(ADR_EMOJI_ID).toString());
}

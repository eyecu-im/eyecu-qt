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
	FPressed(NULL),
	FEmojiMap((AEmoji->emojiData(ACategory))),
	FHasColored(false),
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

void SelectIconWidget::updateLabels(const QString &AColor, bool AForce)
{
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize size(extent, extent);
	for (QMap<QLabel*, QString>::Iterator it=FKeyByLabel.begin(); it!=FKeyByLabel.end(); ++it)
	{
		QString key(*it);
		if (FEmoji->isColored(key))
			key.chop(2);
		QIcon icon = FEmoji->getIcon(key+AColor, size);
		if (icon.isNull() && !AColor.isEmpty())
			icon = FEmoji->getIcon(key, size);
		else
			key.append(AColor);

		if (*it!=key || AForce)
		{
			(*it)=key;
			it.key()->setPixmap(icon.pixmap(size));
		}
	}
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
			FKeyByLabel.insert(label, (*it).unicode);
			FLayout->addWidget(label, row, column);
			if ((*it).colored)
				FHasColored=true;
			column = (column+1) % FColumns;
			row += column==0 ? 1 : 0;
			FLayout->setRowStretch(row, 0);
		}
	if (row<FRows)
		FLayout->setRowStretch(row+1, 1);
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
			emit iconSelected(FKeyByLabel.value(label), label->toolTip());
		FPressed = NULL;
	}
	return QWidget::eventFilter(AWatched,AEvent);
}

void SelectIconWidget::showEvent(QShowEvent *AShowEvent)
{
	Q_UNUSED(AShowEvent)
	int index = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	QString color = index?FEmoji->colorSuffixes()[index-1]:QString();
	if ((FHasColored && FColor!=color) || FNotReady)
	{
		updateLabels(FColor=color, FNotReady);
		FNotReady = false;
	}
	emit hasColoredChanged(FHasColored);
}

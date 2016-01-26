#include "selecticonwidget.h"
#include "emoji.h"
#include <QCursor>
#include <QToolTip>
#include <QTextDocument>
#include <QDebug>
#include <definitions/optionvalues.h>
#include <utils/qt4qt5compat.h>

SelectIconWidget::SelectIconWidget(IEmoji::Category ACategory, IEmoji *AEmoji, QWidget *AParent):
	QWidget(AParent),
	FEmoji(AEmoji),
	FPressed(NULL),
	FEmojiMap((AEmoji->emojiData(ACategory))),
	FHasColored(false),
	FNotReady(true)
{
	FLayout = new QGridLayout(this);
	FLayout->setMargin(2);
	FLayout->setHorizontalSpacing(3);
	FLayout->setVerticalSpacing(3);
	createLabels(QString());
}

SelectIconWidget::~SelectIconWidget()
{}

void SelectIconWidget::updateLabels(const QString &AColor, bool AForce)
{
	for (QMap<QLabel*, QString>::Iterator it=FKeyByLabel.begin(); it!=FKeyByLabel.end(); ++it)
	{
		QString key(*it);
		if (FEmoji->isColored(key))
			key.chop(2);
		QIcon icon = FEmoji->getIcon(key+AColor, QSize(16, 16));
		if (icon.isNull() && !AColor.isEmpty())
			icon = FEmoji->getIcon(key, QSize(16, 16));
		else
			key.append(AColor);

		if (*it!=key || AForce)
		{
			(*it)=key;
			it.key()->setPixmap(icon.pixmap(16, 16));
		}
	}
}

QLabel *SelectIconWidget::getIconLabel(const QString &AKey, const QString &AToolTip)
{
	QLabel *label; //(NULL);
	QString key(AKey);
	label = new QLabel(this);
	label->setMargin(2);
	label->setAlignment(Qt::AlignCenter);
	label->setFrameShape(QFrame::Box);
	label->setFrameShadow(QFrame::Sunken);
	label->installEventFilter(this);
	label->setPixmap(QPixmap(16, 16));
	label->setToolTip(AToolTip);
	FKeyByLabel.insert(label, key);
	return label;
}

void SelectIconWidget::createLabels(const QString &AColor)
{
	Q_UNUSED(AColor)
	int columns = 16;
	int rows = 17;
	int row =0;
	int column = 0;
	for (QMap<uint, EmojiData>::ConstIterator it=FEmojiMap.constBegin(); it!=FEmojiMap.constEnd(); ++it)
	{
		FLayout->addWidget(getIconLabel((*it).unicode, (*it).name), row, column);
		if ((*it).colored)
			FHasColored=true;
		column = (column+1) % columns;
		row += column==0 ? 1 : 0;
		FLayout->setRowStretch(row, 0);
	}
	if (row<rows)
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

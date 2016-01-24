#include "selecticonwidget.h"
#include "emoji.h"
#include <QCursor>
#include <QToolTip>
#include <QTextDocument>

#include <utils/qt4qt5compat.h>

SelectIconWidget::SelectIconWidget(const QString &ACategory, const QString &AColor, IEmoji *AEmoji, QWidget *AParent):
	QWidget(AParent),
	FEmoji(AEmoji),
	FPressed(NULL),
	FEmojiMap((AEmoji->emojiData(ACategory))),
	FHasColored(false)
{
	FLayout = new QGridLayout(this);
	FLayout->setMargin(2);
	FLayout->setHorizontalSpacing(3);
	FLayout->setVerticalSpacing(3);

	createLabels(AColor);
}

SelectIconWidget::~SelectIconWidget()
{}

void SelectIconWidget::updateLabels(const QString &AColor)
{
	for (QMap<QLabel*, QString>::Iterator it=FKeyByLabel.begin(); it!=FKeyByLabel.end(); ++it)
	{
		QString key = *it;
		if (FEmoji->isColored(key))
			key.chop(2);
		QIcon icon;
		if (!AColor.isEmpty())
//			icon = FStorage->getIcon(key+AColor);
			icon = FEmoji->getIcon(key+AColor, QSize(16, 16));
		if (!icon.isNull())
			key.append(AColor);
		if (*it!=key)
		{
			(*it)=key;
//			FStorage->insertAutoIcon(it.key(),key,0,0,"pixmap");
			QLabel *label = it.key();
			label->setPixmap(icon.pixmap(16, 16));
		}
	}
}

QLabel *SelectIconWidget::getIconLabel(const QString &AKey, const QString &AColor)
{
	QLabel *label = new QLabel(this);
	label->setMargin(2);
	label->setAlignment(Qt::AlignCenter);
	label->setFrameShape(QFrame::Box);
	label->setFrameShadow(QFrame::Sunken);
	label->installEventFilter(this);

	QIcon icon;
	if (!AColor.isEmpty())
		icon = FEmoji->getIcon(AKey+AColor, QSize(16, 16));
//		icon = FStorage->getIcon(AKey+AColor);
	QString key(AKey);
	if (!icon.isNull())
		key.append(AColor);
//	FStorage->insertAutoIcon(label, key,0,0,"pixmap");
	label->setPixmap(icon.pixmap(16, 16));
	FKeyByLabel.insert(label, key);
	return label;
}

void SelectIconWidget::createLabels(const QString &AColor)
{
//	QList<QString> keys = FStorage->fileFirstKeys();

	int count(0);
//	for (QList<QString>::ConstIterator it=keys.constBegin(); it!=keys.constEnd(); ++it)
//		if (!FEmoji->isColored(*it) && (*it)!="default")
//			++count;

	count = FEmojiMap.size();

	int columns = count/2 + 1;
	while (columns>1 && columns*columns>count)
		columns--;

	int row =0;
	int column = 0;
//	foreach(QString key, keys)
//		if (FEmoji->isColored(key))
//		{
//			if (key.size()>2)
//				FHasColored=true;
//		}
//		else if (key!="default")
//		{
//			FLayout->addWidget(getIconLabel(key, AColor),row,column);
//			column = (column+1) % columns;
//			row += column==0 ? 1 : 0;
//		}
	for (QMap<uint, EmojiData>::ConstIterator it=FEmojiMap.constBegin(); it!=FEmojiMap.constEnd(); ++it)
	{
		FLayout->addWidget(getIconLabel((*it).unicode, AColor), row, column);
		column = (column+1) % columns;
		row += column==0 ? 1 : 0;
	}
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
		FPressed = NULL;
	}
	return QWidget::eventFilter(AWatched,AEvent);
}

#include "selecticonwidget.h"
#include <QCursor>
#include <QToolTip>
#include <QTextDocument>
#include <utils/qt4qt5compat.h>

SelectIconWidget::SelectIconWidget(IconStorage *AStorage, const QString &AColor, QWidget *AParent) : QWidget(AParent)
{
	FPressed = NULL;
	FStorage = AStorage;

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
	QStringList colorSiffixes;
	QChar first(0xD83C);

	for (ushort i=0xDFFB; i<=0xDFFF; ++i)
	{
		QString suffix;
		suffix.append(first).append(QChar(i));
		colorSiffixes.append(suffix);
	}

	QList<QString> keys = FStorage->fileFirstKeys();

	int index(0);
	foreach(QString key, keys)
	{
		bool colored(false);
		for (QStringList::ConstIterator sit=colorSiffixes.constBegin(); sit!=colorSiffixes.constEnd(); ++sit)
			if (key.endsWith(*sit))
			{
				colored=true;
				break;
			}
		if (!colored)
		{
			QLabel *label = qobject_cast<QLabel *>(FLayout->itemAt(index)->widget());
			QIcon icon;
			if (!AColor.isEmpty())
				icon = FStorage->getIcon(key+AColor);
			if (!icon.isNull())
				key.append(AColor);
			FStorage->insertAutoIcon(label,key,0,0,"pixmap");
			FKeyByLabel.insert(label,key);
			index++;
		}
	}
}

void SelectIconWidget::createLabels(const QString &AColor)
{
	QStringList colorSiffixes;
	QChar first(0xD83C);

	for (ushort i=0xDFFB; i<=0xDFFF; ++i)
	{
		QString suffix;
		suffix.append(first).append(QChar(i));
		colorSiffixes.append(suffix);
	}

	QList<QString> keys = FStorage->fileFirstKeys();

	int count(0);
	for (QList<QString>::ConstIterator kit=keys.constBegin(); kit!=keys.constEnd(); ++kit)
	{
		bool colored(false);
		for (QStringList::ConstIterator sit=colorSiffixes.constBegin(); sit!=colorSiffixes.constEnd(); ++sit)
			if ((*kit).endsWith(*sit))
			{
				colored=true;
				break;
			}
		if (!colored)
			++count;
	}

	int columns = count/2 + 1;
	while (columns>1 && columns*columns>count)
		columns--;

	int row =0;
	int column = 0;
	foreach(QString key, keys)
	{
		bool colored(false);
		for (QStringList::ConstIterator sit=colorSiffixes.constBegin(); sit!=colorSiffixes.constEnd(); ++sit)
			if (key.endsWith(*sit))
			{
				colored=true;
				break;
			}
		if (!colored)
		{
			QLabel *label = new QLabel(this);
			label->setMargin(2);
			label->setAlignment(Qt::AlignCenter);
			label->setFrameShape(QFrame::Box);
			label->setFrameShadow(QFrame::Sunken);
			label->installEventFilter(this);

			QIcon icon;
			if (!AColor.isEmpty())
				icon = FStorage->getIcon(key+AColor);
			if (!icon.isNull())
				key.append(AColor);
			FStorage->insertAutoIcon(label,key,0,0,"pixmap");

			FKeyByLabel.insert(label,key);
			FLayout->addWidget(label,row,column);
			column = (column+1) % columns;
			row += column==0 ? 1 : 0;
		}
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
			emit iconSelected(FStorage->subStorage(),FKeyByLabel.value(label));
		FPressed = NULL;
	}
	return QWidget::eventFilter(AWatched,AEvent);
}

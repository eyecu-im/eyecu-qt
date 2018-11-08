#include "selectsubiconmenu.h"
#include "selectsubiconwidget.h"

SelectSubIconMenu::SelectSubIconMenu(IEmoji *AEmoji, QWidget *AParent):
	Menu(AParent),
	FEmoji(AEmoji)
{
	FLayout = new QVBoxLayout(this);
	FLayout->setMargin(0);
	setAttribute(Qt::WA_AlwaysShowToolTips,true);
	connect(this,SIGNAL(aboutToShow()),SLOT(onAboutToShow()));
}

SelectSubIconMenu::~SelectSubIconMenu()
{}

void SelectSubIconMenu::setEmojiIds(const QStringList &AEmojiIds)
{
	FEmojiIds = AEmojiIds;
}

QSize SelectSubIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectSubIconMenu::onAboutToShow()
{
	QWidget *widget = new SelectSubIconWidget(FEmojiIds, FEmoji, this);
	FLayout->addWidget(widget);
	connect(this, SIGNAL(aboutToHide()), widget, SLOT(deleteLater()));
	connect(widget, SIGNAL(iconSelected(QString,QString)),
					SIGNAL(iconSelected(QString,QString)));
}

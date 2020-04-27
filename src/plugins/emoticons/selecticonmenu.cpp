#include "selecticonmenu.h"

#include <definitions/resources.h>
// *** <<< eyeCU <<< ***
#include <definitions/toolbargroups.h>
#include <utils/toolbarchanger.h>
// *** >>> eyeCU >>> ***
#include <utils/iconstorage.h>


SelectIconMenu::SelectIconMenu(const QString &AIconset, IEmoticons *AEmoticons, QWidget *AParent) : Menu(AParent)
{
	FEmoticons = AEmoticons;
	FStorage = NULL;
	setIconset(AIconset);

	FLayout = new QVBoxLayout(this);
	FLayout->setMargin(0);
	setAttribute(Qt::WA_AlwaysShowToolTips,true);
	connect(this,SIGNAL(aboutToShow()),SLOT(onAboutToShow()));
}

SelectIconMenu::~SelectIconMenu()
{

}

QString SelectIconMenu::iconset() const
{
	return FStorage!=NULL ? FStorage->subStorage() : QString();
}

void SelectIconMenu::setIconset(const QString &ASubStorage)
{
	if (FStorage==NULL || FStorage->subStorage()!=ASubStorage)
	{
		delete FStorage;
		FStorage = new IconStorage(RSR_STORAGE_EMOTICONS,ASubStorage,this);
		FStorage->insertAutoIcon(this,FStorage->fileKeys().value(0));
	}
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{
	SelectIconWidget *widget = new SelectIconWidget(FStorage,this); // *** <<< eyeCU >>> ***
	FLayout->addWidget(widget);
	connect(this,SIGNAL(aboutToHide()),widget,SLOT(deleteLater()));
	connect(widget,SIGNAL(iconSelected(const QString &, const QString &)),SIGNAL(iconSelected(const QString &, const QString &)));
// *** <<< eyeCU <<< ***
	QStringList recent = FEmoticons->recentIcons(FStorage->subStorage());
	if (!recent.isEmpty())
	{
		QToolBar *toolBar = new QToolBar(this);
//		toolBar->setIconSize(QSize(16,16));
		FLayout->addWidget(toolBar);
		ToolBarChanger changer(toolBar);
		for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
		{
			QLabel *label = widget->getIconLabel(*it);
			changer.insertWidget(label, TBG_MWSIM_RECENT);
		}
		connect(this,SIGNAL(aboutToHide()),toolBar,SLOT(deleteLater()));
	}
// *** >>> eyeCU >>> ***
}

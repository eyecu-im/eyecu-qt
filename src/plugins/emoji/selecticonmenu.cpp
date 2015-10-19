#include "selecticonmenu.h"

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <utils/iconstorage.h>
#include <utils/toolbarchanger.h>
#include <utils/action.h>

SelectIconMenu::SelectIconMenu(const QString &AIconset, QWidget *AParent) : Menu(AParent)
{
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
	return FStorage!=NULL ? FStorage->subStorage() : QString::null;
}

void SelectIconMenu::setIconset(const QString &ASubStorage)
{
	QStringList colorSiffixes;
	QChar first(0xD83C);

	for (ushort i=0xDFFB; i<=0xDFFF; ++i)
	{
		QString suffix;
		suffix.append(first).append(QChar(i));
		colorSiffixes.append(suffix);
	}

	if (FStorage==NULL || FStorage->subStorage()!=ASubStorage)
	{
		delete FStorage;
		FStorage = new IconStorage(RSR_STORAGE_EMOJI,ASubStorage,this);

		QList<QString> keys = FStorage->fileKeys();
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
			{
				FStorage->insertAutoIcon(this, *kit);
				menuAction()->setToolTip(FStorage->storageProperty(FILE_STORAGE_NAME, ASubStorage));
				break;
			}
		}
	}
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{
	QWidget *widget = new SelectIconWidget(FStorage,this);
	FLayout->addWidget(widget);

	connect(this,SIGNAL(aboutToHide()),widget,SLOT(deleteLater()));
	connect(widget,SIGNAL(iconSelected(const QString &, const QString &)),SIGNAL(iconSelected(const QString &, const QString &)));

	QToolBar *toolBar = new QToolBar(this);
	FLayout->addWidget(toolBar);
	ToolBarChanger changer(toolBar);

	Menu *menu = new Menu(toolBar);
	menu->setToolTip(tr("Skin color"));

	Action *action = new Action(menu);
	action->setText(tr("Default"));
	menu->addAction(action);

	const QChar first(0xD83C);

	for (int i=0; i<5; i++)
	{
		QString icon;
		icon.append(first).append(QChar(0xDFFB+i));
		action = new Action(menu);
		action->setText(tr("Fitzpatrick type %1").arg(i?QString::number(i+2):tr("1 or 2")));
		action->setIcon(RSR_STORAGE_EMOJI, icon);
		menu->addAction(action);
	}

	QToolButton *button = changer.insertAction(menu->menuAction(), AG_EMOJI_SKINCOLOR);
	button->setPopupMode(QToolButton::InstantPopup);

	connect(this,SIGNAL(aboutToHide()),toolBar,SLOT(deleteLater()));
}

#include "selecticonmenu.h"
#include "selecticonwidget.h"
#include "emoji.h"
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>
#include <utils/action.h>
#include <utils/logger.h>
#include <utils/qt4qt5compat.h>

#define ADR_COLOR Action::DR_Parametr1
#define ADR_GENDER Action::DR_Parametr1
#define ADR_EMOJI Action::DR_Parametr1

SelectIconMenu::SelectIconMenu(const QString &AIconSet, IEmoji *AEmoji, QWidget *AParent):
	Menu(AParent),
	FEmoji(AEmoji),
	FLayout(nullptr),
	FTabWidget(nullptr),
	FToolBarChanger(nullptr)
{
	FLayout = new QVBoxLayout(this);
	FLayout->setMargin(0);
	FLayout->setSpacing(1);

	setIconSet(AIconSet);

	QPixmap pixmap(16, 16);
	pixmap.fill(QColor(0, 0, 0, 0));
	FEmptyIcon.addPixmap(pixmap);

	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	connect(this, SIGNAL(aboutToShow()), SLOT(onAboutToShow()));
	connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)), SLOT(onOptionsChanged(OptionsNode)));
}

SelectIconMenu::~SelectIconMenu()
{}

QString SelectIconMenu::iconSet() const
{
	return QString::null;
}

void SelectIconMenu::setIconSet(const QString &AIconSet)
{
	menuAction()->setIcon(FEmoji->getIcon(FEmoji->emojiData(IEmoji::People)
										  .constBegin().value()->id(),
										  QSize(16, 16)));
	menuAction()->setToolTip(AIconSet);
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{	
	if (!FTabWidget)
	{
		FTabWidget = new QTabWidget(this);
		FLayout->addWidget(FTabWidget);
		unsigned columns = 0;
		unsigned rows = 0;
		unsigned count = 0;
		for (int c = IEmoji::People; c<IEmoji::Last; ++c)
		{
			unsigned cnt = FEmoji->categoryCount(IEmoji::Category(c));
			if (count < cnt)
			{
				count = cnt;
				unsigned cols = cnt/2 + 1;
				while (cols>1 && cols*cols>cnt)
					cols--;
				unsigned r = cnt/cols;
				if (cols*r<cnt)
					r++;
				if (r>cols)
				{
					cols++;
					r--;
				}
				columns=cols;
				rows=r;
			}
		}

		for (int c = IEmoji::People; c<IEmoji::Last; ++c)
		{
			SelectIconWidget *widget = new SelectIconWidget(IEmoji::Category(c), columns, rows, FEmoji, this);
			FTabWidget->setTabToolTip(FTabWidget->addTab(widget, FEmoji->categoryIcon(IEmoji::Category(c)),
														 QString()), FEmoji->categoryName(IEmoji::Category(c)));
			FTabWidget->setTabEnabled(c, FEmoji->categoryCount(IEmoji::Category(c)));
			connect(widget,SIGNAL(iconSelected(QString)),SLOT(onIconSelected(QString)));
			connect(widget,SIGNAL(hasColoredChanged(bool)),SLOT(onHasColoredChanged(bool)));
			connect(widget,SIGNAL(hasGenderedChanged(bool)),SLOT(onHasGenderedChanged(bool)));
		}
		FTabWidget->setCurrentIndex(Options::node(OPV_MESSAGES_EMOJI_CATEGORY).value().toInt());
		connect(FTabWidget, SIGNAL(currentChanged(int)), SLOT(onCategorySwitched(int)));
	}

	// Create toolbar
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize size(extent, extent);
	if (FToolBarChanger)
		delete FToolBarChanger->toolBar();
	QToolBar *toolBar = new QToolBar(this);
	toolBar->setIconSize(size);
	FLayout->addWidget(toolBar);
	FToolBarChanger = new ToolBarChanger(toolBar);
	FToolBarChanger->setSeparatorsVisible(true);

	// Skin color submenu
	int color = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	FSkinColor = new Menu(toolBar);
	FSkinColor->setIcon(FEmptyIcon);
	QActionGroup *group = new QActionGroup(FSkinColor);

	for (int i=Emoji::SkinDefault; i<=Emoji::SkinTone5; ++i)
	{
		Action *action = new Action(group);
		action->setText(i==Emoji::SkinDefault?tr("Default")
											 :tr("Fitzpatrick type %1", "https://en.wikipedia.org/wiki/Fitzpatrick_scale")
												.arg(i==Emoji::SkinTone1?QString::number(i+1):tr("1 or 2")));
		action->setIcon(FEmoji->getIcon(FEmoji->skinColorSuffix(IEmoji::SkinColor(i)), size));
		action->setData(ADR_COLOR, i);
		action->setCheckable(true);
		action->setActionGroup(group);
		FSkinColor->addAction(action);
		if (i == color)
		{
			action->setChecked(true);
			FSkinColor->setIcon(action->icon());
		}
		connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
	}
	FToolBarChanger->insertAction(FSkinColor->menuAction(), TBG_MWSIM_SKINCOLOR)->setPopupMode(QToolButton::InstantPopup);
	FSkinColor->setTitle(tr("Skin color"));

	// Gender submenu
	int gender = Options::node(OPV_MESSAGES_EMOJI_GENDER).value().toInt();
	FGender = new Menu(toolBar);
	group = new QActionGroup(FGender);

	QStringList genderNames;
	genderNames << tr("Default") << tr("Male") << tr("Female");

	for (int i = IEmoji::GenderDefault; i <= IEmoji::GenderFemale; i++)
	{
		Action *action = new Action(group);
		action->setText(genderNames[i]);
		action->setData(ADR_GENDER, i);
		action->setCheckable(true);
		action->setActionGroup(group);
		action->setIcon(FEmoji->getIcon(FEmoji->genderSuffix(Emoji::Gender(i)), size));
		FGender->addAction(action);
		if (i == gender)
		{
			action->setChecked(true);
			FGender->setIcon(action->icon());
		}
		connect(action, SIGNAL(triggered(bool)), SLOT(onGenderSelected()));
	}

	FToolBarChanger->insertAction(FGender->menuAction(), TBG_MWSIM_SKINCOLOR)->setPopupMode(QToolButton::InstantPopup);
	FGender->setTitle(tr("Gender"));

	QStringList recent = FEmoji->recentIcons(QString());
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		const IEmojiData *data = FEmoji->findData(*it, IEmoji::SkinColor(color),
													   IEmoji::Gender(gender));
		if (data)
		{
			QIcon icon = FEmoji->getIcon(data->id(), size);
			if (!icon.isNull())
			{
				const IEmojiData *basicData = FEmoji->findData(*it);
				Action *action = new Action();
				action->setIcon(icon);
				action->setToolTip(basicData->name());
				action->setData(ADR_EMOJI, *it);
				FToolBarChanger->insertAction(action, TBG_MWSIM_RECENT);
				connect(action, SIGNAL(triggered()), SLOT(onRecentIconTriggered()));
			}
		}
		else
			LOG_WARNING("No data found for recent emoji");
	}
}

void SelectIconMenu::onSkinColorSelected()
{
	Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).
			setValue(qobject_cast<Action *>(sender())->data(ADR_COLOR));
}

void SelectIconMenu::onGenderSelected()
{
	Options::node(OPV_MESSAGES_EMOJI_GENDER).
			setValue(qobject_cast<Action *>(sender())->data(ADR_GENDER));
}

void SelectIconMenu::onOptionsChanged(const OptionsNode &ANode)
{
	if (isVisible() &&
		(ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR ||
		 ANode.path() == OPV_MESSAGES_EMOJI_GENDER))
	{
		int index = ANode.value().toInt();

		if (ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR) // Skin color
			FSkinColor->setIcon(index?FEmoji->getIcon(FEmoji->skinColorSuffix(IEmoji::SkinColor(index)))
									 :FEmptyIcon);
		else // Gender
			FGender->setIcon(index?FEmoji->getIcon(FEmoji->genderSuffix(IEmoji::Gender(index)))
								  :FEmptyIcon);
		SelectIconWidget *widget = qobject_cast<SelectIconWidget *>(qobject_cast<QTabWidget *>(FLayout->itemAt(0)->widget())->currentWidget());
		if (widget)
			widget->updateLabels();
		updateRecentActions();
	}
}

void SelectIconMenu::onRecentIconTriggered()
{
	hide();
	Action *action = qobject_cast<Action*>(sender());
	if (action)
		emit iconSelected(action->data(ADR_EMOJI).toString());
}

void SelectIconMenu::onHasColoredChanged(bool AHasColored)
{
	FSkinColor->setEnabled(AHasColored);
}

void SelectIconMenu::onHasGenderedChanged(bool AHasGendered)
{
	FGender->setEnabled(AHasGendered);
}

void SelectIconMenu::onCategorySwitched(int ACategory)
{
	Options::node(OPV_MESSAGES_EMOJI_CATEGORY).setValue(ACategory);
}

void SelectIconMenu::onIconSelected(const QString &AIconKey)
{
	hide();
	emit iconSelected(AIconKey);
}

void SelectIconMenu::updateRecentActions()
{
	QList<QAction *> actions = FToolBarChanger->groupItems(TBG_MWSIM_RECENT);
	int color = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	int gender = Options::node(OPV_MESSAGES_EMOJI_GENDER).value().toInt();
	for (QList<QAction *>::ConstIterator it=actions.constBegin(); it!=actions.constEnd(); ++it)
	{
		Action *action = FToolBarChanger->handleAction(*it);
		action->setIcon(FEmoji->getIcon(FEmoji->findData(action->data(ADR_EMOJI).toString(),
														 IEmoji::SkinColor(color),
														 IEmoji::Gender(gender))->id(),
										FToolBarChanger->toolBar()->iconSize()));
	}
}



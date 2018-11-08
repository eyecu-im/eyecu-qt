#include "selecticonmenu.h"
#include "selecticonwidget.h"
#include "emoji.h"
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>
#include <utils/action.h>
#include <utils/qt4qt5compat.h>

#define ADR_COLOR Action::DR_Parametr1
#define ADR_GENDER Action::DR_Parametr1
#define ADR_EMOJI Action::DR_Parametr1

const QStringList SelectIconMenu::FGenderSuffixes(QStringList() << "2642" << "2640");
const QStringList SelectIconMenu::FSkinColorSuffixes(QStringList() << "1f3fb"
																   << "1f3fc"
																   << "1f3fd"
																   << "1f3fe"
																   << "1f3ff");

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
			connect(widget,SIGNAL(iconSelected(QString)),SIGNAL(iconSelected(QString)));
			connect(widget,SIGNAL(hasColoredChanged(bool)), SLOT(onHasColoredChanged(bool)));
			connect(widget,SIGNAL(hasGenderedChanged(bool)), SLOT(onHasGenderedChanged(bool)));
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

	Action *action = new Action(group);
	action->setText(tr("Default"));
	action->setData(ADR_COLOR, Emoji::SkinDefault);
	action->setCheckable(true);
	action->setActionGroup(group);
	action->setIcon(FSkinColor->icon());
	FSkinColor->addAction(action);

	if (color==0)
	{
		action->setChecked(true);
		FSkinColor->setIcon(action->icon());
	}
	connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
//	QStringList colorSuffixes = FEmoji->colorSuffixes();
	for (int i=Emoji::SkinTone1; i<=Emoji::SkinTone5; ++i)
	{
		action = new Action(group);
		action->setText(tr("Fitzpatrick type %1", "https://en.wikipedia.org/wiki/Fitzpatrick_scale")
						.arg(i==Emoji::SkinTone1?QString::number(i+1):tr("1 or 2")));
		action->setIcon(FEmoji->getIcon(FSkinColorSuffixes[i-1], size));
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
	FGender->setIcon(FEmptyIcon);
	group = new QActionGroup(FGender);

//	QStringList genderSuffixes = FEmoji->genderSuffixes();

	action = new Action(group);
	action->setText(tr("Default"));
	action->setData(ADR_GENDER, Emoji::GenderDefault);
	action->setCheckable(true);
	action->setActionGroup(group);
	action->setIcon(FGender->icon());
	FGender->addAction(action);
	if (gender==Emoji::GenderDefault)
	{
		action->setChecked(true);
		FGender->setIcon(action->icon());
	}
	connect(action, SIGNAL(triggered(bool)), SLOT(onGenderSelected()));

	action = new Action(group);
	action->setText(tr("Male"));
	action->setData(ADR_GENDER, Emoji::GenderMale);
	action->setCheckable(true);
	action->setActionGroup(group);
	action->setIcon(FEmoji->getIcon(FGenderSuffixes[0], size));
	FGender->addAction(action);
	if (gender==Emoji::GenderMale)
	{
		action->setChecked(true);
		FGender->setIcon(action->icon());
	}
	connect(action, SIGNAL(triggered(bool)), SLOT(onGenderSelected()));

	action = new Action(group);
	action->setText(tr("Female"));
	action->setData(ADR_GENDER, Emoji::GenderFemale);
	action->setCheckable(true);
	action->setActionGroup(group);
	action->setIcon(FEmoji->getIcon(FGenderSuffixes[1], size));
	FGender->addAction(action);
	if (gender==Emoji::GenderFemale)
	{
		action->setChecked(true);
		FGender->setIcon(action->icon());
	}
	connect(action, SIGNAL(triggered(bool)), SLOT(onGenderSelected()));

	FToolBarChanger->insertAction(FGender->menuAction(), TBG_MWSIM_SKINCOLOR)->setPopupMode(QToolButton::InstantPopup);
	FGender->setTitle(tr("Gender"));

	QStringList recent = FEmoji->recentIcons(QString());
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		const IEmojiData *data = FEmoji->findData(*it);
		QString id = data->id();
		QString name = data->name();
		if (color && data->diversities().size() >= color)
			data = FEmoji->findData(data->diversities()[color-1]);
		if (gender && data->genders().size() == 2)
			data = FEmoji->findData(data->genders()[gender-1]);
		QIcon icon = FEmoji->getIcon(data->id(), size);
		if (!icon.isNull())
		{
			Action *action = new Action();
			action->setIcon(icon);
			action->setToolTip(name);
			action->setData(ADR_EMOJI, id);
			FToolBarChanger->insertAction(action, TBG_MWSIM_RECENT);
			connect(action, SIGNAL(triggered()), SLOT(onRecentIconTriggered()));
		}
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
			FSkinColor->setIcon(index?FEmoji->getIcon(FSkinColorSuffixes[index-1])
									 :FEmptyIcon);
		else // Gender
			FGender->setIcon(index?FEmoji->getIcon(FGenderSuffixes[index-1])
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

void SelectIconMenu::updateRecentActions()
{
	QList<QAction *> actions = FToolBarChanger->groupItems(TBG_MWSIM_RECENT);
	int color = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	int gender = Options::node(OPV_MESSAGES_EMOJI_GENDER).value().toInt();
	for (QList<QAction *>::ConstIterator it=actions.constBegin(); it!=actions.constEnd(); ++it)
	{
		Action *action = FToolBarChanger->handleAction(*it);
		QString id = action->data(ADR_EMOJI).toString();

		const IEmojiData *data = FEmoji->findData(id);
		if (color && !data->diversities().isEmpty() &&
			color <= data->diversities().size()) // Has skin color color
			data = FEmoji->findData(data->diversities()[color-1]);

		if (gender && data->genders().size() == 2) // Has skin color color
			data = FEmoji->findData(data->genders()[gender-1]);

		action->setIcon(FEmoji->getIcon(data->id(), FToolBarChanger->toolBar()->iconSize()));
	}
}



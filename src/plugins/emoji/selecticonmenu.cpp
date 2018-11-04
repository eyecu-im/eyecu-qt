#include "selecticonmenu.h"
#include "emoji.h"
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>
#include <utils/action.h>
#include <utils/qt4qt5compat.h>

#define ADR_COLOR Action::DR_Parametr1
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
	menuAction()->setIcon(FEmoji->getIcon(FEmoji->emojiData(IEmoji::People).constBegin().value().id,
										  QSize(16, 16)));
	menuAction()->setToolTip(AIconSet);
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{
	int index = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
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
		}
		FTabWidget->setCurrentIndex(Options::node(OPV_MESSAGES_EMOJI_CATEGORY).value().toInt());
		connect(FTabWidget, SIGNAL(currentChanged(int)), SLOT(onCategorySwitched(int)));
	}
	int extent = Options::node(OPV_MESSAGES_EMOJI_SIZE_MENU).value().toInt();
	QSize size(extent, extent);
	if (FToolBarChanger)
		delete FToolBarChanger->toolBar();
	QToolBar *toolBar = new QToolBar(this);
	toolBar->setIconSize(size);
	FLayout->addWidget(toolBar);
	FToolBarChanger = new ToolBarChanger(toolBar);
	FToolBarChanger->setSeparatorsVisible(true);
	FMenu = new Menu(toolBar);
	FMenu->setIcon(FEmptyIcon);
	QActionGroup *group = new QActionGroup(FMenu);

	Action *action = new Action(group);
	action->setText(tr("Default"));
	action->setData(ADR_COLOR, Emoji::SkinDefault);
	action->setCheckable(true);
	action->setActionGroup(group);
	action->setIcon(FMenu->icon());
	FMenu->addAction(action);
	if (index==0)
	{
		action->setChecked(true);
		FMenu->setIcon(action->icon());
	}
	connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
	QStringList colorSuffixes = FEmoji->colorSuffixes();
	for (int i=Emoji::SkinTone1; i<=Emoji::SkinTone5; ++i)
	{
		QString c = colorSuffixes[i-1];
		action = new Action(group);
		action->setText(tr("Fitzpatrick type %1", "https://en.wikipedia.org/wiki/Fitzpatrick_scale")
						.arg(i==Emoji::SkinTone1?QString::number(i+1):tr("1 or 2")));
		action->setIcon(FEmoji->getIcon(c, size));
		action->setData(ADR_COLOR, i);
		action->setCheckable(true);
		action->setActionGroup(group);
		FMenu->addAction(action);
		if (i == index)
		{
			action->setChecked(true);
			FMenu->setIcon(action->icon());
		}
		connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
	}
	FToolBarChanger->insertAction(FMenu->menuAction(), TBG_MWSIM_SKINCOLOR)->setPopupMode(QToolButton::InstantPopup);
	FMenu->setTitle(tr("Skin color"));
	QStringList recent = FEmoji->recentIcons(QString());
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		EmojiData data = FEmoji->findData(*it);
		if (index && data.diversities.size() >= index)
			data = FEmoji->findData(data.diversities[index-1]);
		QIcon icon = FEmoji->getIcon(data.id, size);
		if (!icon.isNull())
		{
			Action *action = new Action();
			action->setIcon(icon);
			action->setToolTip(FEmoji->findData(*it).name);
			action->setData(ADR_EMOJI, data.id);
			FToolBarChanger->insertAction(action, TBG_MWSIM_RECENT);
			connect(action, SIGNAL(triggered()), SLOT(onRecentIconTriggered()));
		}
	}
}

void SelectIconMenu::onSkinColorSelected()
{
	Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).setValue(qobject_cast<Action *>(sender())->data(ADR_COLOR));
}

void SelectIconMenu::onOptionsChanged(const OptionsNode &ANode)
{
	if (isVisible() && ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR)
	{
		int index = ANode.value().toInt();
		FMenu->setIcon(index?FEmoji->getIcon(FEmoji->colorSuffixes()[index-1]):FEmptyIcon);
		SelectIconWidget *widget = qobject_cast<SelectIconWidget *>(qobject_cast<QTabWidget *>(FLayout->itemAt(0)->widget())->currentWidget());
		if (widget)
			widget->updateLabels(index);
		updateRecentActions(index);
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
	FMenu->setEnabled(AHasColored);
}

void SelectIconMenu::onCategorySwitched(int ACategory)
{
	Options::node(OPV_MESSAGES_EMOJI_CATEGORY).setValue(ACategory);
}

void SelectIconMenu::updateRecentActions(int AColor)
{
	QList<QAction *> actions = FToolBarChanger->groupItems(TBG_MWSIM_RECENT);
	for (QList<QAction *>::ConstIterator it=actions.constBegin(); it!=actions.constEnd(); ++it)
	{
		Action *action = FToolBarChanger->handleAction(*it);
		QString id = action->data(ADR_EMOJI).toString();

		EmojiData data = FEmoji->findData(id);
		if (!data.diversities.isEmpty()) // Has skin color color
		{
			if (AColor)
			{
				id = data.diversities[AColor-1];
				data = FEmoji->findData(id);
			}
			QIcon icon = FEmoji->getIcon(id, QSize(16, 16));
			action->setIcon(icon);
			action->setToolTip(data.name);
		}
	}
}



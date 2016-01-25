#include "selecticonmenu.h"
#include "emoji.h"
#include <QDebug>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>
#include <utils/toolbarchanger.h>
#include <utils/action.h>
#include <utils/qt4qt5compat.h>

#define ADR_COLOR Action::DR_Parametr1
#define ADR_EMOJI Action::DR_Parametr1

SelectIconMenu::SelectIconMenu(const QString &AIconset, IEmoji *AEmoji, QWidget *AParent):
	Menu(AParent),
	FEmoji(AEmoji),
	FTabWidget(NULL),
	FToolBar(NULL)
{
	setIconset(AIconset);

	FLayout = new QVBoxLayout(this);
	FLayout->setMargin(0);
	FLayout->setSpacing(1);
	setAttribute(Qt::WA_AlwaysShowToolTips,true);
	connect(this,SIGNAL(aboutToShow()),SLOT(onAboutToShow()));
	connect(Options::instance(),SIGNAL(optionsChanged(OptionsNode)),SLOT(onOptionsChanged(OptionsNode)));
}

SelectIconMenu::~SelectIconMenu()
{}

QString SelectIconMenu::iconset() const
{
	return QString::null;
}

void SelectIconMenu::setIconset(const QString &ASubStorage)
{
	menuAction()->setIcon(FEmoji->getIcon(FEmoji->emojiData(FEmoji->categories().at(0)).constBegin().value().unicode, QSize(16, 16)));
	menuAction()->setToolTip(ASubStorage);
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{
	int index = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	QString color = index?FEmoji->colorSuffixes()[index-1]:QString();

	if (!FTabWidget)
	{
		FTabWidget = new QTabWidget(this);
		FLayout->addWidget(FTabWidget);

		QList<QString> categories = FEmoji->categories();
		SelectIconWidget *selectedWidget = NULL;
		for (QList<QString>::ConstIterator it = categories.constBegin(); it!=categories.constEnd(); ++it)
		{
			SelectIconWidget *widget = new SelectIconWidget(*it, FEmoji, this);
			if (!selectedWidget)
				selectedWidget = widget;
			FTabWidget->addTab(widget, *it);
			connect(widget,SIGNAL(iconSelected(QString, QString)),SIGNAL(iconSelected(QString, QString)));
			connect(widget,SIGNAL(hasColoredChanged(bool)), SLOT(onHasColoredChanged(bool)));
		}
		FTabWidget->setCurrentWidget(selectedWidget);

		FToolBar = new QToolBar(this);
		FToolBar->setIconSize(QSize(16,16));
		FLayout->addWidget(FToolBar);
		ToolBarChanger changer(FToolBar);
		changer.setSeparatorsVisible(true);

		const QChar first(0xD83C);
		FMenu = new Menu(FToolBar);
		FMenu->setIcon(RSR_STORAGE_EMOJI, "default");

		QActionGroup *group = new QActionGroup(FMenu);

		Action *action = new Action(group);
		action->setText(tr("Default"));
		action->setData(ADR_COLOR, 0);
		action->setCheckable(true);
		action->setActionGroup(group);
		action->setIcon(FMenu->icon());
		FMenu->addAction(action);
		if (color.isEmpty())
		{
			action->setChecked(true);
			FMenu->setIcon(action->icon());
		}
		connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));

		for (int i=1; i<6; i++)
		{
			QString icon(first);
			icon.append(QChar(0xDFFA+i));
			action = new Action(group);
			action->setText(tr("Fitzpatrick type %1", "https://en.wikipedia.org/wiki/Fitzpatrick_scale").arg(i?QString::number(i+2):tr("1 or 2")));
			action->setIcon(RSR_STORAGE_EMOJI, icon);
			action->setData(ADR_COLOR, i);
			action->setCheckable(true);
			action->setActionGroup(group);
			FMenu->addAction(action);
			if (icon == color)
			{
				action->setChecked(true);
				FMenu->setIcon(action->icon());
			}
			connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
		}

		QToolButton *button = changer.insertAction(FMenu->menuAction(), TBG_MWSIM_SKINCOLOR);
		button->setPopupMode(QToolButton::InstantPopup);
		FMenu->setTitle(tr("Skin color"));
	}
	QStringList recent = FEmoji->recentIcons("emojione");
	ToolBarChanger changer(FToolBar);
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		Action *action = new Action();
		action->setIcon(FEmoji->getIcon(*it+color, QSize(16, 16)));
		action->setToolTip(FEmoji->findData(*it).name);
		action->setData(ADR_EMOJI, *it+color);
		changer.insertAction(action, TBG_MWSIM_RECENT);
		connect(action, SIGNAL(triggered()), SLOT(onRecentIconTriggered()));
	}
}

void SelectIconMenu::onSkinColorSelected()
{
	Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).setValue(qobject_cast<Action *>(sender())->data(ADR_COLOR));
}

void SelectIconMenu::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR)
	{
		int index = ANode.value().toInt();
		QString icon = index?FEmoji->colorSuffixes()[index-1]:QString("default");
		FMenu->setIcon(RSR_STORAGE_EMOJI, icon);
		SelectIconWidget *widget = qobject_cast<SelectIconWidget *>(qobject_cast<QTabWidget *>(FLayout->itemAt(0)->widget())->currentWidget());
		if (widget)
			widget->updateLabels(index?icon:QString());
	}
}

void SelectIconMenu::onRecentIconTriggered()
{
	Action *action = qobject_cast<Action*>(sender());
	if (action)
		emit iconSelected(action->data(ADR_EMOJI).toString(), action->toolTip());
}

void SelectIconMenu::onHasColoredChanged(bool AHasColored)
{
	FMenu->setEnabled(AHasColored);
}

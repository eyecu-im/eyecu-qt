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
	FTabWidget(NULL)
//	FStorage(NULL)
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
//	return FStorage!=NULL ? FStorage->subStorage() : QString::null;
	return QString::null;
}

void SelectIconMenu::setIconset(const QString &ASubStorage)
{
	qDebug() << "SelectIconMenu::setIconset(" << ASubStorage << ")";
	QList<QString> categories = FEmoji->categories();
	QMap<uint, EmojiData> emojiData = FEmoji->emojiData(categories.at(0));
	EmojiData data = emojiData.constBegin().value();
	QIcon icon = FEmoji->getIcon(data.unicode, QSize(16, 16));
	menuAction()->setIcon(icon);
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
		SelectIconWidget *selectedWidget(NULL);
		for (QList<QString>::ConstIterator it = categories.constBegin(); it!=categories.constEnd(); ++it)
		{
			SelectIconWidget *widget = new SelectIconWidget(*it, FEmoji, this);
			if (!selectedWidget)
				selectedWidget = widget;
			FTabWidget->addTab(widget, *it);
			connect(widget,SIGNAL(iconSelected(const QString &)),SIGNAL(iconSelected(const QString &)));
			connect(widget,SIGNAL(hasColoredChanged(bool)), SLOT(onHasColoredChanged(bool)));
		}
		FTabWidget->setCurrentWidget(selectedWidget);

		QToolBar *toolBar = new QToolBar(this);
		toolBar->setIconSize(QSize(16,16));
		FLayout->addWidget(toolBar);
		ToolBarChanger changer(toolBar);
		changer.setSeparatorsVisible(true);

		const QChar first(0xD83C);
		FMenu = new Menu(toolBar);
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

		QStringList recent = FEmoji->recentIcons("emojione");
		for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
		{
			QLabel *label = selectedWidget->getIconLabel(*it, color);
			changer.insertWidget(label, TBG_MWSIM_RECENT);
		}
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
		qDebug() << "SelectIconMenu::onOptionsChanged(OPV_MESSAGES_EMOJI_SKINCOLOR)";
		int index = ANode.value().toInt();
		qDebug() << "index=" << index;
		QString icon = index?FEmoji->colorSuffixes()[index-1]:QString("default");
		qDebug() << "icon=" << icon;
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
		emit iconSelected(action->data(ADR_EMOJI).toString());
}

void SelectIconMenu::onHasColoredChanged(bool AHasColored)
{
	FMenu->setEnabled(AHasColored);
}

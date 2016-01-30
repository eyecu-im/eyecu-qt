#include "selecticonmenu.h"
#include "emoji.h"
#include <QDebug>
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
	FTabWidget(NULL),
	FToolBarChanger(NULL)
{
	setIconSet(AIconSet);

	QPixmap pixmap(16, 16);
	pixmap.fill(QColor(0, 0, 0, 0));
	FEmptyIcon.addPixmap(pixmap);

	FLayout = new QVBoxLayout(this);
	FLayout->setMargin(0);
	FLayout->setSpacing(1);
	setAttribute(Qt::WA_AlwaysShowToolTips,true);
	connect(this,SIGNAL(aboutToShow()),SLOT(onAboutToShow()));
	connect(Options::instance(),SIGNAL(optionsChanged(OptionsNode)),SLOT(onOptionsChanged(OptionsNode)));
}

SelectIconMenu::~SelectIconMenu()
{}

QString SelectIconMenu::iconSet() const
{
	return QString::null;
}

void SelectIconMenu::setIconSet(const QString &AIconSet)
{
	menuAction()->setIcon(FEmoji->getIcon(FEmoji->emojiData(IEmoji::People).constBegin().value().unicode, QSize(16, 16)));
	menuAction()->setToolTip(AIconSet);
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

		int columns = 0;
		int rows = 0;
		int count = 0;
		for (int c = IEmoji::People; c<=IEmoji::Foods; ++c)
		{
			int cnt = FEmoji->categoryCount((IEmoji::Category)c);
			if (count<cnt)
			{
				count = cnt;
				int c = cnt/2 + 1;
				while (c>1 && c*c>cnt)
					c--;
				int r = cnt/c;
				if (c*r<cnt)
					r++;
				if (r>c)
				{
					c++;
					r--;
				}
				columns=c;
				rows=r;
			}
		}
		SelectIconWidget *selectedWidget = NULL;
		for (int c = IEmoji::People; c<=IEmoji::Foods; ++c)
		{
			SelectIconWidget *widget = new SelectIconWidget((IEmoji::Category)c, columns, rows, FEmoji, this);
			if (!selectedWidget)
				selectedWidget = widget;
			FTabWidget->setTabToolTip(FTabWidget->addTab(widget, FEmoji->categoryIcon((IEmoji::Category)c), QString()), FEmoji->categoryName((IEmoji::Category)c));
			connect(widget,SIGNAL(iconSelected(QString, QString)),SIGNAL(iconSelected(QString, QString)));
			connect(widget,SIGNAL(hasColoredChanged(bool)), SLOT(onHasColoredChanged(bool)));
		}
		FTabWidget->setCurrentWidget(selectedWidget);
	}
	if (FToolBarChanger)
		delete FToolBarChanger->toolBar();
	QToolBar *toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(16,16));
	FLayout->addWidget(toolBar);
	FToolBarChanger = new ToolBarChanger(toolBar);
	FToolBarChanger->setSeparatorsVisible(true);

	FMenu = new Menu(toolBar);
	FMenu->setIcon(FEmptyIcon);

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

	QStringList colorSuffixes = FEmoji->colorSuffixes();
	for (int i=0; i<5; ++i)
	{
		QString c = colorSuffixes[i];
		action = new Action(group);
		action->setText(tr("Fitzpatrick type %1", "https://en.wikipedia.org/wiki/Fitzpatrick_scale").arg(i?QString::number(i+2):tr("1 or 2")));
		action->setIcon(FEmoji->getIcon(c, QSize(16,16)));
		action->setData(ADR_COLOR, i+1);
		action->setCheckable(true);
		action->setActionGroup(group);
		FMenu->addAction(action);
		if (c == color)
		{
			action->setChecked(true);
			FMenu->setIcon(action->icon());
		}
		connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
	}

	FToolBarChanger->insertAction(FMenu->menuAction(), TBG_MWSIM_SKINCOLOR)->setPopupMode(QToolButton::InstantPopup);
	FMenu->setTitle(tr("Skin color"));
	QStringList recent = FEmoji->recentIcons("emojione");
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		QString emoji = *it+color;
		QIcon icon = FEmoji->getIcon(emoji, QSize(16, 16));
		if (icon.isNull() && !color.isEmpty())
		{
			icon = FEmoji->getIcon(*it, QSize(16, 16));
			emoji = *it;
		}
		if (!icon.isNull())
		{
			Action *action = new Action();
			action->setIcon(icon);
			action->setToolTip(FEmoji->findData(*it).name);
			action->setData(ADR_EMOJI, emoji);
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
	if (ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR)
	{
		int index = ANode.value().toInt();
		QString color;
		if (index)
			color = FEmoji->colorSuffixes()[index-1];
		FMenu->setIcon(index?FEmoji->getIcon(color):FEmptyIcon);
		SelectIconWidget *widget = qobject_cast<SelectIconWidget *>(qobject_cast<QTabWidget *>(FLayout->itemAt(0)->widget())->currentWidget());
		if (widget)
			widget->updateLabels(color);
		QList<QAction *> actions = FToolBarChanger->groupItems(TBG_MWSIM_RECENT);
		for (QList<QAction *>::ConstIterator it=actions.constBegin(); it!=actions.constEnd(); ++it)
		{
			Action *action = FToolBarChanger->handleAction(*it);
			QString unicode = action->data(ADR_EMOJI).toString();
			if (FEmoji->isColored(unicode))
				unicode.chop(2);
			QString emoji = unicode+color;
			QIcon icon = FEmoji->getIcon(emoji, QSize(16, 16));
			if (icon.isNull() && !color.isEmpty())
			{
				icon = FEmoji->getIcon(unicode, QSize(16, 16));
				emoji = unicode;
			}
			if (!icon.isNull())
			{
				action->setIcon(icon);
				action->setToolTip(FEmoji->findData(unicode).name);
				action->setData(ADR_EMOJI, emoji);
			}
		}
	}
}

void SelectIconMenu::onRecentIconTriggered()
{
	hide();
	Action *action = qobject_cast<Action*>(sender());
	if (action)
		emit iconSelected(action->data(ADR_EMOJI).toString(), action->toolTip());
}

void SelectIconMenu::onHasColoredChanged(bool AHasColored)
{
	FMenu->setEnabled(AHasColored);
}

QString SelectIconMenu::typeUcs4(const QString &AText)
{
	QString output;
	QVector<uint> ucs4 = AText.toUcs4();
	for (QVector<uint>::ConstIterator it=ucs4.constBegin(); it!=ucs4.constEnd(); ++it)
	{
		if (!output.isEmpty())
			output.append('-');
		output.append(QString::number(*it, 16));
	}
	return output;
}

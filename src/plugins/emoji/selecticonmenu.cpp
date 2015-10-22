#include "selecticonmenu.h"
#include "emoji.h"

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
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
	FStorage(NULL)
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
	return FStorage!=NULL ? FStorage->subStorage() : QString::null;
}

void SelectIconMenu::setIconset(const QString &ASubStorage)
{
	if (FStorage==NULL || FStorage->subStorage()!=ASubStorage)
	{
		delete FStorage;
		FStorage = new IconStorage(RSR_STORAGE_EMOJI,ASubStorage,this);

		QList<QString> keys = FStorage->fileKeys();
		for (QList<QString>::ConstIterator it=keys.constBegin(); it!=keys.constEnd(); ++it)
			if (!FEmoji->isColored(*it))
			{
				FStorage->insertAutoIcon(this, *it);
				menuAction()->setToolTip(FStorage->storageProperty(FILE_STORAGE_NAME, ASubStorage));
				break;
			}
	}
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{
	QString color = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toString();

	SelectIconWidget *widget = new SelectIconWidget(FStorage, color, FEmoji, this);
	FLayout->addWidget(widget);

	connect(this,SIGNAL(aboutToHide()),widget,SLOT(deleteLater()));
	connect(widget,SIGNAL(iconSelected(const QString &, const QString &)),SIGNAL(iconSelected(const QString &, const QString &)));

	QToolBar *toolBar = new QToolBar(this);
	toolBar->setIconSize(QSize(16,16));
	FLayout->addWidget(toolBar);
	ToolBarChanger changer(toolBar);
	changer.setSeparatorsVisible(true);

	FMenu = new Menu(toolBar);	

	QActionGroup *group = new QActionGroup(FMenu);

	Action *action = new Action(group);
	action->setText(tr("Default"));
	action->setData(ADR_COLOR, QString());
	action->setCheckable(true);
	action->setActionGroup(group);
	if (color.isEmpty())
	{
		action->setChecked(true);
		FMenu->setIcon(QIcon());
	}
	connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
	FMenu->addAction(action);	

	const QChar first(0xD83C);

	for (int i=0; i<5; i++)
	{
		QString icon(first);
		icon.append(QChar(0xDFFB+i));
		action = new Action(group);
		action->setText(tr("Fitzpatrick type %1", "https://en.wikipedia.org/wiki/Fitzpatrick_scale").arg(i?QString::number(i+2):tr("1 or 2")));
		action->setIcon(RSR_STORAGE_EMOJI, icon);
		action->setData(ADR_COLOR, icon);
		action->setCheckable(true);
		action->setActionGroup(group);
		if (icon == color)
		{
			action->setChecked(true);
			FMenu->setIcon(action->icon());
		}
		connect(action, SIGNAL(triggered(bool)), SLOT(onSkinColorSelected()));
		FMenu->addAction(action);
	}

	QToolButton *button = changer.insertAction(FMenu->menuAction(), AG_EMOJI_SKINCOLOR);
	button->setPopupMode(QToolButton::InstantPopup);
	button->setToolTip(tr("Skin color"));

	QStringList recent = FEmoji->recentEmoji(FStorage->subStorage());
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		QLabel *label = widget->getEmojiLabel(*it, color);
		changer.insertWidget(label, AG_EMOJI_RECENT);
	}
	connect(this,SIGNAL(aboutToHide()),toolBar,SLOT(deleteLater()));
}

void SelectIconMenu::onSkinColorSelected()
{
	Action *action=qobject_cast<Action *>(sender());
	Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).setValue(action->data(ADR_COLOR));
}

void SelectIconMenu::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR && FMenu)
	{
		if (ANode.value().toString().isEmpty())
			FMenu->setIcon(QIcon());
		else
			FMenu->setIcon(RSR_STORAGE_EMOJI, ANode.value().toString());
		SelectIconWidget *widget = qobject_cast<SelectIconWidget *>(FLayout->itemAt(0)->widget());
		if (widget)
			widget->updateLabels(ANode.value().toString());
	}
}

void SelectIconMenu::onRecentIconTriggered()
{
	Action *action = qobject_cast<Action*>(sender());
	if (action)
		emit iconSelected(FStorage->subStorage(), action->data(ADR_EMOJI).toString());
}

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
	FEmoji(AEmoji)
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
//	if (FStorage==NULL || FStorage->subStorage()!=ASubStorage)
//	{
//		delete FStorage;
//		FStorage = new IconStorage(RSR_STORAGE_EMOJI,ASubStorage,this);

//		QList<QString> keys = FStorage->fileKeys();
//		for (QList<QString>::ConstIterator it=keys.constBegin(); it!=keys.constEnd(); ++it)
	QList<QString> categories = FEmoji->categories();
	qDebug() << "categories=" << categories;
	QMap<uint, EmojiData> emojiData = FEmoji->emojiData(categories.at(0));
	qDebug() << "Got map!";
	EmojiData data = emojiData.constBegin().value();
	qDebug() << "Got data!";
	QIcon icon = FEmoji->getIcon(data.unicode, QSize(16, 16));
	qDebug() << "Got icon!";
//				FStorage->insertAutoIcon(this, *it);
	menuAction()->setIcon(icon);
	qDebug() << "Set icon!";
	menuAction()->setToolTip(ASubStorage);
//				break;
	qDebug() << "Done!";
//	}
}

QSize SelectIconMenu::sizeHint() const
{
	return FLayout->sizeHint();
}

void SelectIconMenu::onAboutToShow()
{
	int index = Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).value().toInt();
	QString color = index?FEmoji->colorSuffixes()[index-1]:QString();

	FTabWidget = new QTabWidget(this);
	FLayout->addWidget(FTabWidget);

	QList<QString> categories = FEmoji->categories();
	SelectIconWidget *selectedWidget(NULL);
	for (QList<QString>::ConstIterator it = categories.constBegin(); it!=categories.constEnd(); ++it)
	{
		SelectIconWidget *widget = new SelectIconWidget(*it, color, FEmoji, this);
		if (!selectedWidget)
			selectedWidget = widget;
		FTabWidget->addTab(widget, *it);

		connect(this,SIGNAL(aboutToHide()),widget,SLOT(deleteLater()));
		connect(widget,SIGNAL(iconSelected(const QString &)),SIGNAL(iconSelected(const QString &)));
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

	if (selectedWidget->hasColored())
	{
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
	}
	else
		FMenu->setEnabled(false);

	QToolButton *button = changer.insertAction(FMenu->menuAction(), TBG_MWSIM_SKINCOLOR);
	button->setPopupMode(QToolButton::InstantPopup);
	FMenu->setTitle(tr("Skin color"));

	QStringList recent = FEmoji->recentIcons("emojione");
	for (QStringList::ConstIterator it=recent.constBegin(); it!=recent.constEnd(); ++it)
	{
		QLabel *label = selectedWidget->getIconLabel(*it, color);
		changer.insertWidget(label, TBG_MWSIM_RECENT);
	}
	connect(this,SIGNAL(aboutToHide()),toolBar,SLOT(deleteLater()));
}

void SelectIconMenu::onSkinColorSelected()
{
	Options::node(OPV_MESSAGES_EMOJI_SKINCOLOR).setValue(qobject_cast<Action *>(sender())->data(ADR_COLOR));
}

void SelectIconMenu::onOptionsChanged(const OptionsNode &ANode)
{	
	if (ANode.path() == OPV_MESSAGES_EMOJI_SKINCOLOR && FMenu)
	{
		int index = ANode.value().toInt();
		QString icon = index?FEmoji->colorSuffixes()[index-1]:QString("default");
			FMenu->setIcon(RSR_STORAGE_EMOJI, icon);
		SelectIconWidget *widget = qobject_cast<SelectIconWidget *>(qobject_cast<QTabWidget *>(FLayout->itemAt(0)->widget())->widget(0));
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

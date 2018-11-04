#ifndef SELECTICONMENU_H
#define SELECTICONMENU_H

#include <QVBoxLayout>
#include <QPointer>
#include <QTabWidget>
#include <utils/menu.h>
#include <utils/options.h>
#include <utils/toolbarchanger.h>

#include "selecticonwidget.h"

class SelectIconMenu :
	public Menu
{
	Q_OBJECT
public:
	SelectIconMenu(const QString &AIconSet, IEmoji *AEmoji, QWidget *AParent = NULL);
	~SelectIconMenu();
	QWidget *instance() { return this; }
	QString iconSet() const;
	void setIconSet(const QString &AIconSet);

signals:
	void iconSelected(const QString &AIconKey);

public:
	virtual QSize sizeHint() const;

protected slots:
	void onAboutToShow();
	void onSkinColorSelected();
	void onOptionsChanged(const OptionsNode &ANode);
	void onRecentIconTriggered();
	void onHasColoredChanged(bool AHasColored);
	void onCategorySwitched(int ACategory);

protected:
	void updateRecentActions(int AColor);

private:
	IEmoji *FEmoji;
	QVBoxLayout *FLayout;
	QTabWidget	*FTabWidget;
	QPointer<Menu> FMenu;
	ToolBarChanger *FToolBarChanger;
	QIcon		FEmptyIcon;
};

#endif // SELECTICONMENU_H

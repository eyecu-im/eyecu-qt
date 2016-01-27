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
	SelectIconMenu(const QString &AIconset, IEmoji *AEmoji, QWidget *AParent = NULL);
	~SelectIconMenu();
	QWidget *instance() { return this; }
	QString iconset() const;
	void setIconset(const QString &ASubStorage);
signals:
	void iconSelected(const QString &AIconKey, const QString &AIconText);
public:
	virtual QSize sizeHint() const;
protected slots:
	void onAboutToShow();
	void onSkinColorSelected();
	void onOptionsChanged(const OptionsNode &ANode);
	void onRecentIconTriggered();
	void onHasColoredChanged(bool AHasColored);
protected:
	static QString typeUcs4(const QString &AText);
private:
	IEmoji *FEmoji;
	QVBoxLayout *FLayout;
	QTabWidget	*FTabWidget;
	QPointer<Menu> FMenu;
	QToolBar	*FToolBar;
	ToolBarChanger *FToolBarChanger;
	QIcon		FEmptyIcon;
};

#endif // SELECTICONMENU_H

#ifndef SELECTICONMENU_H
#define SELECTICONMENU_H

#include <QVBoxLayout>
#include <QPointer>
#include <QTabWidget>
#include <utils/menu.h>
#include <utils/options.h>

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
	void iconSelected(const QString &AIconKey);
public:
	virtual QSize sizeHint() const;
protected slots:
	void onAboutToShow();
	void onSkinColorSelected();
	void onOptionsChanged(const OptionsNode &ANode);
	void onRecentIconTriggered();
	void onHasColoredChanged(bool AHasColored);
private:
	IEmoji *FEmoji;
	QVBoxLayout *FLayout;
	QTabWidget	*FTabWidget;
//	IconStorage *FStorage;
	QPointer<Menu> FMenu;
};

#endif // SELECTICONMENU_H

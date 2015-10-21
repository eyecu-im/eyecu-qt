#ifndef SELECTICONMENU_H
#define SELECTICONMENU_H

#include <QVBoxLayout>
#include <QPointer>
#include "selecticonwidget.h"
#include <utils/menu.h>
#include <utils/options.h>

class SelectIconMenu :
	public Menu
{
	Q_OBJECT
public:
	SelectIconMenu(const QString &AIconset, Emoji *AEmoji, QWidget *AParent = NULL);
	~SelectIconMenu();
	QWidget *instance() { return this; }
	QString iconset() const;
	void setIconset(const QString &ASubStorage);
signals:
	void iconSelected(const QString &ASubStorage, const QString &AIconKey);
public:
	virtual QSize sizeHint() const;
protected slots:
	void onAboutToShow();
	void onSkinColorSelected();
	void onOptionsChanged(const OptionsNode &ANode);
private:
	Emoji *FEmoji;
	QVBoxLayout *FLayout;
	IconStorage *FStorage;
	QPointer<Menu> FMenu;
};

#endif // SELECTICONMENU_H

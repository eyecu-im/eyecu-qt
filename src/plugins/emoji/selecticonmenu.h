#ifndef SELECTICONMENU_H
#define SELECTICONMENU_H

#include <QVBoxLayout>
#include <QPointer>
#include <QTabWidget>
#include <utils/menu.h>
#include <utils/options.h>
#include <utils/toolbarchanger.h>
#include <interfaces/iemoji.h>

class SelectIconMenu :
	public Menu
{
	Q_OBJECT
public:
	SelectIconMenu(const QString &AIconSet, IEmoji *AEmoji, QWidget *AParent = nullptr);
	~SelectIconMenu();
	QString iconSet() const;
	void setIconSet(const QString &AIconSet);

signals:
	void iconSelected(const QString &AIconKey);

public:
	virtual QSize sizeHint() const;

protected slots:
	void onAboutToShow();
	void onSkinColorSelected();
	void onGenderSelected();
	void onOptionsChanged(const OptionsNode &ANode);
	void onRecentIconTriggered();
	void onHasColoredChanged(bool AHasColored);
	void onHasGenderedChanged(bool AHasGendered);
	void onCategorySwitched(int ACategory);
	void onIconSelected(const QString &AIconKey);

protected:
	void updateRecentActions();

private:
	IEmoji *FEmoji;
	QVBoxLayout *FLayout;
	QTabWidget	*FTabWidget;
	QPointer<Menu> FSkinColor;
	QPointer<Menu> FGender;
	ToolBarChanger *FToolBarChanger;
	QIcon		FEmptyIcon;
};

#endif // SELECTICONMENU_H

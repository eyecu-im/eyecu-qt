#include "menubarwidget.h"

MenuBarWidget::MenuBarWidget(IMessageWindow *AWindow, QWidget *AParent) : QMenuBar(AParent)
{
	FWindow = AWindow;
	FMenuBarChanger = new MenuBarChanger(this);

	// On Ubuntu 11.10 empty menubar cause segmentation fault
	addAction(QString())->setVisible(false);
}

MenuBarWidget::~MenuBarWidget()
{

}

bool MenuBarWidget::isVisibleOnWindow() const
{
	return isVisibleTo(FWindow->instance());
}

IMessageWindow *MenuBarWidget::messageWindow() const
{
	return FWindow;
}

MenuBarChanger * MenuBarWidget::menuBarChanger() const
{
	return FMenuBarChanger;
}

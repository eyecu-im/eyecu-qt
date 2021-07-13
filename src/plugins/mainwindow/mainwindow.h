#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSplitter>
#include <interfaces/imainwindow.h>
#include "maintabwidget.h"
#include "maincentralwidget.h"

class MainWindow :
	public QMainWindow,
	public IMainWindow
{
	Q_OBJECT;
	Q_INTERFACES(IMainWindow);
public:
	MainWindow(QWidget *AParent = NULL, Qt::WindowFlags AFlags = 0);
	~MainWindow();
	//IMainWindow
	virtual QMainWindow *instance() { return this; }
	virtual bool isActive() const;
	virtual void showWindow(bool AMinimized = false);
	virtual void closeWindow();
	// Menu Management
	virtual Menu *mainMenu() const;
	virtual MenuBarChanger *mainMenuBar() const;
	// Widgets Management
	virtual BoxWidget *mainLeftWidget() const;
	virtual IMainTabWidget *mainTabWidget() const;
	virtual bool isCentralWidgetVisible() const;
	virtual IMainCentralWidget *mainCentralWidget() const;
	// Tool Bars Management
	virtual ToolBarChanger *topToolBarChanger() const;
	virtual ToolBarChanger *bottomToolBarChanger() const;
	virtual QList<ToolBarChanger *> toolBarChangers() const;
	virtual int toolBarChangerOrder(ToolBarChanger *AChanger) const;
	virtual ToolBarChanger *toolBarChangerByOrder(int AOrderId) const;
	virtual void insertToolBarChanger(int AOrderId, ToolBarChanger *AChanger);
	virtual void removeToolBarChanger(ToolBarChanger *AChanger);
signals:
	void toolBarChangerInserted(int AOrderId, ToolBarChanger *AChanger);
	void toolBarChangerRemoved(ToolBarChanger *AChanger);
	void centralWidgetVisibleChanged(bool AVisible);
public:
	void saveWindowGeometryAndState();
	void loadWindowGeometryAndState();
protected:
	void updateWindow();
	QMenu *createPopupMenu();
	void correctWindowPosition();
	void restoreAcceptDrops(QWidget *AParent);
	void setCentralWidgetVisible(bool AVisible);
protected:
	void closeEvent(QCloseEvent *AEvent);
	void showEvent(QShowEvent *AEvent);
	bool eventFilter(QObject *AObject, QEvent *AEvent);
protected slots:
	void onUpdateCentralWidgetVisible();
	void onCurrentCentralPageChanged();
	void onCentralPageAddedOrRemoved(IMainCentralPage *APage);
	void onSplitterMoved(int APos, int AIndex);
private:
	IMainTabWidget *FTabWidget;
	IMainCentralWidget *FCentralWidget;
private:
	Menu *FMainMenu;
	QSplitter *FSplitter;
	BoxWidget *FLeftWidget;
	MenuBarChanger *FMainMenuBar;
private:
	bool FAligned;
	bool FCentralVisible;
	int FLeftWidgetWidth;
	int FSplitterHandleWidth;
	QMap<int, ToolBarChanger *> FToolBarOrders;
};

#endif // MAINWINDOW_H

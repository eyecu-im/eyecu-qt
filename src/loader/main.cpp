#include <QLibrary>
#include <QApplication>
#include <QPalette>
#include "pluginmanager.h"
// *** <<< eyeCU <<< ***
#include "splash.h"
// *** >>> eyeCU >>> ***
int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_DontShowIconsInMenus,false);
#if (QT_VERSION >= QT_VERSION_CHECK(5,6,0))
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling,true);
#endif

	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);
	app.addLibraryPath(app.applicationDirPath());
	app.setApplicationName("eyeCU");
// *** <<< eyeCU <<< ***
	Splash splash;
	splash.showMessage("Loading: Utils", Qt::AlignBottom, QPalette().color(QPalette::Text));
// *** >>> eyeCU >>> ***
	QLibrary utils(app.applicationDirPath()+"/utils", &app);
	utils.load();

	PluginManager pm(&app);
// *** <<< eyeCU <<< ***
	splash.connect(&pm, SIGNAL(closeSplash(QWidget*)), SLOT(finishSplash(QWidget*)));
	splash.connect(&pm, SIGNAL(splashMessage(QString)), SLOT(displayMessage(QString)));
// *** >>> eyeCU >>> ***
	pm.restart();

	return app.exec();
}


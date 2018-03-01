#include <QLibrary>
#include <QApplication>
#include <QPalette>
#include "pluginmanager.h"
// *** <<< eyeCU <<< ***
#include "splash.h"
// *** >>> eyeCU >>> ***
int main(int argc, char *argv[])
{
#ifdef Q_OS_MACX
	if (QSysInfo::MacintoshVersion == QSysInfo::MV_YOSEMITE )
	{
		// https://bugreports.qt-project.org/browse/QTBUG-40833
		QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Helvetica Neue");
	}
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

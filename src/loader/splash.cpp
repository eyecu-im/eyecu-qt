#include "splash.h"

#include <QApplication>
#include <QSplashScreen>
#include <QSvgRenderer>
#include <QImage>
#include <QPainter>
#include <QDir>

#include <definitions/resources.h>
#include <definitions/version.h>
#include <utils/filestorage.h>

#include "gitinfo.h"

#define SPLASH_WIDTH 386
#define SPLASH_HEIGHT 256

Splash::Splash():
	QSplashScreen()
{
	QDir resourcesDir(QDir::isAbsolutePath(RESOURCES_DIR) ? RESOURCES_DIR : QApplication::instance()->applicationDirPath()+"/"+RESOURCES_DIR);
	if (resourcesDir.cd(RSR_STORAGE_MENUICONS) &&
		resourcesDir.cd(FILE_STORAGE_SHARED_DIR))
	{
		QSvgRenderer renderer(resourcesDir.absoluteFilePath("eyecu.svg"));
		if (renderer.isValid())
		{
			QImage image(QSize(SPLASH_WIDTH, SPLASH_HEIGHT), QImage::Format_RGB32);
			if (!image.isNull())
			{
				image.fill(palette().color(QPalette::Window));
				QPainter painter(&image);
				renderer.render(&painter, QRect(0, -80, SPLASH_WIDTH, SPLASH_WIDTH));
				painter.drawText(0, SPLASH_HEIGHT-40, SPLASH_WIDTH, 20, Qt::AlignCenter, QString("Version: %1.%2 %3")
								 .arg(CLIENT_VERSION)
								 .arg(QDateTime::fromTime_t(QString(GIT_DATE).toInt()).date().toString("yyyyMMdd"))
								 .arg(CLIENT_VERSION_SUFFIX));
				QPixmap pixmap = QPixmap::fromImage(image);
				if (!pixmap.isNull())
				{
					setPixmap(pixmap);
					show();
				}
			}
		}
	}
}

void Splash::displayMessage(const QString &AMessage)
{
	showMessage(AMessage, Qt::AlignBottom, palette().color(QPalette::Text));
}

void Splash::finishSplash(QWidget *AMainWindow)
{
	if (AMainWindow)
		finish(AMainWindow);
	else
		close();
}

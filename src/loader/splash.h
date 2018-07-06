#ifndef SPLASH_H
#define SPLASH_H

#include <QSplashScreen>

class Splash : public QSplashScreen
{
	Q_OBJECT

public:
	Splash();

public slots:
	void displayMessage(const QString &AMessage);
	void finishSplash(QWidget *AMainWindow);
};

#endif // SPLASH_H

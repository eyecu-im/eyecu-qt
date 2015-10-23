#ifndef SELECTICONWIDGET_H
#define SELECTICONWIDGET_H

#include <QMap>
#include <QLabel>
#include <QEvent>
#include <QGridLayout>
#include <QMainWindow>
#include <interfaces/iemoji.h>
#include <utils/iconstorage.h>

class SelectIconWidget :
	public QWidget
{
	Q_OBJECT
public:
	SelectIconWidget(IconStorage *AStorage, const QString &AColor, IEmoji *AEmoji, QWidget *AParent = NULL);
	~SelectIconWidget();
	void updateLabels(const QString &AColor);
	QLabel *getIconLabel(const QString &AKey, const QString &AColor);
	bool hasColored() const {return FHasColored;}
signals:
	void iconSelected(const QString &ASubStorage, const QString &AIconKey);
protected:
	void createLabels(const QString &AColor);
protected:
	virtual bool eventFilter(QObject *AWatched, QEvent *AEvent);
private:
	IEmoji *FEmoji;
	QLabel *FPressed;
	QGridLayout *FLayout;
	IconStorage *FStorage;
	QMap<QLabel *, QString> FKeyByLabel;
	bool FHasColored;
};

#endif // SELECTICONWIDGET_H

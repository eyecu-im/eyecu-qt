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
	SelectIconWidget(IEmoji::Category ACategory, uint AColumns, uint ARows, IEmoji *AEmoji, QWidget *AParent = NULL);
	~SelectIconWidget();
	void updateLabels(int AColor);
	bool hasColored() const {return FHasColored;}
	void invalidate() {FNotReady = true;}
signals:
	void iconSelected(const QString &AIconKey);
	void hasColoredChanged(bool AHasColored);
protected:
	void createLabels();
protected:
// QObject interface
	virtual bool eventFilter(QObject *AWatched, QEvent *AEvent);
// QWidget interface
	virtual void showEvent(QShowEvent *AShowEvent);
private:
	IEmoji *FEmoji;
	QLabel *FPressed;
	QGridLayout *FLayout;
	const QMap<uint, EmojiData> FEmojiMap;
	QMap<QLabel *, QString> FKeyByLabel;
	int	FColor;
	bool FHasColored;
	bool FNotReady;
	uint FColumns;
	uint FRows;
};

#endif // SELECTICONWIDGET_H

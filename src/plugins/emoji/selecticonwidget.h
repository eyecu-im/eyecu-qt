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
	void updateLabels();
	bool hasColored() const {return FHasColored;}
//	void invalidate() {FNotReady = true;}
signals:
	void iconSelected(const QString &AIconKey);
	void hasColoredChanged(bool AHasColored);
	void hasGenderedChanged(bool AHasGendered);
protected:
	void createLabels();
protected:
// QWidget interface
	virtual void showEvent(QShowEvent *AShowEvent);
protected slots:
	void onActionTriggered(QAction *AAction);

private:
	IEmoji *FEmoji;
	QLabel *FPressed;
	QGridLayout *FLayout;
	const QMap<uint, IEmojiData*> FEmojiMap;
	int	FColor;
	bool FHasColored;
	int	FGender;
	bool FHasGendered;
	bool FNotReady;
	uint FColumns;
	uint FRows;
};

#endif // SELECTICONWIDGET_H

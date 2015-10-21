#ifndef EMOJIOPTIONS_H
#define EMOJIOPTIONS_H

#include <QWidget>
#include <interfaces/iemoji.h>
#include <interfaces/ioptionsmanager.h>
#include "ui_emojioptions.h"

class EmojiOptions :
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
public:
	EmojiOptions(IEmoji *AEmoji, QWidget *AParent);
	~EmojiOptions();
	virtual QWidget* instance() { return this; }
public slots:
	virtual void apply();
	virtual void reset();
signals:
	void modified();
	void childApply();
	void childReset();
protected slots:
	void onUpButtonClicked();
	void onDownButtonClicked();
	void onMakeSelectableButtonToggled(bool ASelectable);
	void onCurrentItemChanged(QListWidgetItem *ACurrent, QListWidgetItem *APrevious);
	void onItemChanged(QListWidgetItem *AItem);
private:
	Ui::EmojiOptionsClass ui;
private:
	IEmoticons *FEmoji;
};

#endif // EMOJIOPTIONS_H

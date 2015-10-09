#ifndef EMOTICONSOPTIONS_H
#define EMOTICONSOPTIONS_H

#include <QWidget>
#include <interfaces/iemoticons.h>
#include <interfaces/ioptionsmanager.h>
#include "ui_emojioptions.h"

class EmojiOptions :
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT;
	Q_INTERFACES(IOptionsDialogWidget);
public:
	EmojiOptions(IEmoticons *AEmoticons, QWidget *AParent);
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
private:
	Ui::EmojiOptionsClass ui;
private:
	IEmoticons *FEmoticons;
};

#endif // EMOTICONSOPTIONS_H

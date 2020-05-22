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
	void onListBoxCurrentIndexChanged(int AIndex);

private:
	Ui::EmojiOptionsClass ui;
private:
	IEmoji *FEmoji;
};

#endif // EMOJIOPTIONS_H

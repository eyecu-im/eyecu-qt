#ifndef PAYLOADTYPEOPTIONS_H
#define PAYLOADTYPEOPTIONS_H

#include <QWidget>
#include <QAVCodec>
#include <QAVOutputFormat>

#include <interfaces/ioptionsmanager.h>

#include "ui_codecoptions.h"

namespace Ui {
class CodecOptions;
}

class CodecOptions :
    public QWidget,
	public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	explicit CodecOptions(QWidget *parent = nullptr);
	~CodecOptions();
    virtual QWidget* instance() { return this; }
	Ui::CodecOptions *ui;

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected:
	void changeEvent(QEvent *e);

protected slots:
	void onAvailableCodecCurrentRowChanged(int ARow);
	void onUsedCodecCurrentRowChanged(int ARow);

	void onUsedCodecPriorityChange();
	void onCodecUse();
	void onCodecUnuse();

private:
	QAVOutputFormat FRtp;
};

#endif // PAYLOADTYPEOPTIONS_H

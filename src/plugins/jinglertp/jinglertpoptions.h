#ifndef JINGLERTPOPTIONS_H
#define JINGLERTPOPTIONS_H

#include <QWidget>
#include <QAVCodec>
#include <QAudioDeviceInfo>

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

#include "ui_jinglertpoptions.h"

namespace Ui {
class JingleRtpOptions;
}

class JingleRtpOptions :
    public QWidget,
	public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    explicit JingleRtpOptions(QWidget *parent = 0);
    ~JingleRtpOptions();
    virtual QWidget* instance() { return this; }
    Ui::JingleRtpOptions *ui;

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected:
	void changeEvent(QEvent *e);
	QList<QAVP> availablePayloadTypes() const;

protected slots:
    void modify(int);

	void onAvailablePayloadTypeSelectionChanged();
	void onUsedPayloadTypeSelectionChanged();

	void onUsedPayloadTypePriorityUp();
	void onUsedPayloadTypePriorityDown();
	void onPayloadTypeUse();
	void onPayloadTypeUnuse();

	void onPayloadTypeAdd();
	void onPayloadTypeEdit();
	void onPayloadTypeRemove();

private:
	QList<QAVP> FAvailableStaticPayloadTypes;
};

#endif // JINGLERTPOPTIONS_H

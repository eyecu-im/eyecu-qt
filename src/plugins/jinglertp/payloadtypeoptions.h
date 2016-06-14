#ifndef PAYLOADTYPEOPTIONS_H
#define PAYLOADTYPEOPTIONS_H

#include <QWidget>
#include <QAVCodec>

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

#include "ui_payloadtypeoptions.h"

namespace Ui {
class PayloadTypeOptions;
}

class PayloadTypeOptions :
    public QWidget,
	public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	explicit PayloadTypeOptions(QWidget *parent = 0);
	~PayloadTypeOptions();
    virtual QWidget* instance() { return this; }
	Ui::PayloadTypeOptions *ui;

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

#endif // PAYLOADTYPEOPTIONS_H

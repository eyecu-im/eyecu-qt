#ifndef SCHEDULEROPTIONS_H
#define SCHEDULEROPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>

namespace Ui {
class SchedulerOptions;
}

class SchedulerOptions : public QWidget, public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
public:
	explicit SchedulerOptions(QWidget *parent = 0);
	~SchedulerOptions();

	// IOptionsDialogWidget interface
	virtual QWidget *instance() {return this;}

public slots:
	virtual void apply();
	virtual void reset();

signals:
	virtual void modified();
	virtual void childApply();
	virtual void childReset();

private:
	Ui::SchedulerOptions *ui;
};

#endif // SCHEDULEROPTIONS_H

#ifndef SCHEDULEROPTIONS_H
#define SCHEDULEROPTIONS_H

#include <QTreeWidgetItem>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/irostermanager.h>
#include <interfaces/ipresencemanager.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>

namespace Ui {
class SchedulerOptions;
}

class SchedulerItem;

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

protected:
	QTreeWidgetItem *addItem(const SchedulerItem &AItem);

protected slots:
	void onCurrentItemChanged(QTreeWidgetItem *ACurrent, QTreeWidgetItem *APrevious);
	void onItemAdd();
	void onItemEdit();
	void onItemDelete();

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::SchedulerOptions *ui;
	IAccountManager	*FAccountManager;
	IRosterManager	*FRosterManager;
};

#endif // SCHEDULEROPTIONS_H

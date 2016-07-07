#ifndef SCHEDULERITEMDIALOG_H
#define SCHEDULERITEMDIALOG_H

#include <QDialog>
#include <interfaces/iaccountmanager.h>
#include <interfaces/irostermanager.h>
#include <interfaces/ipresencemanager.h>
#include "scheduler.h"

namespace Ui {
class SchedulerItemDialog;
}

class SchedulerItemDialog : public QDialog
{
	Q_OBJECT

public:
	SchedulerItemDialog(const SchedulerItem &AItem, IAccountManager *AAccountManager, QWidget *AParent = 0);
	~SchedulerItemDialog();
	SchedulerItem getItem() const;

protected slots:
	void onTimeoutSpinboxValueChanged(int AValue);
	void onAccountSelected(int AIndex);
	void validate();

private:
	Ui::SchedulerItemDialog *ui;
	IAccountManager *FAccountManager;
	IRosterManager	*FRosterManager;
	IPresenceManager  *FPresenceManager;
	SchedulerItem	FItem;
};

#endif // SCHEDULERITEMDIALOG_H

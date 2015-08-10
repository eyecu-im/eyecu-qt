#ifndef ATTENTIONDIALOG_H
#define ATTENTIONDIALOG_H

#include <interfaces/inotifications.h>
#include "ui_attentiondialog.h"

namespace Ui {
    class AttentionDialog;
}

class AttentionDialog : public QDialog
{
    Q_OBJECT

public:
	AttentionDialog(int ANotifyId, const INotification &ANotification, INotifications *ANotifications=0, QWidget *parent=0);
    ~AttentionDialog();
    Ui::AttentionDialog *ui;

private:
    const int notifyId;
    INotifications *FNotifications;

protected slots:
    void onOkClick();
};

#endif // ATTENTIONDIALOG_H

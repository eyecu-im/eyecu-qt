#ifndef INVITEUSERDIALOG_H
#define INVITEUSERDIALOG_H

#include <QDialog>

namespace Ui {
class ConfirmationDialog;
}

class ConfirmationDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConfirmationDialog(const QString &ATitle, const QString &ALabel, QString &AReason, bool &AStore, bool &AAsk, QWidget *parent = nullptr);
	~ConfirmationDialog();

private:
	Ui::ConfirmationDialog *ui;
	QString &FReason;
	bool &FStore;
	bool &FAsk;

	// QDialog interface
public slots:
	virtual void accept() override;
};

#endif // INVITEUSERDIALOG_H

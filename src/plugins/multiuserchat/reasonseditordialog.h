#ifndef REASONSEDITORDIALOG_H
#define REASONSEDITORDIALOG_H

#include <QDialog>
#include <utils/options.h>

namespace Ui {
class ReasonsEditorDialog;
}

class ReasonsEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ReasonsEditorDialog(const QString &ATitle, const QString &ALabel, QString &AReason, bool &ABad, bool &AStore, bool &AAsk, QWidget *parent = nullptr);
	~ReasonsEditorDialog();

private:
	Ui::ReasonsEditorDialog *ui;
	QString &FReason;
	bool &FBad;
	bool &FStore;
	bool &FAsk;
	QString FList;

	// QDialog interface
public slots:
	virtual void accept() override;

private slots:
	void onItemAdd();
	void onItemDelete();
	void save();
	void currentChanged(const QString & FReason);
};

#endif // REASONSEDITORDIALOG_H

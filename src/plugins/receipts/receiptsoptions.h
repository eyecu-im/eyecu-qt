#ifndef RECEIPTSOPTIONS_H
#define RECEIPTSOPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class ReceiptsOptions;
}

class ReceiptsOptions:
		public QWidget,
		public IOptionsDialogWidget
{
	Q_OBJECT

public:
	ReceiptsOptions(QWidget *AParent = nullptr);
	~ReceiptsOptions();

	// IOptionsDialogWidget interface
	QWidget *instance();

public slots:
	void apply();
	void reset();

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::ReceiptsOptions *ui;
};

#endif // RECEIPTSOPTIONS_H

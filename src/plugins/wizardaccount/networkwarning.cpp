#include "networkwarning.h"
#include "ui_networkwarning.h"

NetworkWarning::NetworkWarning(const QString &ATitleText, const QString &AMessageText, QWidget *AParent) :
	QDialog(AParent),
	ui(new Ui::NetworkWarning)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(ATitleText);
	ui->lblIcon->setPixmap(QApplication::style()->standardPixmap(QStyle::SP_MessageBoxWarning));
	ui->lblMessage->setText(AMessageText);
	setFixedSize(sizeHint());
}

NetworkWarning::~NetworkWarning()
{
	delete ui;
}

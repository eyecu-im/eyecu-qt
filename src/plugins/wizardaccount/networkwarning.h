#ifndef NETWORKWARNING_H
#define NETWORKWARNING_H

#include <QDialog>

namespace Ui {
class NetworkWarning;
}

class NetworkWarning : public QDialog
{
	Q_OBJECT

public:
	NetworkWarning(const QString &ATitleText, const QString &AMessageText, QWidget *AParent = 0);
	~NetworkWarning();

private:
	Ui::NetworkWarning *ui;
};

#endif // NETWORKWARNING_H

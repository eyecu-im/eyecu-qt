#ifndef ADDSERVER_H
#define ADDSERVER_H

#include <QDialog>

namespace Ui {
class AddServer;
}

class AddServer : public QDialog
{
	Q_OBJECT

public:
	AddServer(bool turn, QWidget *parent = nullptr);
	~AddServer();

	QString host() const;
	quint16 port() const;
	QString username() const;
	QString password() const;

protected slots:
	void onShowPassword(bool show);

private:
	Ui::AddServer *ui;
};

#endif // ADDSERVER_H

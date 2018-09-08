#include "addserver.h"
#include "ui_addserver.h"

AddServer::AddServer(bool turn, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AddServer)
{
	ui->setupUi(this);

	setWindowTitle(tr("Add %1 server")
				   .arg(QString::fromLatin1(turn?"TURN":"STUN")));

	ui->grpbCredentials->setVisible(turn);

#if QT_VERSION >= 0x040700
	ui->ledHost->setPlaceholderText(tr("Server host name or IP address"));
	ui->ledUsername->setPlaceholderText(tr("TURN server username"));
	ui->ledPassword->setPlaceholderText(tr("TURN server password"));
#endif
}

AddServer::~AddServer()
{
	delete ui;
}

QString AddServer::host() const
{
	return ui->ledHost->text();
}

quint16 AddServer::port() const
{
	return quint16(ui->spbPort->value());
}

QString AddServer::username() const
{
	return ui->ledUsername->text();
}

QString AddServer::password() const
{
	return ui->ledPassword->text();
}

void AddServer::onShowPassword(bool show)
{
	ui->ledPassword->setEchoMode(show?QLineEdit::Normal:QLineEdit::Password);
}

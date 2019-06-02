#ifndef OTRAUTHDIALOG_H
#define OTRAUTHDIALOG_H

#include "otr.h"

#include <QMessageBox>

namespace Ui {
class OtrAuthDialog;
}

class OtrAuthDialog : public QDialog
{
	Q_OBJECT
public:
	OtrAuthDialog(Otr* AOtr, const Jid& AStreamJid, const Jid& AContactJid,
				  const QString& AQuestion, bool ASender,
				  QWidget* AParent = nullptr);
	~OtrAuthDialog();

	void reset();
	bool finished();
	void updateSMP(int AProgress);
	void notify(const QMessageBox::Icon AIcon, const QString& AMessage);

public slots:
	void reject();
	void accept();

protected slots:
	void checkRequirements();

private:
	enum AuthState {AUTH_READY, AUTH_IN_PROGRESS, AUTH_FINISHED};
	enum Method {METHOD_QUESTION, METHOD_SHARED_SECRET, METHOD_FINGERPRINT};

	Otr*			FOtr;
	Jid				FAccount;
	Jid				FContact;
	QString			FContactName;
	bool			FIsSender;
	AuthState		FState;
	OtrFingerprint	FFingerprint;

	Ui::OtrAuthDialog *ui;
};

#endif // OTRAUTHDIALOG_H

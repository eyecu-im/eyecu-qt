#include "otrauthdialog.h"
#include "ui_otrauthdialog.h"
#include <QPushButton>

OtrAuthDialog::OtrAuthDialog(Otr *AOtr, const Jid &AStreamJid,
							 const Jid &AContactJid,
							 const QString& AQuestion,
							 bool ASender, QWidget* AParent):
	QDialog(AParent),	
	FOtr(AOtr),
	FAccount(AStreamJid),
	FContact(AContactJid),
	FIsSender(ASender),
	ui(new Ui::OtrAuthDialog)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("&Authenticate"));
	setAttribute(Qt::WA_DeleteOnClose);

	FContactName = FOtr->humanContact(FAccount, FContact);

	QString qaExplanation;
	QString ssExplanation;

	if (FIsSender)
	{
		setWindowTitle(tr("Authenticate %1").arg(FContactName));
		qaExplanation = tr("To authenticate via question and answer, "
						   "ask a question whose answer is only known "
						   "to you and %1.").arg(FContactName);
		ssExplanation = tr("To authenticate via shared secret, "
						   "enter a secret only known "
						   "to you and %1.").arg(FContactName);
	}
	else
	{
		setWindowTitle(tr("Authenticate to %1").arg(FContactName));
		qaExplanation = tr("%1 wants to authenticate you. To authenticate, "
						   "answer the question asked below.")
						   .arg(FContactName);
		ssExplanation = tr("%1 wants to authenticate you. To authenticate, "
						   "enter your shared secret below.")
						   .arg(FContactName);
	}

	ui->lblExplanationQA->setText(qaExplanation);
	ui->lblExplanationSS->setText(ssExplanation);
	if (FIsSender)
	{
		ui->lblAuthenticated->setVisible(FOtr->isVerified(FAccount, FContact));

		QString ownFpr = FOtr->getPrivateKeys().value(FAccount,
							tr("No private key for account \"%1\"")
							  .arg(FOtr->humanAccount(FAccount)));

		FFingerprint = FOtr->getActiveFingerprint(FAccount, FContact);

		ui->lblFingerprintLocal->setText(ownFpr);
		ui->lblContactsFingerprint->setText(tr("%1's fingerprint:").arg(FContactName));
		ui->lblFingerprintRemote->setText(FFingerprint.FFingerprintHuman);
		QFont font;
		font.setStyleHint(QFont::Monospace);
		ui->lblFingerprintLocal->setFont(font);
		ui->lblFingerprintRemote->setFont(font);
	}

	Method method = METHOD_QUESTION;

	if (FIsSender)
		ui->ledQuestion->setFocus();
	else
	{
		if (AQuestion.isEmpty())
		{
			method = METHOD_SHARED_SECRET;
			ui->ledSharedSecret->setFocus();
		}
		else
		{
			ui->ledQuestion->setText(AQuestion);
			ui->ledAnswer->setFocus();
		}
	}

	ui->cmbMethod->setCurrentIndex(method);
	ui->stackedWidget->setCurrentIndex(method);

	reset();
}

OtrAuthDialog::~OtrAuthDialog()
{
	delete ui;
}

void OtrAuthDialog::reject()
{
	if (FState == AUTH_IN_PROGRESS)
		FOtr->abortSMP(FAccount, FContact);
	QDialog::reject();
}

void OtrAuthDialog::reset()
{
	FState = FIsSender? AUTH_READY : AUTH_IN_PROGRESS;

	ui->cmbMethod->setEnabled(FIsSender);
	ui->ledQuestion->setEnabled(FIsSender);
	ui->ledAnswer->setEnabled(true);
	ui->ledSharedSecret->setEnabled(true);
	ui->prbQA->setEnabled(false);
	ui->prbSS->setEnabled(false);

	checkRequirements();
}

bool OtrAuthDialog::finished()
{
	return FState == AUTH_FINISHED;
}

void OtrAuthDialog::checkRequirements()
{
	int method = ui->cmbMethod->currentIndex();
	ui->buttonBox->button(QDialogButtonBox::Ok)
			->setEnabled((method == METHOD_QUESTION &&
						  !ui->ledQuestion->text().isEmpty() &&
						  !ui->ledAnswer->text().isEmpty()) ||
						 (method == METHOD_SHARED_SECRET &&
						  !ui->ledSharedSecret->text().isEmpty()) ||
						 (method == METHOD_FINGERPRINT));
}

void OtrAuthDialog::accept()
{
	switch (ui->cmbMethod->currentIndex())
	{
		case METHOD_QUESTION:
			if (ui->ledQuestion->text().isEmpty() ||
				ui->ledAnswer->text().isEmpty())
				return;

			FState = AUTH_IN_PROGRESS;

			ui->cmbMethod->setEnabled(false);
			ui->ledQuestion->setEnabled(false);
			ui->ledAnswer->setEnabled(false);
			ui->prbQA->setEnabled(true);
			ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

			if (FIsSender)
				FOtr->startSMP(FAccount, FContact, ui->ledQuestion->text(), ui->ledAnswer->text());
			else
				FOtr->continueSMP(FAccount, FContact, ui->ledAnswer->text());
			updateSMP(33);
			break;

		case METHOD_SHARED_SECRET:
			if (ui->ledSharedSecret->text().isEmpty())
				return;

			FState = AUTH_IN_PROGRESS;

			ui->cmbMethod->setEnabled(false);
			ui->ledSharedSecret->setEnabled(false);
			ui->prbSS->setEnabled(true);
			ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

			if (FIsSender)
				FOtr->startSMP(FAccount, FContact, QString(), ui->ledSharedSecret->text());
			else
				FOtr->continueSMP(FAccount, FContact, ui->ledSharedSecret->text());
			updateSMP(33);
			break;

		case METHOD_FINGERPRINT:
			if (FFingerprint.FFingerprint)
			{
				QString msg(tr("Account: ") + FOtr->humanAccount(FAccount) + "\n" +
//FIXME: Show correct contact name here
							tr("User: ") + FContact.full() + "\n" +
							tr("Fingerprint: ") + FFingerprint.FFingerprintHuman + "\n\n" +
							tr("Have you verified that this is in fact the correct fingerprint?"));

				QMessageBox mb(QMessageBox::Information, tr("Psi OTR"),
							   msg, QMessageBox::Yes | QMessageBox::No, this,
							   Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

				FOtr->verifyFingerprint(FFingerprint, mb.exec() == QMessageBox::Yes);
				close();
			}
			break;
	}
}

void OtrAuthDialog::updateSMP(int AProgress)
{
	if (AProgress<0)
	{
		if (AProgress == -1)
			notify(QMessageBox::Warning,
				   tr("%1 has canceled the authentication process.")
					 .arg(FContactName));
		else
			notify(QMessageBox::Warning,
				   tr("An error occurred during the authentication process."));

		if (FIsSender)
			reset();
		else
			close();

		return;
	}

	int method = ui->cmbMethod->currentIndex();

	if (method == METHOD_SHARED_SECRET)
		ui->prbSS->setValue(AProgress);
	else if (method == METHOD_QUESTION)
		ui->prbQA->setValue(AProgress);

	if (AProgress == 100) {
		if (FIsSender || method == METHOD_SHARED_SECRET)
			FOtr->stateChange(FAccount, FContact,
							   IOtr::StateChangeTrust);

		if (FOtr->smpSucceeded(FAccount, FContact))
		{
			FState = AUTH_FINISHED;
			if (FOtr->isVerified(FAccount, FContact))
				notify(QMessageBox::Information,
					   tr("Authentication successful."));
			else
				notify(QMessageBox::Information,
					   tr("You have been successfully authenticated.\n\n"
						  "You should authenticate %1 as "
						  "well by asking your own question.")
						  .arg(FContactName));
			close();
		}
		else
		{
			FState = FIsSender? AUTH_READY : AUTH_FINISHED;
			notify(QMessageBox::Critical, tr("Authentication failed."));
			if (FIsSender)
				reset();
			else
				close();
		}
	}
}

void OtrAuthDialog::notify(const QMessageBox::Icon AIcon, const QString& AMessage)
{
	QMessageBox mb(AIcon, tr("Off-the-Record messaging"),
				   AMessage, QMessageBox::Ok, this,
				   Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	mb.exec();
}

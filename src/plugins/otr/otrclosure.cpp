/*
 * psiotrclosure.cpp
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *               2011-2012  Florian Fieber
 *                    2014  Boris Pek (tehnick-8@mail.ru)
 *
 * This program was originally written as part of a diplom thesis
 * advised by Prof. Dr. Ruediger Weis (PST Labor)
 * at the Technical University of Applied Sciences Berlin.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDebug>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>

#include "otrclosure.h"

AuthenticationDialog::AuthenticationDialog(Otr *AOtr,
										   const QString& AAccount,
										   const QString& AContact,
										   const QString& AQuestion,
										   bool ASender, QWidget* AParent):
	QDialog(AParent),
	FOtr(AOtr),
	FMethod(METHOD_QUESTION),
	FAccount(AAccount),
	FContact(AContact),
	FIsSender(ASender)
{
    setAttribute(Qt::WA_DeleteOnClose);

	FContactName = FOtr->humanContact(FAccount, FContact);

    QString qaExplanation;
    QString ssExplanation;
    QString fprExplanation;
	if (FIsSender)
    {
		setWindowTitle(tr("Authenticate %1").arg(FContactName));
        qaExplanation = tr("To authenticate via question and answer, "
                           "ask a question whose answer is only known "
						   "to you and %1.").arg(FContactName);
        ssExplanation = tr("To authenticate via shared secret, "
                           "enter a secret only known "
						   "to you and %1.").arg(FContactName);
        fprExplanation = tr("To authenticate manually, exchange your "
                            "fingerprints over an authenticated channel and "
                            "compare each other's fingerprint with the one "
                            "listed beneath.");
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

	FMethodBox = new QComboBox(this);
	FMethodBox->setMinimumWidth(300);

	FMethodBox->addItem(tr("Question and answer"));
	FMethodBox->addItem(tr("Shared secret"));
	FMethodBox->addItem(tr("Fingerprint verification"));

    QLabel* qaExplanationLabel = new QLabel(qaExplanation, this);
    qaExplanationLabel->setWordWrap(true);
    QLabel* ssExplanationLabel = new QLabel(ssExplanation, this);
    ssExplanationLabel->setWordWrap(true);

	FQuestionEdit     = new QLineEdit(this);
	FAnswerEdit       = new QLineEdit(this);
	FSharedSecretEdit = new QLineEdit(this);

    QLabel* questionLabel = new QLabel(tr("&Question:"), this);
	questionLabel->setBuddy(FQuestionEdit);

    QLabel* answerLabel = new QLabel(tr("A&nswer:"), this);
	answerLabel->setBuddy(FAnswerEdit);

    QLabel* sharedSecretLabel = new QLabel(tr("&Shared Secret:"), this);
	sharedSecretLabel->setBuddy(FSharedSecretEdit);

	FProgressBar  = new QProgressBar(this);

	FCancelButton = new QPushButton(tr("&Cancel"), this);
	FStartButton  = new QPushButton(tr("&Authenticate"), this);

	FStartButton->setDefault(true);

	FMethodWidget[METHOD_QUESTION] = new QWidget(this);
    QVBoxLayout* qaLayout = new QVBoxLayout;
    qaLayout->setContentsMargins(0, 0, 0, 0);
    qaLayout->addWidget(qaExplanationLabel);
    qaLayout->addSpacing(20);
    qaLayout->addWidget(questionLabel);
	qaLayout->addWidget(FQuestionEdit);
    qaLayout->addSpacing(10);
    qaLayout->addWidget(answerLabel);
	qaLayout->addWidget(FAnswerEdit);
    qaLayout->addSpacing(20);
	FMethodWidget[METHOD_QUESTION]->setLayout(qaLayout);

	FMethodWidget[METHOD_SHARED_SECRET] = new QWidget(this);
    QVBoxLayout* ssLayout = new QVBoxLayout;
    ssLayout->setContentsMargins(0, 0, 0, 0);
    ssLayout->addWidget(ssExplanationLabel);
    ssLayout->addSpacing(20);
    ssLayout->addWidget(sharedSecretLabel);
	ssLayout->addWidget(FSharedSecretEdit);
    ssLayout->addSpacing(20);
	FMethodWidget[METHOD_SHARED_SECRET]->setLayout(ssLayout);

	FMethodWidget[METHOD_FINGERPRINT] = nullptr;
	QLabel* authenticatedLabel = nullptr;
	if (FIsSender)
    {
		if (FOtr->isVerified(FAccount, FContact))
            authenticatedLabel = new QLabel(QString("<b>%1</b>")
                                             .arg(tr("This contact is already "
                                                     "authenticated.")), this);

		QString ownFpr = FOtr->getPrivateKeys().value(
							FAccount,
                            tr("No private key for account \"%1\"")
							  .arg(FOtr->humanAccount(FAccount)));

		FFingerprint = FOtr->getActiveFingerprint(FAccount, FContact);

        QLabel* fprExplanationLabel = new QLabel(fprExplanation, this);
        fprExplanationLabel->setWordWrap(true);

        QLabel* ownFprDescLabel = new QLabel(tr("Your fingerprint:"), this);
        QLabel* ownFprLabel     = new QLabel(ownFpr, this);
        QLabel* fprDescLabel    = new QLabel(tr("%1's fingerprint:")
												.arg(FContactName), this);
		QLabel* fprLabel        = new QLabel(FFingerprint.fingerprintHuman, this);
        ownFprLabel->setFont(QFont("monospace"));
        fprLabel->setFont(QFont("monospace"));

		FMethodWidget[METHOD_FINGERPRINT] = new QWidget(this);
        QVBoxLayout* fprLayout = new QVBoxLayout;
        fprLayout->setContentsMargins(0, 0, 0, 0);
        fprLayout->addWidget(fprExplanationLabel);
        fprLayout->addSpacing(20);
        fprLayout->addWidget(ownFprDescLabel);
        fprLayout->addWidget(ownFprLabel);
        fprLayout->addSpacing(10);
        fprLayout->addWidget(fprDescLabel);
        fprLayout->addWidget(fprLabel);
		FMethodWidget[METHOD_FINGERPRINT]->setLayout(fprLayout);
    }

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
	buttonLayout->addWidget(FCancelButton);
	buttonLayout->addWidget(FStartButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(20, 20, 20, 20);
    if (authenticatedLabel)
    {
        mainLayout->addWidget(authenticatedLabel);
        mainLayout->addSpacing(20);
    }
	mainLayout->addWidget(FMethodBox);
    mainLayout->addSpacing(20);
	mainLayout->addWidget(FMethodWidget[METHOD_QUESTION]);
	mainLayout->addWidget(FMethodWidget[METHOD_SHARED_SECRET]);
	if (FMethodWidget[METHOD_FINGERPRINT])
    {
		mainLayout->addWidget(FMethodWidget[METHOD_FINGERPRINT]);
    }
	mainLayout->addWidget(FProgressBar);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);


	connect(FMethodBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeMethod(int)));
	connect(FMethodBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(checkRequirements()));
	connect(FQuestionEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(checkRequirements()));
	connect(FAnswerEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(checkRequirements()));
	connect(FSharedSecretEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(checkRequirements()));
	connect(FCancelButton, SIGNAL(clicked()),
            this, SLOT(reject()));
	connect(FStartButton, SIGNAL(clicked()),
            this, SLOT(startAuthentication()));

	if (!FIsSender)
    {
		if (AQuestion.isEmpty())
			FMethod = METHOD_SHARED_SECRET;
        else
			FQuestionEdit->setText(AQuestion);
    }

	changeMethod(FMethod);
	FMethodBox->setCurrentIndex(FMethod);

    reset();
}

AuthenticationDialog::~AuthenticationDialog()
{
}

void AuthenticationDialog::reject()
{
	if (FState == AUTH_IN_PROGRESS)
		FOtr->abortSMP(FAccount, FContact);
    QDialog::reject();
}

void AuthenticationDialog::reset()
{
	FState = FIsSender? AUTH_READY : AUTH_IN_PROGRESS;

	FMethodBox->setEnabled(FIsSender);
	FQuestionEdit->setEnabled(FIsSender);
	FAnswerEdit->setEnabled(true);
	FSharedSecretEdit->setEnabled(true);
	FProgressBar->setEnabled(false);

	FProgressBar->setValue(0);

    checkRequirements();
}

bool AuthenticationDialog::finished()
{
	return FState == AUTH_FINISHED;
}

void AuthenticationDialog::checkRequirements()
{
	FStartButton->setEnabled((FMethod == METHOD_QUESTION &&
							  !FQuestionEdit->text().isEmpty() &&
							  !FAnswerEdit->text().isEmpty()) ||
							 (FMethod == METHOD_SHARED_SECRET &&
							  !FSharedSecretEdit->text().isEmpty()) ||
							  (FMethod == METHOD_FINGERPRINT));
}

void AuthenticationDialog::changeMethod(int AIndex)
{
	FMethod = static_cast<Method>(AIndex);
    for (int i=0; i<3; i++)
		if (FMethodWidget[i])
			FMethodWidget[i]->setVisible(i == AIndex);
	FProgressBar->setVisible(FMethod != METHOD_FINGERPRINT);
    adjustSize();
}

void AuthenticationDialog::startAuthentication()
{
	switch (FMethod)
    {
        case METHOD_QUESTION:
			if (FQuestionEdit->text().isEmpty() ||
				FAnswerEdit->text().isEmpty())
                return;

			FState = AUTH_IN_PROGRESS;

			FMethodBox->setEnabled(false);
			FQuestionEdit->setEnabled(false);
			FAnswerEdit->setEnabled(false);
			FProgressBar->setEnabled(true);
			FStartButton->setEnabled(false);

			if (FIsSender)
				FOtr->startSMP(FAccount, FContact, FQuestionEdit->text(), FAnswerEdit->text());
            else
				FOtr->continueSMP(FAccount, FContact, FAnswerEdit->text());
            updateSMP(33);
            break;

        case METHOD_SHARED_SECRET:
			if (FSharedSecretEdit->text().isEmpty())
                return;

			FState = AUTH_IN_PROGRESS;

			FMethodBox->setEnabled(false);
			FSharedSecretEdit->setEnabled(false);
			FProgressBar->setEnabled(true);
			FStartButton->setEnabled(false);

			if (FIsSender)
				FOtr->startSMP(FAccount, FContact, QString(), FSharedSecretEdit->text());
            else
				FOtr->continueSMP(FAccount, FContact, FSharedSecretEdit->text());
            updateSMP(33);
            break;

        case METHOD_FINGERPRINT:
			if (FFingerprint.fingerprint)
            {
				QString msg(tr("Account: ") + FOtr->humanAccount(FAccount) + "\n" +
							tr("User: ") + FContact + "\n" +
							tr("Fingerprint: ") + FFingerprint.fingerprintHuman + "\n\n" +
                            tr("Have you verified that this is in fact the correct fingerprint?"));

                QMessageBox mb(QMessageBox::Information, tr("Psi OTR"),
                               msg, QMessageBox::Yes | QMessageBox::No, this,
                               Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

				FOtr->verifyFingerprint(FFingerprint,
                                         mb.exec() == QMessageBox::Yes);
                close();
            }
            break;
    }
}

void AuthenticationDialog::updateSMP(int AProgress)
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

	FProgressBar->setValue(AProgress);

	if (AProgress == 100) {
		if (FIsSender || FMethod == METHOD_SHARED_SECRET)
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

void AuthenticationDialog::notify(const QMessageBox::Icon AIcon,
								  const QString& AMessage)
{
	QMessageBox mb(AIcon, tr("Off-the-Record messaging"),
				   AMessage, QMessageBox::Ok, this,
                   Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    mb.exec();
}

//
//-----------------------------------------------------------------------------
//

OtrClosure::OtrClosure(const QString& AAccount, const QString& AContact,
							 Otr *AOtr):
	FOtr(AOtr),
	FAccount(AAccount),
	FContact(AContact),
	FIsLoggedIn(false)
{
}

OtrClosure::~OtrClosure()
{
}

void OtrClosure::authenticateContact()
{
	if (FAuthDialog || !encrypted())
        return;

	FAuthDialog = new AuthenticationDialog(FOtr, FAccount, FContact, QString(), true);
	FAuthDialog->show();
}

bool OtrClosure::isRunning() const
{
	return FAuthDialog;
}

//-----------------------------------------------------------------------------

void OtrClosure::receivedSmp(const QString& AQuestion)
{
	if ((FAuthDialog && !FAuthDialog->finished()) || !encrypted())
    {
		FOtr->abortSMP(FAccount, FContact);
        return;
    }

	if (!FAuthDialog)
		FAuthDialog = new AuthenticationDialog(FOtr, FAccount, FContact, AQuestion, false);
}

int OtrClosure::showSmpDialog()
{
	return FAuthDialog->exec();
}

//-----------------------------------------------------------------------------

void OtrClosure::updateSmpDialog(int AProgress)
{
	if (FAuthDialog)
    {		
		FAuthDialog->show();
		FAuthDialog->raise();
		FAuthDialog->activateWindow();
		FAuthDialog->updateSMP(AProgress);
    }
}

//-----------------------------------------------------------------------------

void OtrClosure::setIsLoggedIn(bool AIsLoggedIn)
{
	FIsLoggedIn = AIsLoggedIn;
}

//-----------------------------------------------------------------------------

bool OtrClosure::isLoggedIn() const
{
	return FIsLoggedIn;
}

//-----------------------------------------------------------------------------

bool OtrClosure::encrypted() const
{
	return FOtr->getMessageState(FAccount, FContact) ==
		   IOtr::MsgStateEncrypted;
}

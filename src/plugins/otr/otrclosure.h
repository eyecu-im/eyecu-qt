/*
 * psiotrclosure.h
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *               2011-2012  Florian Fieber
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

#ifndef PSIOTRCLOSURE_H_
#define PSIOTRCLOSURE_H_

#include <QMessageBox>
#include "otr.h"

class QAction;
class QMenu;
class QComboBox;
class QLineEdit;
class QProgressBar;
class QPushButton;

class AuthenticationDialog : public QDialog
{
    Q_OBJECT
public:
	AuthenticationDialog(Otr* AOtr, const QString& AAccount,
						 const QString& AContact, const QString& AQuestion,
						 bool ASender, QWidget* AParent = nullptr);
    ~AuthenticationDialog();

    void reset();
    bool finished();
	void updateSMP(int AProgress);
	void notify(const QMessageBox::Icon AIcon, const QString& AMessage);

public slots:
    void reject();

private:
    enum AuthState {AUTH_READY, AUTH_IN_PROGRESS, AUTH_FINISHED};
    enum Method {METHOD_QUESTION, METHOD_SHARED_SECRET, METHOD_FINGERPRINT};

	Otr*			FOtr;
	Method			FMethod;
	QString			FAccount;
	QString			FContact;
	QString			FContactName;
	bool			FIsSender;
	AuthState		FState;
	OtrFingerprint	FFingerprint;

	QWidget*		FMethodWidget[3];
	QComboBox*		FMethodBox;
	QLineEdit*		FQuestionEdit;
	QLineEdit*		FAnswerEdit;
	QLineEdit*		FSharedSecretEdit;
	QProgressBar*	FProgressBar;
	QPushButton*	FCancelButton;
	QPushButton*	FStartButton;

private slots:
	void changeMethod(int AIndex);
    void checkRequirements();
    void startAuthentication();
};

//-----------------------------------------------------------------------------

class OtrClosure : public QObject
{
    Q_OBJECT

public:
	OtrClosure(const QString& AAccount, const QString& AContact, Otr* AOtr);
	~OtrClosure();
	void setIsLoggedIn(bool AIsLoggedIn);
    bool isLoggedIn() const;
    bool encrypted() const;
	void receivedSMP(const QString& AQuestion, QWidget *AParent=nullptr);
	void updateSMP(int AProgress);
    void authenticateContact();

private:
	Otr*	FOtr;
	QString	FAccount;
	QString	FContact;
	bool	FIsLoggedIn;
	AuthenticationDialog* FAuthDialog;

public slots:
    void finishAuth();
};

#endif

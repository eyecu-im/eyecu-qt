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
#include <QPointer>
// #include "otr.h"
#include "otrauthdialog.h"

class QAction;
class QMenu;
class QComboBox;
class QLineEdit;
class QProgressBar;
class QPushButton;

//-----------------------------------------------------------------------------

class OtrClosure : public QObject
{
    Q_OBJECT

public:
	OtrClosure(const QString &AAccount, const QString &AContact, Otr *AOtr);
	~OtrClosure();
	void setIsLoggedIn(bool AIsLoggedIn);
    bool isLoggedIn() const;
    bool encrypted() const;
	void receivedSmp(const QString& AQuestion);
	void showSmpDialog();
	void updateSmpDialog(int AProgress);
    void authenticateContact();
	bool isRunning() const;

private:
	Otr*	FOtr;
	QString	FAccount;
	QString	FContact;
	bool	FIsLoggedIn;
	QPointer<OtrAuthDialog> FAuthDialog;
};

#endif

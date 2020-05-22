/*
 * otrclosure.cpp
 *
 * Off-the-Record Messaging plugin for eyeCU
 * Copyright (C) 2018-2024  Konsyantin Kozlov AKA Yagiza yagiza@yandex.ru)
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

#include "otrclosure.h"
#include "otrauthdialog.h"

OtrClosure::OtrClosure(const Jid &AStreamJid, const Jid &AContactJid, Otr *AOtr):
	FOtr(AOtr),
	FAccount(AStreamJid),
	FContact(AContactJid),
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

	FAuthDialog = new OtrAuthDialog(FOtr, FAccount, FContact, QString(), true);
	FAuthDialog->show();
}

bool OtrClosure::isRunning() const
{
	return FAuthDialog;
}

void OtrClosure::receivedSmp(const QString& AQuestion)
{
	if ((FAuthDialog && !FAuthDialog->finished()) || !encrypted())
    {
		FOtr->abortSMP(FAccount, FContact);
        return;
    }

	if (!FAuthDialog)
		FAuthDialog = new OtrAuthDialog(FOtr, FAccount, FContact, AQuestion, false);
}

void OtrClosure::showSmpDialog()
{
	FAuthDialog->show();
}

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

void OtrClosure::setIsLoggedIn(bool AIsLoggedIn)
{
	FIsLoggedIn = AIsLoggedIn;
}

bool OtrClosure::isLoggedIn() const
{
	return FIsLoggedIn;
}

bool OtrClosure::encrypted() const
{
	return FOtr->getMessageState(FAccount, FContact) ==
		   IOtr::MsgStateEncrypted;
}

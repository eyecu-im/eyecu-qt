/*
 * otrmessaging.cpp - Interface to libotr
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *                    2011  Florian Fieber
 *                    2014  Boris Pek (tehnick-8@mail.ru)
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

#include "otrmessaging.h"
#include "otrinternal.h"

#include <QString>
#include <QList>
#include <QHash>

OtrFingerprint::OtrFingerprint()
    : fingerprint(NULL)
{

}

OtrFingerprint::OtrFingerprint(const OtrFingerprint &fp)
    : fingerprint(fp.fingerprint),
      account(fp.account),
      username(fp.username),
      fingerprintHuman(fp.fingerprintHuman),
      trust(fp.trust)
{

}

OtrFingerprint::OtrFingerprint(unsigned char* fingerprint,
                         QString account, QString username,
                         QString trust)
    : fingerprint(fingerprint),
      account(account),
      username(username),
      trust(trust)
{
    fingerprintHuman = OtrInternal::humanFingerprint(fingerprint);
}

//-----------------------------------------------------------------------------

OtrMessaging::OtrMessaging(IOtr* callback, IOtr::OtrPolicy policy)
	: FOtrPolicy(policy),
	  FOtrInternal(new OtrInternal(callback, FOtrPolicy)),
	  FOtr(callback)
{
}

//-----------------------------------------------------------------------------

OtrMessaging::~OtrMessaging()
{
	delete FOtrInternal;
}

//-----------------------------------------------------------------------------

QString OtrMessaging::encryptMessage(const QString& account,
                                     const QString& contact,
                                     const QString& message)
{
	return FOtrInternal->encryptMessage(account, contact, message);
}

//-----------------------------------------------------------------------------

IOtr::OtrMessageType OtrMessaging::decryptMessage(const QString& account,
												  const QString& contact,
												  const QString& message,
												  QString& decrypted)
{
	return FOtrInternal->decryptMessage(account, contact, message, decrypted);
}

//-----------------------------------------------------------------------------

QList<OtrFingerprint> OtrMessaging::getFingerprints()
{
	return FOtrInternal->getFingerprints();
}

//-----------------------------------------------------------------------------

void OtrMessaging::verifyFingerprint(const OtrFingerprint& AFingerprint,
									 bool AVerified)
{
	FOtrInternal->verifyFingerprint(AFingerprint, AVerified);
}

//-----------------------------------------------------------------------------

void OtrMessaging::deleteFingerprint(const OtrFingerprint& AFingerprint)
{
	FOtrInternal->deleteFingerprint(AFingerprint);
}

//-----------------------------------------------------------------------------

QHash<QString, QString> OtrMessaging::getPrivateKeys()
{
	return FOtrInternal->getPrivateKeys();
}

//-----------------------------------------------------------------------------

void OtrMessaging::deleteKey(const QString& AAccount)
{
	FOtrInternal->deleteKey(AAccount);
}

//-----------------------------------------------------------------------------

void OtrMessaging::startSession(const QString& AAccount, const QString& AContact)
{
	FOtrInternal->startSession(AAccount, AContact);
}

//-----------------------------------------------------------------------------

void OtrMessaging::endSession(const QString& AAccount, const QString& AContact)
{
	FOtrInternal->endSession(AAccount, AContact);
}

//-----------------------------------------------------------------------------

void OtrMessaging::expireSession(const QString& AAccount, const QString& AContact)
{
	FOtrInternal->expireSession(AAccount, AContact);
}

//-----------------------------------------------------------------------------

void OtrMessaging::startSMP(const QString& AAccount, const QString& AContact,
							const QString& AQuestion, const QString& ASecret)
{
	FOtrInternal->startSMP(AAccount, AContact, AQuestion, ASecret);
}

//-----------------------------------------------------------------------------

void OtrMessaging::continueSMP(const QString& AAccount, const QString& AContact,
							   const QString& ASecret)
{
	FOtrInternal->continueSMP(AAccount, AContact, ASecret);
}

//-----------------------------------------------------------------------------

void OtrMessaging::abortSMP(const QString& AAccount, const QString& AContact)
{
	FOtrInternal->abortSMP(AAccount, AContact);
}

//-----------------------------------------------------------------------------

IOtr::OtrMessageState OtrMessaging::getMessageState(const QString& AAccount,
													const QString& AContact)
{
	return FOtrInternal->getMessageState(AAccount, AContact);
}

//-----------------------------------------------------------------------------

QString OtrMessaging::getMessageStateString(const QString& AAccount,
											const QString& AContact)
{
	return FOtrInternal->getMessageStateString(AAccount, AContact);
}

//-----------------------------------------------------------------------------

QString OtrMessaging::getSessionId(const QString& AAccount,
								   const QString& AContact)
{
	return FOtrInternal->getSessionId(AAccount, AContact);
}

//-----------------------------------------------------------------------------

OtrFingerprint OtrMessaging::getActiveFingerprint(const QString& AAccount,
												  const QString& AContact)
{
	return FOtrInternal->getActiveFingerprint(AAccount, AContact);
}

//-----------------------------------------------------------------------------

bool OtrMessaging::isVerified(const QString& AAccount, const QString& AContact)
{
	return FOtrInternal->isVerified(AAccount, AContact);
}

//-----------------------------------------------------------------------------

bool OtrMessaging::smpSucceeded(const QString& AAccount, const QString& AContact)
{
	return FOtrInternal->smpSucceeded(AAccount, AContact);
}

//-----------------------------------------------------------------------------

void OtrMessaging::setPolicy(IOtr::OtrPolicy APolicy)
{
	FOtrPolicy = APolicy;
}

//-----------------------------------------------------------------------------

IOtr::OtrPolicy OtrMessaging::getPolicy()
{
	return FOtrPolicy;
}

//-----------------------------------------------------------------------------

void OtrMessaging::generateKey(const QString& AAccount)
{
	FOtrInternal->generateKey(AAccount);
}

//-----------------------------------------------------------------------------

bool OtrMessaging::displayOtrMessage(const QString& AAccount,
									 const QString& AContact,
									 const QString& AMessage)
{
	return FOtr->displayOtrMessage(AAccount, AContact, AMessage);
}

//-----------------------------------------------------------------------------

void OtrMessaging::stateChange(const QString& AAccount, const QString& AContact,
							   IOtr::OtrStateChange AChange)
{
	return FOtr->stateChange(AAccount, AContact, AChange);
}

//-----------------------------------------------------------------------------

QString OtrMessaging::humanAccount(const QString& AAccountId)
{
	return FOtr->humanAccount(AAccountId);
}

//-----------------------------------------------------------------------------

QString OtrMessaging::humanContact(const QString& AAccountId,
								   const QString& AContact)
{
	return FOtr->humanContact(AAccountId, AContact);
}

//-----------------------------------------------------------------------------

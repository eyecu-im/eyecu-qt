/*
 * psiotrconfig.h - Configuration dialogs
 *
 * Off-the-Record Messaging plugin for Psi+
 * Copyright (C) 2007-2011  Timo Engel (timo-e@freenet.de)
 *                    2011  Florian Fieber
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

#ifndef PSIOTRCONFIG_H_
#define PSIOTRCONFIG_H_

#include "otrmessaging.h"

// xnamed! <<
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/ipresencemanager.h>
// xnamed! >>

#include <QWidget>
#include <QVariant>

//class OptionAccessingHost;
class OtrCallback;
//class IAccountManager;
//class AccountInfoAccessingHost;
//class IOptionsManager;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QStandardItemModel;
class QTableView;
class QPoint;

// ---------------------------------------------------------------------------

namespace psiotr
{

// ---------------------------------------------------------------------------

const QString  OPTION_POLICY            = "otr-policy";
const QVariant DEFAULT_POLICY           = QVariant(OTR_POLICY_ENABLED);
const QString  OPTION_END_WHEN_OFFLINE  = "end-session-when-offline";
const QVariant DEFAULT_END_WHEN_OFFLINE = QVariant(false);

// ---------------------------------------------------------------------------

/**
 * This dialog appears in the 'Plugins' section of the Psi configuration.
 */
class ConfigDialog : public QWidget,
                     public IOptionsDialogWidget
{
Q_OBJECT;
Q_INTERFACES(IOptionsDialogWidget);

public:
//    ConfigDialog(OtrMessaging* otr, OptionAccessingHost* optionHost,
//                 AccountInfoAccessingHost* accountInfo,
    ConfigDialog(OtrMessaging* otr, OtrCallback* optionHost,
                 QWidget* parent = 0);
    virtual QWidget* instance() { return this; }

private:
    OtrMessaging*             m_otr;
    //OptionAccessingHost*      m_optionHost;
    //AccountInfoAccessingHost* m_accountInfo;
    IOptionsManager *FOptionsManager;
    IAccountManager *FAccountManager;
public slots:
    virtual void apply();
    virtual void reset();
signals:
    void modified();
    void childApply();
    void childReset();
};

// ---------------------------------------------------------------------------

/**
 * Configure OTR policy.
 */
class ConfigOtrWidget : public QWidget
{
Q_OBJECT

public:
    ConfigOtrWidget(//IOptionsManager* optionHost,
                    OtrMessaging* otr,
                    QWidget* parent = 0);

private:
    //OptionAccessingHost* m_optionHost;
    OtrMessaging*        m_otr;

    QButtonGroup*        m_policy;

    QCheckBox*           m_endWhenOffline;

    IOptionsManager *FOptionsManager;

private slots:
    void updateOptions();
};

// ---------------------------------------------------------------------------

/**
 * Show fingerprint of your contacts.
 */
class FingerprintWidget : public QWidget
{
Q_OBJECT

public:
    FingerprintWidget(OtrMessaging* otr, QWidget* parent = 0);

protected:
    void updateData();

private:
    OtrMessaging*       m_otr;
    QTableView*         m_table;
    QStandardItemModel* m_tableModel;
    QList<Fingerprint>  m_fingerprints;

private slots:
    void deleteFingerprint();
    void verifyFingerprint();
    void copyFingerprint();
    void contextMenu(const QPoint& pos);
};

// ---------------------------------------------------------------------------

/**
 * Display a table with account and fingerprint of private key.
 */
class PrivKeyWidget : public QWidget
{
Q_OBJECT

public:
//    PrivKeyWidget(AccountInfoAccessingHost* accountInfo,
//                  OtrMessaging* otr, QWidget* parent);
    PrivKeyWidget(OtrMessaging* otr, QWidget* parent);

protected:
    void updateData();

private:
    //AccountInfoAccessingHost* m_accountInfo;
    OtrMessaging*             m_otr;
    QTableView*               m_table;
    QStandardItemModel*       m_tableModel;
    QHash<QString, QString>   m_keys;
    QComboBox*                m_accountBox;

    IOptionsManager *FOptionsManager;
    IPresenceManager  *FPresenceManager;
    IAccountManager  *FAccountManager;

private slots:
    void deleteKey();
    void generateKey();
    void copyFingerprint();
    void contextMenu(const QPoint& pos);
};

//-----------------------------------------------------------------------------

} // namespace psiotr

#endif

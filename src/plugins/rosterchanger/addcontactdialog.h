#ifndef ADDCONTACTDIALOG_H
#define ADDCONTACTDIALOG_H

#include <QDialog>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/irosterchanger.h>
#include <interfaces/irostermanager.h>
#include <interfaces/ivcardmanager.h>
#include <interfaces/iaccountmanager.h>
#include "ui_addcontactdialog.h"

class AddContactDialog :
	public QDialog,
	public IAddContactDialog
{
	Q_OBJECT;
	Q_INTERFACES(IAddContactDialog);
public:
	AddContactDialog(IRosterChanger *ARosterChanger, const Jid &AStreamJid, QWidget *AParent = NULL);
	~AddContactDialog();
	//IAddContactDialog
	virtual QDialog *instance() { return this; }
	virtual Jid streamJid() const;
	virtual Jid contactJid() const;
	virtual void setContactJid(const Jid &AContactJid);
	virtual QString nickName() const;
	virtual void setNickName(const QString &ANick);
	virtual QString group() const;
	virtual void setGroup(const QString &AGroup);
	virtual bool subscribeContact() const;
	virtual void setSubscribeContact(bool ASubscribe);
	virtual QString subscriptionMessage() const;
	virtual void setSubscriptionMessage(const QString &AMessage);
	virtual void updateSubscriptionMessage(const QString &ANick);
	virtual ToolBarChanger *toolBarChanger() const;
signals:
	void dialogDestroyed();
protected:
	void initialize();
protected slots:
	void onDialogAccepted();
	void onToolBarActionTriggered(bool);
	void onVCardReceived(const Jid &AContactJid);
private:
	Ui::AddContactDialogClass ui;
private:
	IRoster *FRoster;
	IMessageProcessor *FMessageProcessor;
	IVCardManager *FVCardManager;
	IRosterChanger *FRosterChanger;
private:
	Action *FShowChat;
	Action *FSendMessage;
	Action *FShowVCard;
	Action *FResolve;
private:
	bool FResolving;
	Jid FStreamJid;
	ToolBarChanger *FToolBarChanger;
};

#endif // ADDCONTACTDIALOG_H

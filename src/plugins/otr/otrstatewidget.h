#ifndef OTRSATEWIDGET_H
#define OTRSATEWIDGET_H

#include <QLabel>
#include <QToolButton>
#include <interfaces/imessagewidgets.h>

#include "otrmessaging.h"

class OtrStateWidget: public QToolButton
{
	Q_OBJECT

public:
	OtrStateWidget(IOtr* AOtr, OtrMessaging* AOtrMessaging, IMessageWindow *AWindow,
				 const QString &AAccount, const QString &AContact, QWidget *AParent);
	~OtrStateWidget();
protected slots:
	void onWindowAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onUpdateMessageState(const Jid &AStreamJid, const Jid &AContactJid);
protected slots:
    void initiateSession(bool b);
    void endSession(bool b);
    void authenticateContact(bool b);
    void sessionID(bool b);
    void fingerprint(bool b);
private:
	IOtr*		  FOtr;
	OtrMessaging* FOtrMessaging;
	QString       FAccount;
	QString       FContact;
	IMessageWindow *FWindow;
private:
	Menu		  *FMenu;
	Action*       FAuthenticateAction;
	Action*       FSessionIdAction;
	Action*       FFingerprintAction;
	Action*       FStartSessionAction;
	Action*       FEndSessionAction;

};

#endif // OTRSATEWIDGET_H

#ifndef OTRSATEWIDGET_H
#define OTRSATEWIDGET_H

#include <QLabel>
#include <QToolButton>
#include <interfaces/imessagewidgets.h>

#include "otrmessaging.h"

namespace psiotr
{

class OtrStateWidget :
	public QToolButton
{
	Q_OBJECT;
public:
	OtrStateWidget(OtrCallback* callback, OtrMessaging* otrc, IMessageWindow *AWindow,
		         const QString &account, const QString &contact, QWidget *AParent);
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
    OtrCallback* m_callback;
    OtrMessaging* m_otr;
    QString       m_account;
    QString       m_contact;
	IMessageWindow *FWindow;
private:
	Menu *FMenu;
    Action*       m_authenticateAction;
    Action*       m_sessionIdAction;
    Action*       m_fingerprintAction;
    Action*       m_startSessionAction;
    Action*       m_endSessionAction;

};

} // namespace psiotr

#endif // OTRSATEWIDGET_H

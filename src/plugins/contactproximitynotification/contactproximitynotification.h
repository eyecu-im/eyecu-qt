#ifndef CONTACTPROXINITYNOTIFICATION_H
#define CONTACTPROXINITYNOTIFICATION_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/igeoloc.h>
#include <interfaces/imapcontacts.h>
#include <interfaces/ipositioning.h>
#include <interfaces/inotifications.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/ioptionsmanager.h>

#define CONTACTPROXINITYNOTIFICATION_UUID "{4CA8FD05-37C1-8C61-210A-76BCA97D4EF8}"

class ContactProximityNotification:  public QObject, public IPlugin, public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IContactProximityNotification")
#endif
public:
    ContactProximityNotification(QObject *parent = 0);

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return CONTACTPROXINITYNOTIFICATION_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

protected:
    void displayNotification(const Jid &AStreamJid, const Jid &AContactJid);
	void checkContactProximity(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, const GeolocElement &ACurrentPosition);
	void checkContactsProximity(const GeolocElement &ACurrentPosition);
    void removeProximityNotification(const Jid &AStreamJid, const Jid &AContactJid);
    IPresenceItem presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const;

protected slots:
    void onLocationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged);
    void onLocationRemoved(const Jid &AStreamJid, const Jid &AContactJid);
	void onNewPositionAvailable(const GeolocElement &APosition);
    void onNotificationActivated(int ANotifyId);
    void onNotificationRemoved(int ANotifyId);
    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onWindowActivated();
    void onContactShowedOnTheMap(const QString &AId);

private:
	INotifications		*FNotifications;
	IPositioning		*FPositioning;
	IMapContacts		*FMapContacts;
	IMessageProcessor	*FMessageProcessor;
	IMessageWidgets		*FMessageWidgets;
	IPresenceManager	*FPresenceManager;
	IOptionsManager		*FOptionsManager;
	QList<Jid>			FNotifiedContacts;
    QHash<Jid, QPair<Jid, MercatorCoordinates> > FContactCoordinates;
    QHash<Jid, QHash<Jid, int> > FNotifies;
};

#endif // CONTACTPROXINITYNOTIFICATION_H

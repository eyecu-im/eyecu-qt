#ifndef GEOLOC_H
#define GEOLOC_H

#include <interfaces/igeoloc.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/itraymanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/istatusicons.h>
#include <interfaces/irostermanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/imap.h>
#include <interfaces/ipositioning.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imapcontacts.h>

#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkinds.h>

class Geoloc: public QObject,
			  public IPlugin,
			  public IGeoloc,
			  public IOptionsDialogHolder,
			  public IRostersLabelHolder,
			  public IPEPHandler
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IGeoloc IOptionsDialogHolder IRostersLabelHolder IPEPHandler ) // IRosterDataHolder
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IGeoloc")
#endif
public:
	Geoloc();
	~Geoloc();

	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return GEOLOC_UUID;}
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin(){return true;}

	//IPEPHandler
	virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza);

	//IRostersLabelHolder
	QList<quint32> rosterLabels(int AOrder, const IRosterIndex *AIndex) const;
	AdvancedDelegateItem rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const;

	//IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

	//IGeoloc
	virtual QIcon   getIcon() const;
	virtual QString getIconFileName() const;
	virtual GeolocElement getGeoloc(const Jid &AJid) const;
	virtual bool    hasGeoloc(const Jid &AJid) const;
	virtual quint32 rosterLabelId() const {return FRosterLabelId; }
	virtual QString getLabel(const Jid &AContactJid) const;
	virtual QString getLabel(const GeolocElement &AGeoloc) const;

protected:
	void updateRosterLabels(const Jid &AContactJid);
	void putGeoloc(const Jid &AStreamJid, const Jid &AContactJid, const GeolocElement &AGeolocElement);
	void removeGeoloc(const Jid &AStreamJid, const Jid &AContactJid);
	void sendGeoloc(GeolocElement APosition);
	void sendGeoloc(const GeolocElement &APosition, const Jid &AStreamJid);
	void registerDiscoFeatures();
	void updateChatWindows();
	void updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid);
	void updateChatWindow(IMessageChatWindow *AMessageChatWindow);
	bool checkRosterIndex(const IRosterIndex *AIndex)	const;

protected slots:
	virtual bool onNewPositionAvailable(const GeolocElement &APosition);

	void retractGeoloc();
	void onStreamOpened(IXmppStream *AXmppStream);
	void onStreamClosed(IXmppStream *AXmppStream);
	void onRosterIndexInserted(IRosterIndex *AIndex);
	void onRosterIndexDataChanged(IRosterIndex *AIndex, int ARole);
	void onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int,QString> &AToolTips);
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);
	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onPresenceActiveChanged(IPresence *APresence, bool AActive);

signals:
	void locationReceived(const Jid &AStreamJid, const Jid &AJidContact, const MercatorCoordinates &ACoordinates, bool AReliabilityChanged);
	void locationRemoved(const Jid &AStreamJid, const Jid &AJidContact);
	//IRostersLabelHolder
	void rosterLabelChanged(quint32 ALabelId, IRosterIndex *AIndex);

private:
	IPEPManager			*FPEPManager;
	IServiceDiscovery	*FDiscovery;
	IXmppStreamManager	*FXmppStreams;
	IOptionsManager		*FOptionsManager;
	IRostersModel		*FRostersModel;
	IRostersViewPlugin	*FRostersViewPlugin;
	IMessageWidgets		*FMessageWidgets;
	IMapContacts		*FMapContacts;
	IPositioning		*FPositioning;
	IAccountManager		*FAccountManager;

	IconStorage			*FIconStorage;
	quint32				FRosterLabelId;
	bool				FSimpleContactsView;
	bool				FToggleSend;
	QList<Jid>			FXMPPStreams;
	QHash<Jid, GeolocElement>		FGeolocHash;
	QHash<QString, GeolocElement>	FGeolocBareHash;
	QHash<Jid, QString>	FIdHash;
	const QList<int>	FRosterIndexKinds;
 };

#endif // GEOLOC_H

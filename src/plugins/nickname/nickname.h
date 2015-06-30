#ifndef NICKNAME_H
#define NICKNAME_H

#include <interfaces/inickname.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/irostermanager.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/irostersview.h>
#include <interfaces/iservicediscovery.h>

#include <utils/options.h>
#include <utils/menu.h>

class Nickname : public QObject,
                 public IPlugin,
                 public INickname,
                 public IRosterDataHolder,
				 public IOptionsDialogHolder,
                 public IStanzaHandler,
                 public IPEPHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin INickname IRosterDataHolder IOptionsDialogHolder IStanzaHandler IPEPHandler) // IMapObjectDataHolder
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.INickname")
#endif
public:
    Nickname();
    ~Nickname();

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return NICKNAME_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}

    // IStanzaHandler interface
    bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);

    //IPEPHandler
    bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza);    

    //IRosterDataHolder
    QList<int>  rosterDataRoles(int AOrder) const;
    QList<int>  rosterDataTypes() const {return FRosterIndexTypes;}
    QVariant    rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const;
    bool        setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole);

    //IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
    void updateDataHolder(const Jid &AContactJid);

protected:
    void registerDiscoFeatures();
    void sendNick(const QString &ANickname, const Jid &AStreamJid);
    QString currentItemId(const Jid &AStreamJid) const;
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;

protected slots:
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);
    void onRosterIndexToolTips(IRosterIndex *AIndex, quint32 ALabelId, QMap<int,QString> &AToolTips);
    void onRosterItemReceived(IRoster *ARoster, const IRosterItem &AItem, const IRosterItem &ABefore);

    void onPresenceOpened(IPresence *APresence);

signals:
    //IRosterDataHolder
    void rosterDataChanged(IRosterIndex *AIndex = NULL, int ARole = 0);
    //IMapObjectDataHolder
    void mapDataChanged(int AType, const QString &AId, int ARole);

private:
	IOptionsManager		*FOptionsManager;
	IStanzaProcessor	*FStanzaProcessor;
	IPEPManager			*FPEPManager;
	IServiceDiscovery	*FDiscovery;
	IPresenceManager	*FPresenceManager;
	IRosterManager		*FRosterManager;
	IRostersModel		*FRostersModel;
	IRostersViewPlugin	*FRostersViewPlugin;
	IAccountManager		*FAccountManager;

	QHash<QString, QString>		FNicknameHash;
	const QList<int>			FRosterIndexTypes;
	QHash<QString, QString>		FIdHash;
	QHash<QString, QStringList>	FNotifiedContacts;

	int					FSHIMessage;
	int					FSHIPresence;
	int					FSHOMessage;
	int					FSHOPresence;
};

#endif // NICKNAME_H

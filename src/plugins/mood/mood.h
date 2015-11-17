#ifndef MOOD_H
#define MOOD_H

#include <interfaces/imood.h>
#include <interfaces/imap.h>
#include <interfaces/ipepmanager.h>
#include <interfaces/ipresencemanager.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imapcontacts.h>

class MoodData
{
public:
    MoodData(const QString &AName, const QString &AText=QString()):name(AName=="no_mood"?QString():AName),text(AText){}
    MoodData(){}
    void clear(){name.clear(); text.clear();}

    QString name;
    QString text;

    bool isEmpty() const {return name.isEmpty();}

    bool operator == (const MoodData &AOther) {return name==AOther.name && text==AOther.text;}
    bool operator != (const MoodData &AOther) {return !operator == (AOther);}
};

typedef QHash<Jid, MoodData> MoodElement;
typedef QHash<Jid, Action *> MoodAction;

class Mood : public QObject,
             public IPlugin,
             public IMood,
             public IRosterDataHolder,
			 public IRostersLabelHolder,
			 public IOptionsDialogHolder,
             public IPEPHandler,
			 public MapObjectDataHolder,
             public IMessageEditor,
             public IMessageWriter
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IMood IRostersLabelHolder IRosterDataHolder IOptionsDialogHolder IPEPHandler MapObjectDataHolder IMessageEditor IMessageWriter)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMood")
#endif
public:
    Mood();
    ~Mood();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MOOD_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
    //IMessageEditor
    virtual bool messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection);
    //IMessageWriter
    virtual void writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
    virtual void writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);

    //IPEPHandler
    virtual bool processPEPEvent(const Jid &AStreamJid, const Stanza &AStanza);    

    //IMapDataHolder
	virtual QGraphicsItem *mapData(SceneObject *ASceneObject, int ARole, QGraphicsItem *ACurrentElement);

	//IRostersLabelHolder
	QList<quint32> rosterLabels(int AOrder, const IRosterIndex *AIndex) const;
	AdvancedDelegateItem rosterLabel(int AOrder, quint32 ALabelId, const IRosterIndex *AIndex) const;

    //IRosterDataHolder
    virtual QList<int> rosterDataRoles(int AOrder) const;
    virtual QVariant rosterData(int AOrder, const IRosterIndex *AIndex, int ARole) const;
    virtual bool setRosterData(int AOrder, const QVariant &AValue, IRosterIndex *AIndex, int ARole);

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
    virtual void updateDataHolder(const Jid &AContactJid);

    //IMood
	virtual QIcon   getIcon(const QString &AMoodName) const;
	virtual QIcon   getIcon(const Jid &AContactJid) const;
	virtual QString getIconFileName(const QString &AMoodName) const;
	virtual QString getIconFileName(const Jid &AContactJid) const;
    virtual QString getText(const Jid &AContactJid) const;
    virtual QString getLabel(const Jid &AContactJid) const;    

public slots:
    void onMapObjectInserted(int AType, const QString &AId);         // SLOT: Map object inserted
    void onMapObjectRemoved(int AType, const QString &AId);         // SLOT: Map object removed
    void onMapObjectShowed(int AType, const QString &AId){Q_UNUSED(AType) Q_UNUSED(AId)}  // SLOT: Map object showed

protected:
    void registerDiscoFeatures();
	void updateChatWindows(bool AInfoBar);
	void updateChatWindows(const Jid &AContactJid, const Jid &AStreamJid);
	void updateChatWindow(IMessageChatWindow *AMessageChatWindow);
	void updateChatWindowInfo(IMessageChatWindow *AMessageChatWindow);
    void sendMood(const MoodData &AMoodData, const Jid &AStreamJid);
    void setMoodForMessage(const MoodData &AMoodData, Message &AMessage);
    void setMoodForAccount(Jid AStreamJid);
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
    QString getLabel(const MoodData &AMoodData) const;
    void displayNotification(const Jid &AStreamJid, const Jid &AContactJid);
    void updateChatWindowActions(IMessageChatWindow *AChatWindow);
    void removeNotifiedMessages(IMessageChatWindow *AWindow);
    IPresenceItem presenceItemForBareJid(const Jid &AStreamJid, const Jid &AContactJid) const;
    void saveComments(const MoodData &AMoodData);

protected slots:
    void onStreamOpened(IXmppStream *AXmppStream);
    void onStreamClosed(IXmppStream *AXmppStream);
    void onRosterIndexInserted(IRosterIndex *AIndex);
    void onRosterIndexContextMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
	void onRosterIndexClipboardMenu(const QList<IRosterIndex *> &AIndexes, quint32 ALabelId, Menu *AMenu);
    void onRosterIndexToolTips(IRosterIndex * AIndex, quint32 ALabelId, QMap<int,QString> &AToolTips);
    void onSetMoodByAction(bool);
	void onCopyToClipboard();
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);
    void showMoodSelector(bool);
    void onShortcutActivated(const QString &AString, QWidget *AWidget);

    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onNormalWindowCreated(IMessageNormalWindow *AWindow);
    void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);

    void onWindowActivated();
    void onNotificationActivated(int ANotifyId);

    void onAddMood(bool);

    QString currentItemId(const Jid &AStreamJid) const;

signals:
	//IRosterDataHolder
	void rosterDataChanged(IRosterIndex *AIndex = NULL, int ARole = 0);
	//IMapDataHolder
	void mapDataChanged(int AType, const QString &AId, int ARole);
	//IRostersLabelHolder
	void rosterLabelChanged(quint32 ALabelId, IRosterIndex *AIndex);

private:
	IOptionsManager		*FOptionsManager;
	IMessageProcessor	*FMessageProcessor;
	IPEPManager			*FPEPManager;
	IServiceDiscovery	*FDiscovery;
	IRostersViewPlugin	*FRostersViewPlugin;
	IPresenceManager    *FPresenceManager;
	IRostersModel		*FRostersModel;
	IXmppStreamManager	*FXmppStreamManager;
	IMessageWidgets		*FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;
	IMap				*FMap;
	IconStorage			*FIconStorage;
	IMapContacts		*FMapContacts;
	INotifications		*FNotifications;
	MoodData			FCurrentMood;
	bool				FSimpleContactsView;
	quint32				FRosterLabelId;
    QHash<Jid, QHash<QString, MoodData> > FCurrentChatMood;

	QSet<Jid>			FStreamsOnline;
	QHash<Jid, MoodData> FMoodHash;
    QHash<QString, QStringList> FMoodTextLists;
	const QList<int>    FRosterIndexKinds;

    QHash<QString, QString> FIdHash;
    QStringList             FMoodList;
    QHash<QString,QString>  FMoodKeys;

    QHash<Jid, MoodElement >  FMoodChat;
    QHash<Jid, MoodElement >  FMoodMess;
    QHash<Jid, MoodAction >   FMoodChatActions;
    QHash<Jid, MoodAction >   FMoodMessActions;
    QHash<Jid, QHash<Jid, int> > FNotifies;
};

#endif // MOOD_H

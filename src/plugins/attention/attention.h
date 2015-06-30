#ifndef ATTENTION_H
#define ATTENTION_H

#include <interfaces/iattention.h>
#include <interfaces/iavatars.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/ixmppstreammanager.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/irostermanager.h>
#include <interfaces/irostersview.h>
#include <interfaces/irostersmodel.h>
#include <interfaces/istatusicons.h>

#include "attentiondialog.h"

struct WindowStatus
{
    QDateTime startTime;
    QDateTime createTime;
    QString lastStatusShow;
    QDate lastDateSeparator;
};

class Attention:
    public QObject,
    public IPlugin,
    public IAttention,
    public IMessageHandler,
    public IMessageWriter,
	public IOptionsDialogHolder,
    public IArchiveHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IAttention IMessageHandler IMessageWriter IOptionsDialogHolder IArchiveHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IAttention")
#endif
    enum NotifyKinds {
        AttentionPopup         = 0x0100
    };

public:
    Attention();
    ~Attention();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return ATTENTION_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}


    //IMeassageHandler
    virtual bool messageCheck(int AOrder, const Message &AMessage, int ADirection);
    virtual bool messageDisplay(const Message &AMessage, int ADirection);
    virtual INotification messageNotify(INotifications *ANotifications, const Message &AMessage, int ADirection);
    virtual bool messageShowWindow(int AMessageId);
    virtual bool messageShowWindow(int AOrder, const Jid &AStreamJid, const Jid &AContactJid, Message::MessageType AType, int AShowMode);

    //IMessageWriter
    virtual void writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual void writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang){Q_UNUSED(AOrder) Q_UNUSED(AMessage) Q_UNUSED(ADocument) Q_UNUSED(ALang)}

    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IArchiveHandler
    virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn);

protected:
    void registerDiscoFeatures();
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;

// Imported Lion's code
    IMessageChatWindow *getWindow(const Jid &AStreamJid, const Jid &AContactJid);
    void updateWindow(IMessageChatWindow *AWindow);
    void removeNotifiedMessages(IMessageChatWindow *AWindow);
    void setMessageStyle(IMessageChatWindow *AWindow);
	void fillContentOptions(IMessageChatWindow *AWindow, IMessageStyleContentOptions &AOptions) const;
    void showDateSeparator(IMessageChatWindow *AWindow, const QDateTime &ADateTime);
    void showStyledMessage(IMessageChatWindow *AWindow, const Message &AMessage);

private:
	IMessageProcessor	*FMessageProcessor;
	IMessageArchiver	*FMessageArchiver;
	IAvatars			*FAvatars;
	IOptionsManager		*FOptionsManager;
	IServiceDiscovery	*FDiscovery;
	IMessageWidgets		*FMessageWidgets;
	IMessageStyleManager *FMessageStyleManager;
	IconStorage			*FIconStorage;
	IStatusIcons		*FStatusIcons;
	IMainWindowPlugin	*FMainWindowPlugin;
	QMainWindow			*FMainWindow;
	INotifications		*FNotifications;

private:
    QMultiMap<IMessageChatWindow*, int>     FNotifiedMessages;
    QMap<int, AttentionDialog*>             FAttentionDialogs;
    QMap<IMessageChatWindow*, WindowStatus> FWindowStatus;

protected slots:
    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onSetAttentionByAction(bool);
    //-----
    void onNotificationAppended(int ANotifyId, const INotification &ANotification);
    void onNotificationRemoved(int ANotifyId);
    void onDelayedMainWindowActivation();
    //-----
    void onWindowActivated();
};

#endif // ATTENTION_H

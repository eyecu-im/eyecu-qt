#ifndef RECEIPTS_H
#define RECEIPTS_H

#include <interfaces/ireceipts.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagearchiver.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/imainwindow.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/inotifications.h>
#include <interfaces/imessagewidgets.h>

#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/namespaces.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>

#include <utils/options.h>

#define NAME_RECEIPTS   "receipts"

class Receipts : public QObject,
                 public IPlugin,
                 public IReceipts,
				 public IOptionsDialogHolder,
                 public IMessageEditor,
                 public IMessageWriter,
				 public IArchiveHandler
 {
    Q_OBJECT
	Q_INTERFACES(IPlugin IReceipts IOptionsDialogHolder IMessageEditor IMessageWriter IArchiveHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IReceipts")
#endif
public:
    Receipts();
    ~Receipts();

    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return RECEIPTS_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
    //IMessageEditor
    virtual bool messageReadWrite(int AOrder, const Jid &AStreamJid, Message &AMessage, int ADirection);
    //IMessageWriter
	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang);
	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang);
    //IArchiveHandler
    virtual bool archiveMessageEdit(int AOrder, const Jid &AStreamJid, Message &AMessage, bool ADirectionIn);

protected:
    QHash<QString, QString> getReceipts(Jid jid) const;
    void setDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId);
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
    void removeNotifiedMessages(IMessageChatWindow *AWindow);

signals:
	void messageDelivered(const Jid &AStreamJid, const Jid &AContactJid, const QString &AMessageId);

private:
    IMessageProcessor   *FMessageProcessor;
    IMessageArchiver    *FMessageArchiver;
    IServiceDiscovery   *FDiscovery;    
    IOptionsManager     *FOptionsManager;
    INotifications      *FNotifications;
    IMessageWidgets     *FMessageWidgets;
    IconStorage         *FIconStorage;

    QHash<IMessageChatWindow *, int>   FNotifies;
    QHash<Jid, QHash<Jid, QStringList> > FDeliveryRequestHash;

    void registerDiscoFeatures(bool ARegister);

protected slots:
    void onWindowActivated();
    void onNotificationActivated(int ANotifyId);
	//Options
	void onOptionsOpened();
	void onOptionsChanged(const OptionsNode &ANode);

signals:
    void delivered(const QString &AId);
};

#endif // RECEIPTS_H

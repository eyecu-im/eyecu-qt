#ifndef XHTMLIM_H
#define XHTMLIM_H

#include <QNetworkAccessManager>

#include <interfaces/ixhtmlim.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/ibitsofbinary.h>

#include "edithtml.h"
#include "xhtmloptions.h"

class XhtmlIm:
        public QObject,
        public IPlugin,
        public IXhtmlIm,
        public IMessageWriter,
		public IOptionsDialogHolder,
        public IMessageEditContentsHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IXhtmlIm IMessageWriter IOptionsDialogHolder IMessageEditContentsHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IXhtmlIm")
#endif
	Q_ENUMS(TimeUnits BobUrlProperties)
public:
    enum TimeUnits
    {
        Seconds,
        Minutes,
        Hours,
        Days,
        Weeks,
        Months,
        Years
    };

    enum BobUrlProperties
    {
        PMaxAge=QTextFormat::UserProperty,
        PMimeType,
        PEmbed
    };


    XhtmlIm();
    ~XhtmlIm();

	void updateUnitsComboBox(QComboBox *AComboBox, int AValue);

    //IPlugin
    QObject *instance() { return this; }
    QUuid pluginUuid() const { return XHTMLIM_UUID; }
    void pluginInfo(IPluginInfo *APluginInfo);
    bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    bool initObjects();
    bool initSettings();
    bool startPlugin(){return true;}
    //IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
    //IMessageWriter
    void writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
    void writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
    //IEditContentsHandler
    bool messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData);
    bool messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData);
    bool messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument);
    bool messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded);
public slots:
	void onBobUrlOpen(QUrl AUrl);

protected:
    static void fixHtml(QString &AHtmlCode);
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool isSupported(const IMessageAddress *AMessageAddress) const;
    void addRichTextEditToolbar(SplitterWidget *AplitterWidget, int AOrderId, IMessageEditWidget *AEditWidget, bool AEnableFormatAutoReset);
	void updateChatWindowActions(bool ARichTextEditor, IMessageChatWindow *AChatWindow);
	void updateNormalWindowActions(bool ARichTextEditor, IMessageNormalWindow *ANormalWindow);
	void updateMessageWindows(bool ARichTextEditor);
	void registerDiscoFeatures();
	void updateToolbar(bool ASupported, bool AEnabled, ToolBarChanger *AToolBarChanger);


	QTextCursor getCursor(bool ASelectWholeDocument=false, bool ASelect=true);
	void mergeFormatOnWordOrSelection(QTextCursor ACursor, const QTextCharFormat &AFormat);
	void clearFormatOnWordOrSelection();

protected slots:
	void onViewContextMenu(const QPoint &APosition, Menu *AMenu);
	void onImageCopy();
	void onImageCopyLink();
	void onImageSave();
	void onImageOpen();

	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onNormalWindowCreated(IMessageNormalWindow *AWindow);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onRichTextEditorToggled(bool AChecked);
	void onEditWidgetCreated(IMessageEditWidget *AWidget);
	void onEditWidgetContextMenuRequested(const QPoint &APosition, Menu *AMenu);

	void onResetFormat(bool AStatus);
	void onRemoveFormat();
	void onSelectFont();
	void onSelectDecoration(bool ASelected);
	void onInsertLink();
	void onInsertImage();
	void onSetToolTip();
	void onInsertSpecial();
	void onTextCode(bool AChecked);
	void onColor();
	void onIndentChange();

protected slots:
	void onOptionsChanged(const OptionsNode &ANode);

private:
    IOptionsManager*        FOptionsManager;
    IMessageProcessor*      FMessageProcessor;
    IMessageWidgets*        FMessageWidgets;
    IServiceDiscovery*      FDiscovery;
    IBitsOfBinary*          FBitsOfBinary;
    QNetworkAccessManager*  FNetworkAccessManager;
    IconStorage*            FIconStorage;
    QStringList             FValidSchemes;
	IMessageEditWidget		*FCurrentMessageEditWidget;
	int						FCurrentCursorPosition;
};

#endif // XHTMLIM_H

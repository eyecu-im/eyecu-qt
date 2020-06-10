#ifndef XHTMLIM_H
#define XHTMLIM_H

#include <QNetworkAccessManager>

#include <interfaces/ixhtmlim.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iservicediscovery.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/istanzacontentencryption.h>
#include <interfaces/imultiuserchat.h>
#include <interfaces/ibitsofbinary.h>

#include "edithtml.h"
#include "xhtmloptions.h"

#define DT_BOLD			1
#define DT_ITALIC		2
#define DT_UNDERLINE	3
#define DT_OVERLINE		4
#define DT_STRIKEOUT	5

#define FMT_NORMAL		0
#define FMT_HEADING1	1
#define FMT_HEADING2	2
#define FMT_HEADING3	3
#define FMT_HEADING4	4
#define FMT_HEADING5	5
#define FMT_HEADING6	6
#define FMT_PREFORMAT	7

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

	static void updateUnitsComboBox(QComboBox *AComboBox, int AValue);
	static int checkBlockFormat(const QTextCursor &ACursor);
	static void clearBlockProperties(QTextEdit *ATextEdit, int ABlockNumber, const QSet<QTextFormat::Property> &AProperties);
	static bool isPreformatted(const QTextCursor &ACursor);
	static bool isCode(const QTextCursor &ACursor);

	QTextCursor getCursor(QTextEdit *ATextEdit, bool ASelectWholeDocument=false, bool ASelect=true);
	QTextCursor getCursor(QTextEdit *ATextEdit, int APosition, bool ASelectWholeDocument=false);
	QTextCursor getCursor();
	void mergeFormatOnSelection(QTextCursor ACursor, const QTextCharFormat &AFormat, QTextEdit *ATextEdit=NULL);
	void clearFormatOnSelection(QTextCursor ACursor, QTextEdit *ATextEdit);
	void setFormat(QTextEdit *ATextEdit, int AFormatType, int APosition=-1);

    //IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return XHTMLIM_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin(){return true;}
    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);
    //IMessageWriter
	virtual bool writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang);
	virtual bool writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual bool writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang);
    //IEditContentsHandler
	virtual bool messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData);
	virtual bool messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData);
	virtual bool messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument);
	virtual bool messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded);

public slots:
	void onBobUrlOpen(QUrl AUrl);

protected:
    static void fixHtml(QString &AHtmlCode);
    bool isSupported(const Jid &AStreamJid, const Jid &AContactJid) const;
	bool isSupported(const IMessageAddress *AMessageAddress) const;
    void addRichTextEditToolbar(SplitterWidget *AplitterWidget, int AOrderId, IMessageEditWidget *AEditWidget, bool AEnableFormatAutoReset);
	void updateChatWindowActions(bool ARichTextEditor, IMessageChatWindow *AChatWindow);
	void updateMultiChatWindowActions(bool ARichTextEditor, IMultiUserChatWindow *AChatWindow);
	void updateNormalWindowActions(bool ARichTextEditor, IMessageNormalWindow *ANormalWindow);
	void updateMessageWindows(bool ARichTextEditor);
	void registerDiscoFeatures();
	void cleanupDocument(QTextDocument *ADocument);
	bool isFormatted(const QTextDocument *ADocument);

	IMessageEditWidget *messageEditWidget(Action **AAction);

	void selectFont(QTextEdit *AEditWidget, int APosition=-1);
	void selectColor(int AType, IMessageEditWidget *AEditWidget, int APosition=-1);
	void selectDecoration(QTextEdit *ATextEdit, QTextCursor ACursor, int ADecorationType, bool ASelected);
	void insertLink(QTextCursor ACursor, QWidget *AParent);
	void insertImage(QTextCursor ACursor, IMessageEditWidget *AEditWidget);
	void insertSpecial(QTextCursor ACursor, QChar ASpecialCharacter);
	void setCode(QTextEdit *ATextEdit, QTextCursor ACursor, bool ACode);
	void setCapitalization(QTextEdit *ATextEdit, QTextCursor ACursor, QFont::Capitalization ACapitalization);
	void setAlignment(QTextCursor ACursor, Qt::Alignment AAlignment);
	void changeIndent(QTextCursor ACursor, bool AIncrease);

protected slots:
	void onViewContextMenu(const QPoint &APosition, Menu *AMenu);
	void onImageCopy();
	void onImageCopyLink();
	void onImageSave();
	void onImageOpen();

	void onChatWindowCreated(IMessageChatWindow *AWindow);
	void onNormalWindowCreated(IMessageNormalWindow *AWindow);
	void onMultiChatWindowCreated(IMultiUserChatWindow *AWindow);
	void onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore);
	void onRichTextEditorToggled(bool AChecked);
	void onEditWidgetCreated(IMessageEditWidget *AWidget);
	void onEditWidgetContextMenuRequested(const QPoint &APosition, Menu *AMenu);

	void onShortcutActivated(const QString &AId, QWidget *AWidget);
	void onMessageSent();

	void onResetFormat(bool AStatus);
	void onRemoveFormat();
	void onSelectFont();
	void onSelectDecoration(bool ASelected);
	void onSelectCapitalization();
	void onInsertLink();
	void onInsertImage();
	void onInsertSpecial();
	void onTextCode(bool AChecked);
	void onColor();
	void onIndentChange();
	void onTextAlign();
	void onInsertList();
	void onSetFormat();

protected slots:
	void onOptionsChanged(const OptionsNode &ANode);

signals:
	void specialCharacterInserted(QChar ASpecialCharacter);

private:
    IOptionsManager*        FOptionsManager;
    IMessageProcessor*      FMessageProcessor;
	IStanzaContentEncrytion* FStanzaContentEncrytion;
    IMessageWidgets*        FMessageWidgets;
	IMultiUserChatManager*	FMultiUserChatManager;
    IServiceDiscovery*      FDiscovery;
    IBitsOfBinary*          FBitsOfBinary;
    QNetworkAccessManager*  FNetworkAccessManager;
    IconStorage*            FIconStorage;
    QStringList             FValidSchemes;
	QChar					FSpecialCharacter;
};

#endif // XHTMLIM_H

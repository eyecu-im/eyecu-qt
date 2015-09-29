#include <QDebug>
#include <QLayout>
#include <QBoxLayout>
#include <QColorDialog>
#include <QMainWindow>
#include <QTextObject>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QFontDialog>
#include <QDesktopServices>
#if QT_VERSION >= 0x050000
#include <QMimeData>
#include <QStandardPaths>
#endif

#include <interfaces/iurlprocessor.h>

#include <definitions/menuicons.h>
#include <definitions/namespaces.h>
#include <definitions/resources.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditororders.h>
#include <definitions/toolbargroups.h>
#include <definitions/resources.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/messagechatwindowwidgets.h>
#include <definitions/messagenormalwindowwidgets.h>
#include <definitions/messageeditcontentshandlerorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/resources.h>
#include <definitions/actiongroups.h>
#include <definitions/xhtmlicons.h>

#include <utils/textmanager.h>
#include <utils/animatedtextbrowser.h>
#include <utils/action.h>
#include <XmlTextDocumentParser>

#include "xhtmlim.h"
#include "savequery.h"
#include "resourceretriever.h"
#include "imageopenthread.h"
#include "insertimage.h"
#include "addlink.h"
#include "settooltip.h"

#define ADR_DECORATION_TYPE Action::DR_Parametr1
#define ADR_SPECIAL_SYMBOL  Action::DR_Parametr1
#define ADR_COLOR_TYPE		Action::DR_Parametr1

#define DT_UNDERLINE 1
#define DT_OVERLINE	 2
#define DT_STRIKEOUT 3

#define CT_FOREGROUND 0
#define CT_BACKGROUND 1

XhtmlIm::XhtmlIm():
	FOptionsManager(NULL),
	FMessageProcessor(NULL),
	FMessageWidgets(NULL),
	FDiscovery(NULL),
	FBitsOfBinary(NULL),
	FNetworkAccessManager(NULL),
	FIconStorage(NULL)
{}

XhtmlIm::~XhtmlIm()
{}

void XhtmlIm::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("XHTML-IM");
	APluginInfo->description = tr("Implements XEP-0071: XHTML-IM");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
}

bool XhtmlIm::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageProcessor").value(0);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());
	else  return false;

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IBitsOfBinary").value(0,NULL);
	if (plugin)
		FBitsOfBinary = qobject_cast<IBitsOfBinary *>(plugin->instance());

	plugin = APluginManager->pluginInterface("IMessageWidgets").value(0,NULL);
	if (plugin)
	{
		FMessageWidgets = qobject_cast<IMessageWidgets *>(plugin->instance());
		if (FMessageWidgets)
		{
			connect(FMessageWidgets->instance(),SIGNAL(editWidgetCreated(IMessageEditWidget *)),SLOT(onEditWidgetCreated(IMessageEditWidget *)));
			connect(FMessageWidgets->instance(),SIGNAL(chatWindowCreated(IMessageChatWindow *)),SLOT(onChatWindowCreated(IMessageChatWindow *)));
			connect(FMessageWidgets->instance(),SIGNAL(normalWindowCreated(IMessageNormalWindow *)),SLOT(onNormalWindowCreated(IMessageNormalWindow *)));
		}
	}


	plugin = APluginManager->pluginInterface("IUrlProcessor").value(0);
	if (plugin)
	{
		IUrlProcessor *urlProcessor = qobject_cast<IUrlProcessor *>(plugin->instance());
		if (urlProcessor)
			FNetworkAccessManager = urlProcessor->networkAccessManager();
	}
	if (!FNetworkAccessManager)
		FNetworkAccessManager=new QNetworkAccessManager(this);

	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	//AInitOrder = 100;   // This one should be initialized AFTER ...
	return true;
	//return FMessageWidgets!=NULL;
}

bool XhtmlIm::initObjects()
{
	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_XHTMLIM, tr("XHTML-IM"), SGO_MESSAGEWINDOWS_XHTMLIM);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGE, tr("Insert image"), tr("Alt+I", "Insert image"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTLINK, tr("Insert link"), tr("Alt+L", "Insert link"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNBSP, tr("Insert non-breaking space"), tr("Ctrl+Space", "Insert NBSP"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNEWLINE, tr("Insert new line"), tr("Alt+Return", "Insert new line"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_SETTOOLTIP, tr("Set tool tip"), tr("Alt+T", "Insert link"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_BOLD, tr("Bold"), tr("Ctrl+B", "Bold"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_ITALIC, tr("Italic"), tr("Ctrl+I", "Italic"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_UNDERLINE, tr("Underline"), tr("Ctrl+U", "Underline"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_STRIKEOUT, tr("Strikeout"), tr("Ctrl+S", "Strikeout"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_CODE, tr("Code"), tr("Alt+C", "Code"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FONT, tr("Font"), tr("Ctrl+F", "Font"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FOREGROUNDCOLOR, tr("Foreground color"), tr("Alt+F", "Foreground color"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_BACKGROUNDCOLOR, tr("Background color"), tr("Alt+B", "Background color"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNCENTER, tr("Center"), tr("Ctrl+E", "Align center"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNLEFT, tr("Left"), tr("Ctrl+L", "Align left"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNRIGHT, tr("Right"), tr("Ctrl+R", "Align right"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNJUSTIFY, tr("Justify"), tr("Ctrl+J", "Align justify"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATREMOVE, tr("Remove formatting"), tr("Alt+R", "Remove formatting"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATAUTORESET, tr("Toggle reset formatting on message send"), tr("Alt+A", "Toggle reset formatting on message send"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INDENTINCREASE, tr("Increase indent"), tr("", "Incerease indent"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INDENTDECREASE, tr("Decrease indent"), tr("Shift+Tab", "Decerease indent"), Shortcuts::WindowShortcut);

	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGEDIALOG, tr("\"Insert image\" dialog"), SGO_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGE);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGEDIALOG_BROWSE, tr("Browse"), tr("Ctrl+B", "Browse"), Shortcuts::WindowShortcut);

	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS_LINKDIALOG, tr("\"Add link\" dialog"), SGO_MESSAGEWINDOWS_LINKDIALOG);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_LINKDIALOG_OK, tr("Ok"), tr("Ctrl+Return", "Ok"), Shortcuts::WindowShortcut);

	FIconStorage = IconStorage::staticStorage(RSR_STORAGE_HTML);

	if (FDiscovery) registerDiscoFeatures();

	if (FMessageWidgets)
		FMessageWidgets->insertEditContentsHandler(ECHO_XHTML_COPY_INSERT,this);

	if (FMessageProcessor)
	{
		FMessageProcessor->insertMessageWriter(MWO_XHTML_M2T, this);
		FMessageProcessor->insertMessageWriter(MWO_XHTML_T2M, this);
	}

	if (FBitsOfBinary)
	{
		QDesktopServices::setUrlHandler("cid", this, "onBobUrlOpen");
		InsertImage::setBob(FBitsOfBinary);
	}

	FValidSchemes << "http" << "https" << "ftp" << "file" << "cid";

	return true;
}

void XhtmlIm::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = true;
	dfeature.var = NS_XHTML_IM;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_XHTML);
	dfeature.name = tr("XHTML-IM");
	dfeature.description = tr("Supports XHTML message formating");
	FDiscovery->insertDiscoFeature(dfeature);
}

void XhtmlIm::updateToolbar(bool ASupported, bool AEnabled, ToolBarChanger *AToolBarChanger)
{
	QList<QAction *> actions = AToolBarChanger->groupItems(TBG_MWTBW_RICHTEXT_EDITOR);
	QAction *action = actions.isEmpty()?NULL:actions.first();
	if (ASupported)
	{
		if (!action)
		{
			Action *action = new Action();
			action->setText(tr("Show rich text editor toolbar"));
			action->setIcon(FIconStorage->getIcon(XHI_RICHTEXT));
			action->setCheckable(true);
			action->setChecked(AEnabled);
			AToolBarChanger->insertAction(action, TBG_MWTBW_RICHTEXT_EDITOR);
			connect(action, SIGNAL(toggled(bool)), SLOT(onRichTextEditorToggled(bool)));
		}
	}
	else
	{
		if (action)
		{
			AToolBarChanger->removeItem(action);
			action->deleteLater();
		}
	}
}

bool XhtmlIm::initSettings()
{
	QString pictures;
#if QT_VERSION >= 0x050000
	QStringList dirList = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
	if (dirList.isEmpty())
	{
		dirList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
		if (dirList.isEmpty())
		{
			dirList = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
			if (dirList.isEmpty())
				dirList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
		}
	}
	if (!dirList.isEmpty())
		pictures = QDir::fromNativeSeparators(dirList.first());
#else
	pictures = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
	if (pictures.isEmpty())
		pictures = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
	if (pictures.isEmpty())
		pictures = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
	if (pictures.isEmpty())
		pictures = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
	pictures = QDir::fromNativeSeparators(pictures);
#endif
	Options::setDefaultValue(OPV_XHTML_MAXAGE, 2565000); // 1 month
	Options::setDefaultValue(OPV_XHTML_EMBEDSIZE, 1024);
	Options::setDefaultValue(OPV_XHTML_DEFAULTIMAGEFORMAT, "png");
	Options::setDefaultValue(OPV_XHTML_TABINDENT, true);
	Options::setDefaultValue(OPV_XHTML_NORICHTEXT, true);
	Options::setDefaultValue(OPV_XHTML_EDITORTOOLBAR, true);
	Options::setDefaultValue(OPV_XHTML_FORMATAUTORESET, true);
	Options::setDefaultValue(OPV_XHTML_IMAGESAVEDIRECTORY, pictures);
	Options::setDefaultValue(OPV_XHTML_IMAGEOPENDIRECTORY, pictures);

	if (FOptionsManager)
	{
		IOptionsDialogNode dnode = {ONO_XHTML, OPN_XHTML, MNI_XHTML, tr("XHTML")};
		FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
	}
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> XhtmlIm::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_XHTML)
	{
		widgets.insertMulti(OHO_XHTML_GENERAL, FOptionsManager->newOptionsDialogHeader(tr("General"), AParent));
		widgets.insertMulti(OWO_XHTML_TABINDENT, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_XHTML_TABINDENT), tr("Use indentation instead of tabulation at the beginning of the paragraph"), AParent));
		widgets.insertMulti(OWO_XHTML_NORICHTEXT, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_XHTML_NORICHTEXT), tr("Do not send rich text without formatting"), AParent));
		widgets.insertMulti(OWO_XHTML_EDITORTOOLBAR, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_XHTML_EDITORTOOLBAR), tr("Show rich text editor toolbar"), AParent));
		if (FBitsOfBinary)
		{
			widgets.insertMulti(OHO_XHTML_BOB, FOptionsManager->newOptionsDialogHeader(tr("Bits of binary"), AParent));
			widgets.insertMulti(OWO_XHTML_BOB, new XhtmlOptions(this, AParent));
		}
	}
	return widgets;
}

bool XhtmlIm::isSupported(const Jid &AStreamJid, const Jid &AContactJid) const
{
	return FDiscovery==NULL || !FDiscovery->hasDiscoInfo(AStreamJid,AContactJid)
			|| FDiscovery->discoInfo(AStreamJid,AContactJid).features.contains(NS_XHTML_IM);
}

bool XhtmlIm::isSupported(const IMessageAddress *AMessageAddress) const
{
	return isSupported(AMessageAddress->streamJid(), AMessageAddress->contactJid())	;
}

void XhtmlIm::addRichTextEditToolbar(SplitterWidget *ASplitterWidget, int AOrderId, IMessageEditWidget *AEditWidget, bool AEnableFormatAutoReset)
{
	ASplitterWidget->insertWidget(AOrderId, new EditHtml(AEditWidget, AEnableFormatAutoReset, FBitsOfBinary, FNetworkAccessManager, this));
}

void XhtmlIm::updateChatWindowActions(bool ARichTextEditor, IMessageChatWindow *AChatWindow)
{
	bool supported = isSupported(AChatWindow->address());
	updateToolbar(supported, ARichTextEditor, AChatWindow->toolBarWidget()->toolBarChanger());
	QWidget *xhtmlEdit = AChatWindow->messageWidgetsBox()->widgetByOrder(MCWW_RICHTEXTTOOLBARWIDGET);
	if(ARichTextEditor && supported)
	{
		if (!xhtmlEdit)
			addRichTextEditToolbar(AChatWindow->messageWidgetsBox(), MCWW_RICHTEXTTOOLBARWIDGET, AChatWindow->editWidget(), true);
	}
	else
		if (xhtmlEdit)
		{
			AChatWindow->messageWidgetsBox()->removeWidget(xhtmlEdit);
			xhtmlEdit->deleteLater();
		}
}

void XhtmlIm::updateNormalWindowActions(bool ARichTextEditor, IMessageNormalWindow *ANormalWindow)
{
	bool supported = isSupported(ANormalWindow->address());
	updateToolbar(supported, ARichTextEditor, ANormalWindow->toolBarWidget()->toolBarChanger());
	QWidget *xhtmlEdit = ANormalWindow->messageWidgetsBox()->widgetByOrder(MCWW_RICHTEXTTOOLBARWIDGET);
	if(ARichTextEditor && supported)
	{
		if (!xhtmlEdit)
			addRichTextEditToolbar(ANormalWindow->messageWidgetsBox(), MCWW_RICHTEXTTOOLBARWIDGET, ANormalWindow->editWidget(), false);
	}
	else
		if (xhtmlEdit)
		{
			ANormalWindow->messageWidgetsBox()->removeWidget(xhtmlEdit);
			xhtmlEdit->deleteLater();
		}
}


void XhtmlIm::updateMessageWindows(bool ARichTextEditor)
{
	QList<IMessageChatWindow *> chatWindows = FMessageWidgets->chatWindows();
	for (QList<IMessageChatWindow *>::ConstIterator it = chatWindows.constBegin(); it!=chatWindows.constEnd(); ++it)
		updateChatWindowActions(ARichTextEditor, *it);

	QList<IMessageNormalWindow *> normalWindows = FMessageWidgets->normalWindows();
	for (QList<IMessageNormalWindow *>::ConstIterator it = normalWindows.constBegin(); it!=normalWindows.constEnd(); ++it)
		updateNormalWindowActions(ARichTextEditor, *it);
}

void XhtmlIm::fixHtml(QString &AHtmlCode)
{
	for (int first=AHtmlCode.indexOf("<br "); first!=-1; first=AHtmlCode.indexOf("<br ", first+4))
		AHtmlCode.remove(first+3, AHtmlCode.indexOf(">", first+4)-first-3);
}

void XhtmlIm::onNormalWindowCreated(IMessageNormalWindow *AWindow)
{
	if(isSupported(AWindow->streamJid(), AWindow->contactJid()))
	{
		if (AWindow->mode()==IMessageNormalWindow::WriteMode) // Only for sending!!!
			updateNormalWindowActions(Options::node(OPV_XHTML_EDITORTOOLBAR).value().toBool(), AWindow);
		else
			connect(AWindow->viewWidget()->instance(), SIGNAL(viewContextMenu(QPoint, Menu *)),
													   SLOT(onViewContextMenu(QPoint, Menu*)));
	}
}

void XhtmlIm::onAddressChanged(const Jid &AStreamBefore, const Jid &AContactBefore)
{
	Q_UNUSED(AStreamBefore)
	Q_UNUSED(AContactBefore)

	IMessageAddress *address=qobject_cast<IMessageAddress *>(sender());
	IMessageChatWindow *window=FMessageWidgets->findChatWindow(address->streamJid(), address->contactJid());
	if (window)
		updateChatWindowActions(Options::node(OPV_XHTML_EDITORTOOLBAR).value().toBool(), window);
}

void XhtmlIm::onRichTextEditorToggled(bool AChecked)
{
	Options::node(OPV_XHTML_EDITORTOOLBAR).setValue(AChecked);
}

void XhtmlIm::onEditWidgetCreated(IMessageEditWidget *AWidget)
{
	QTextEdit *textEdit = AWidget->textEdit();
	textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(AWidget->instance(),SIGNAL(contextMenuRequested(const QPoint &, Menu *)),SLOT(onEditWidgetContextMenuRequested(const QPoint &, Menu *)));
}

void XhtmlIm::onEditWidgetContextMenuRequested(const QPoint &APosition, Menu *AMenu)
{
	FCurrentMessageEditWidget = qobject_cast<IMessageEditWidget *>(sender());
	qDebug() << "IMessageWindow=" << FCurrentMessageEditWidget->messageWindow();
	qDebug() << "MessageWindow Widget=" << FCurrentMessageEditWidget->messageWindow()->instance();
	qDebug() << "IMessageChatWindow=" << qobject_cast<IMessageChatWindow *>(FCurrentMessageEditWidget->messageWindow()->instance());
	bool chatWindow(qobject_cast<IMessageChatWindow *>(FCurrentMessageEditWidget->messageWindow()->instance()));
	if (FCurrentMessageEditWidget)
	{
		if (true)
		{
			QTextCursor cursor = FCurrentMessageEditWidget->textEdit()->cursorForPosition(APosition);
			FCurrentCursorPosition = cursor.atEnd()?-1:cursor.position();
			if (FCurrentCursorPosition != -1)
				cursor.setPosition(FCurrentCursorPosition);
			QTextCharFormat charFormat = cursor.charFormat();

			Menu *menu = new Menu(AMenu);
			menu->setTitle(tr("Format"));
			AMenu->addAction(menu->menuAction(),AG_XHTMLIM_FORMATTING);
//			QActionGroup *group = new QActionGroup(menu);

			Action *font=new Action(menu);
			font->setIcon(QIcon::fromTheme("format-text-font", FIconStorage->getIcon(XHI_FORMAT_RICH)));
			font->setText(tr("Font"));
			font->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FONT);
			font->setPriority(QAction::LowPriority);
			connect(font, SIGNAL(triggered()), this, SLOT(onSelectFont()));
			font->setCheckable(false);
			menu->addAction(font, AG_XHTMLIM_FONT);

			Action *underline=new Action(menu);
			underline->setIcon(QIcon::fromTheme("format-text-underline", FIconStorage->getIcon(XHI_TEXT_UNDERLINE)));
			underline->setText(tr("Underline"));
			underline->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_UNDERLINE);
			underline->setData(ADR_DECORATION_TYPE, DT_UNDERLINE);
			connect(underline, SIGNAL(triggered(bool)), this, SLOT(onSelectDecoration(bool)));
			underline->setCheckable(true);
			underline->setChecked(charFormat.fontUnderline());
			menu->addAction(underline, AG_XHTMLIM_FONT);

			Action *overline=new Action(menu);
			overline->setIcon(QIcon::fromTheme("format-text-overline", FIconStorage->getIcon(XHI_TEXT_OVERLINE)));
			overline->setText(tr("Overline"));
			overline->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_OVERLINE);
			overline->setData(ADR_DECORATION_TYPE, DT_OVERLINE);
			connect(overline, SIGNAL(triggered(bool)), this, SLOT(onSelectDecoration(bool)));
			overline->setCheckable(true);
			overline->setChecked(charFormat.fontOverline());
			menu->addAction(overline, AG_XHTMLIM_FONT);

			Action *strikeout=new Action(menu);
			strikeout->setIcon(QIcon::fromTheme("format-text-strikethrough", FIconStorage->getIcon(XHI_TEXT_STRIKEOUT)));
			strikeout->setText(tr("Strikethrough"));
			strikeout->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_STRIKEOUT);
			strikeout->setData(ADR_DECORATION_TYPE, DT_STRIKEOUT);
			connect(strikeout, SIGNAL(triggered(bool)), this, SLOT(onSelectDecoration(bool)));
			strikeout->setCheckable(true);
			strikeout->setChecked(charFormat.fontStrikeOut());
			menu->addAction(strikeout, AG_XHTMLIM_FONT);

			//  Code
			Action *code=new Action(menu);
			code->setIcon(QIcon::fromTheme("format-text-code", FIconStorage->getIcon(XHI_CODE)));
			code->setText(tr("Code"));
			code->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CODE);
			code->setPriority(QAction::LowPriority);
			connect(code, SIGNAL(toggled(bool)), SLOT(onTextCode(bool)));
			code->setCheckable(true);
			code->setChecked(getCursor().charFormat().fontFixedPitch());
			menu->addAction(code, AG_XHTMLIM_FONT);

			// Color
			Menu *color = new Menu(menu);
			color->setTitle(tr("Color"));
//			special->menuAction()->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NBSP);
//			special->menuAction()->setData(ADR_SPECIAL_SYMBOL, QChar::Nbsp);

			Action *foregroundColor = new Action(menu);
			foregroundColor->setText(tr("Foreground"));
			foregroundColor->setData(ADR_COLOR_TYPE, CT_FOREGROUND);
			connect(foregroundColor, SIGNAL(triggered()), SLOT(onColor()));
			color->addAction(foregroundColor);

			Action *backgroundColor = new Action(menu);
			backgroundColor->setText(tr("Background"));
			backgroundColor->setData(ADR_COLOR_TYPE, CT_BACKGROUND);
			connect(backgroundColor, SIGNAL(triggered()), SLOT(onColor()));
			color->addAction(backgroundColor);

			menu->addAction(color->menuAction(), AG_XHTMLIM_FONT);
			//  *** Special options ***
			//  Insert link
			Action *insertLink=new Action(menu);
			insertLink->setIcon(QIcon::fromTheme("insert-link",IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK)));
			insertLink->setText(tr("Insert link"));
			insertLink->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTLINK);
			insertLink->setCheckable(true);
			insertLink->setChecked(charFormat.isAnchor());
			connect(insertLink, SIGNAL(triggered()), SLOT(onInsertLink()));
			menu->addAction(insertLink, AG_XHTMLIM_INSERT);

			//  Insert image
			Action *insertImage=new Action(menu);
			insertImage->setIcon(QIcon::fromTheme("insert-image",FIconStorage->getIcon(XHI_INSERT_IMAGE)));
			insertImage->setText(tr("Insert image"));
			insertImage->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGE);
			insertImage->setPriority(QAction::LowPriority);
			connect(insertImage, SIGNAL(triggered()), SLOT(onInsertImage()));
			insertImage->setCheckable(true);
			insertImage->setChecked(charFormat.isImageFormat());
			menu->addAction(insertImage, AG_XHTMLIM_INSERT);

			//  Set tool tip
			Action *setToolTip=new Action(menu);
			setToolTip->setIcon(QIcon::fromTheme("set-tooltip", FIconStorage->getIcon(XHI_SET_TOOLTIP)));
			setToolTip->setText(tr("Set tool tip"));
			setToolTip->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_SETTOOLTIP);
			setToolTip->setPriority(QAction::LowPriority);
			connect(setToolTip, SIGNAL(triggered()), SLOT(onSetToolTip()));
			setToolTip->setCheckable(true);
			setToolTip->setChecked(charFormat.hasProperty(QTextFormat::TextToolTip));
			menu->addAction(setToolTip, AG_XHTMLIM_INSERT);


			// Special formatting
			Menu *special = new Menu(menu);
			special->setTitle(tr("Insert special symbol"));
			special->menuAction()->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NBSP);
			special->menuAction()->setData(ADR_SPECIAL_SYMBOL, QChar::Nbsp);
			connect(special->menuAction(), SIGNAL(triggered()), SLOT(onInsertSpecial()));

			QActionGroup *group=new QActionGroup(special);

			Action *action = new Action(group);
			action->setText(tr("Non-breaking space"));
			action->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NBSP);
			action->setData(ADR_SPECIAL_SYMBOL, QChar::Nbsp);
			action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNBSP);
			action->setCheckable(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertSpecial()));
			special->addAction(action);

			action = new Action(group);
			action->setText(tr("New line"));
			action->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NEWLINE);
			action->setData(ADR_SPECIAL_SYMBOL, QChar::LineSeparator);
			action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNEWLINE);
			action->setCheckable(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertSpecial()));
			special->addAction(action);
			menu->addAction(special->menuAction(), AG_XHTMLIM_INSERT);

			Action *removeFormat=new Action(menu);
			removeFormat->setIcon(QIcon::fromTheme("format-text-clear", FIconStorage->getIcon(XHI_FORMAT_CLEAR)));
			removeFormat->setText(tr("Remove format"));
			removeFormat->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATREMOVE);
			removeFormat->setPriority(QAction::LowPriority);
			connect(removeFormat, SIGNAL(triggered()), this, SLOT(onRemoveFormat()));
			removeFormat->setCheckable(false);
			menu->addAction(removeFormat, AG_XHTMLIM_SPECIAL);

			if (chatWindow)
			{
				Action *formatAutoReset = new Action(this);
				formatAutoReset->setIcon(QIcon::fromTheme("format-rich-text", FIconStorage->getIcon(XHI_FORMAT_PLAIN)));
				formatAutoReset->setText(tr("Reset formatting on message send"));
				formatAutoReset->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATAUTORESET);
				formatAutoReset->setPriority(QAction::LowPriority);
				connect(formatAutoReset, SIGNAL(toggled(bool)), SLOT(onResetFormat(bool)));
				formatAutoReset->setCheckable(true);
				formatAutoReset->setChecked(Options::node(OPV_XHTML_FORMATAUTORESET).value().toBool());
				menu->addAction(formatAutoReset, AG_XHTMLIM_SPECIAL);
			}

//			menu->setEnabled(!menu->isEmpty());
		}
	}
}

void XhtmlIm::onResetFormat(bool AStatus)
{
	Options::node(OPV_XHTML_FORMATAUTORESET).setValue(AStatus);
}

void XhtmlIm::onRemoveFormat()
{
	clearFormatOnWordOrSelection();
}

void XhtmlIm::onSelectFont()
{
	bool ok;
	QTextCursor cursor = getCursor();
	QFont font = QFontDialog::getFont(&ok, cursor.charFormat().font(), FCurrentMessageEditWidget->textEdit()->window());
	if (ok)
	{
		QTextCharFormat charFormat;
		charFormat.setFont(font);
		mergeFormatOnWordOrSelection(cursor, charFormat);
	}
}

void XhtmlIm::onSelectDecoration(bool ASelected)
{
	Action *action = qobject_cast<Action *>(sender());
	QTextCursor cursor = getCursor();
	QTextCharFormat charFormat = cursor.charFormat();
	switch (action->data(ADR_DECORATION_TYPE).toInt())
	{
		case DT_OVERLINE:
			charFormat.setFontOverline(ASelected);
			break;
		case DT_UNDERLINE:
			charFormat.setFontUnderline(ASelected);
			break;
		case DT_STRIKEOUT:
			charFormat.setFontStrikeOut(ASelected);
			break;
	}
	mergeFormatOnWordOrSelection(cursor, charFormat);
}

void XhtmlIm::onInsertLink()
{
	QTextCursor cursor = getCursor();

	QTextCharFormat charFmtCurrent=cursor.charFormat();

	if (!cursor.hasSelection())
	{
		if (charFmtCurrent.isAnchor())
		{
			QTextBlock block=cursor.block();
			for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
			{
				QTextFragment currentFragment = it.fragment();
				if (currentFragment.isValid())
				{
					if (currentFragment.contains(cursor.position()))
					{
						cursor.setPosition(currentFragment.position());
						cursor.setPosition(currentFragment.position()+currentFragment.length(), QTextCursor::KeepAnchor);
						break;
					}
				}
			}
		}
		else
			cursor.select(QTextCursor::WordUnderCursor);
	}

	bool needsToBeInserted=(cursor.selection().isEmpty());

	Action *action=qobject_cast<Action *>(sender());

	AddLink *addLink = new AddLink(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK),
								   QUrl::fromEncoded(charFmtCurrent.anchorHref().toLatin1()), cursor.selectedText(), action->parentWidget()->window());

	switch (addLink->exec())
	{
		case AddLink::Add:
		{
			QTextCharFormat charFmt=charFmtCurrent;
			charFmt.setAnchor(true);
			charFmt.setAnchorHref(addLink->url().toEncoded());
			charFmt.setFontUnderline(true);
			charFmt.setForeground(QBrush(Qt::blue));
			cursor.beginEditBlock();
			if (needsToBeInserted)
			{
				cursor.insertText(addLink->description(), charFmt);
				cursor.insertText(" ", charFmtCurrent);
			}
			else
				cursor.mergeCharFormat(charFmt);
			cursor.endEditBlock();
			break;
		}

		case AddLink::Remove:
		{
			QTextCharFormat charFmt;
			cursor.beginEditBlock();
			if (cursor.hasSelection())
			{
				charFmt.setAnchor(false);
				charFmt.setAnchorHref(QString());
				charFmt.setAnchorName(QString());
				cursor.mergeCharFormat(charFmt);
			}
			else
			{
				charFmt = charFmtCurrent;
				charFmt.clearProperty(QTextFormat::AnchorHref);
				charFmt.clearProperty(QTextFormat::AnchorName);
				charFmt.clearProperty(QTextFormat::IsAnchor);
				cursor.setCharFormat(charFmt);
			}
			cursor.endEditBlock();
			break;
		}
	}
	addLink->deleteLater();
}

void XhtmlIm::onInsertImage()
{
	QUrl        imageUrl;
	QByteArray  imageData;
	QSize       size;
	QString     alt;
	QTextCursor cursor = getCursor();
	QTextCharFormat charFmtCurrent=cursor.charFormat();

	bool supportBoB=FBitsOfBinary && FBitsOfBinary->isSupported(FCurrentMessageEditWidget->messageWindow()->streamJid(), FCurrentMessageEditWidget->messageWindow()->contactJid());

	if (!cursor.hasSelection())
		if (charFmtCurrent.isImageFormat())
		{
			QTextImageFormat imageFormat=charFmtCurrent.toImageFormat();
			cursor.select(QTextCursor::WordUnderCursor);
			imageUrl = QUrl::fromEncoded(imageFormat.name().toLatin1());
			imageData=FCurrentMessageEditWidget->document()->resource(QTextDocument::ImageResource, imageUrl).toByteArray();
			size.setWidth(imageFormat.width());
			size.setHeight(imageFormat.height());
			alt=imageFormat.property(XmlTextDocumentParser::ImageAlternativeText).toString();
		}

	Action *action=qobject_cast<Action *>(sender());
	InsertImage *inserImage = new InsertImage(this, FNetworkAccessManager, imageData, imageUrl, size, alt, action->parentWidget()->window());

	inserImage->setWindowIcon(FIconStorage->getIcon(XHI_INSERT_IMAGE));
	if(!supportBoB)
		inserImage->ui->pbBrowse->hide();
	if(inserImage->exec() == QDialog::Accepted)
	{
		if(!inserImage->getUrl().isEmpty())
		{
			cursor.beginEditBlock();
			QTextImageFormat imageFormat;
			QString          alt=inserImage->getAlternativeText();
			if (!alt.isEmpty())
				imageFormat.setProperty(XmlTextDocumentParser::ImageAlternativeText, alt);
			if (!inserImage->physResize())
			{
				if(inserImage->newHeight()!=inserImage->originalHeight())
					imageFormat.setHeight(inserImage->newHeight());
				if(inserImage->newWidth()!=inserImage->originalWidth())
					imageFormat.setWidth(inserImage->newWidth());
			}
			if(inserImage->isRemote())
			{
				QUrl url=inserImage->getUrl();
				imageFormat.setName(url.toEncoded());
				cursor.document()->addResource(QTextDocument::ImageResource, url, inserImage->getImageData());
				cursor.insertImage(imageFormat);
			}
			else
				if(supportBoB)
				{
					QByteArray imageData=inserImage->getImageData();
					QString contentId=FBitsOfBinary->contentIdentifier(imageData);
					QString uri=QString("cid:").append(contentId);
					imageFormat.setName(uri);
					imageFormat.setProperty(XhtmlIm::PMaxAge, inserImage->getMaxAge());
					imageFormat.setProperty(XhtmlIm::PMimeType, inserImage->getFileType());
					imageFormat.setProperty(XhtmlIm::PEmbed, inserImage->embed());
					cursor.document()->addResource(QTextDocument::ImageResource, QUrl(uri), imageData);
					cursor.insertImage(imageFormat);
				}
			cursor.endEditBlock();
		}
	}
	inserImage->deleteLater();
}

void XhtmlIm::onSetToolTip()
{
	QTextCursor cursor = getCursor();
	QTextCharFormat charFormat=cursor.charFormat();
	if (!charFormat.hasProperty(QTextFormat::TextToolTip) &&
		!cursor.hasSelection())
		cursor.select(QTextCursor::WordUnderCursor);

	Action *action=qobject_cast<Action *>(sender());
	int toolTipType = charFormat.intProperty(XmlTextDocumentParser::ToolTipType);

	SetToolTip *setToolTip = new SetToolTip(toolTipType, charFormat.toolTip(), action->parentWidget()->window());

	if(setToolTip->exec() == QDialog::Accepted)
	{
		cursor.beginEditBlock();
		if (setToolTip->toolTipText().isEmpty())	// Remove tooltip
		{
			if (cursor.hasSelection())
			{
				charFormat.setProperty(QTextFormat::TextToolTip, QVariant());
				charFormat.setProperty(XmlTextDocumentParser::ToolTipType, XmlTextDocumentParser::None);
				if (charFormat.underlineStyle()==QTextCharFormat::DotLine && charFormat.underlineColor()==Qt::red)
				{
					charFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
					charFormat.setUnderlineColor(QColor());
				}
				cursor.mergeCharFormat(charFormat);
			}
			else
			{
				charFormat.clearProperty(QTextFormat::TextToolTip);
				charFormat.clearProperty(XmlTextDocumentParser::ToolTipType);
				if (charFormat.underlineStyle()==QTextCharFormat::DotLine && charFormat.underlineColor()==Qt::red)
				{
					charFormat.clearProperty(QTextFormat::TextUnderlineStyle);
					charFormat.clearProperty(QTextFormat::TextUnderlineColor);
				}
				cursor.setCharFormat(charFormat);
			}
		}
		else
		{
			QTextCharFormat format;
			format.setProperty(QTextFormat::TextToolTip, setToolTip->toolTipText());
			if (setToolTip->type()!=SetToolTip::None)
			{
				format.setUnderlineStyle(QTextCharFormat::DotLine);
				format.setUnderlineColor(Qt::red);
			}
			else
				if (charFormat.underlineStyle()==QTextCharFormat::DotLine &&
					charFormat.underlineColor()==Qt::red)
				{
					format.setUnderlineStyle(QTextCharFormat::NoUnderline);
					format.setUnderlineColor(QColor());
				}
			format.setProperty(XmlTextDocumentParser::ToolTipType, setToolTip->type());
			cursor.mergeCharFormat(format);
		}
		cursor.endEditBlock();
	}
	setToolTip->deleteLater();
}

void XhtmlIm::onInsertSpecial()
{
	Action *action = qobject_cast<Action *>(sender());
	QChar specialSybmol = (QChar)(action->data(ADR_SPECIAL_SYMBOL).toInt());
	Menu *special = qobject_cast<Menu *>(action->parentWidget());
	special->menuAction()->setData(ADR_SPECIAL_SYMBOL, specialSybmol);
	special->menuAction()->setIcon(action->icon());
	QTextCursor cursor = getCursor(false, false);
	cursor.beginEditBlock();
	cursor.insertText(specialSybmol);
	cursor.endEditBlock();
}

void XhtmlIm::onTextCode(bool AChecked)
{
	QTextCursor cursor = getCursor();
	cursor.beginEditBlock();
	if (AChecked)
	{
		QTextCharFormat charFormat;
		charFormat.setFontFixedPitch(true);
		mergeFormatOnWordOrSelection(cursor, charFormat);
	}
	else
	{
		QTextCharFormat charFormat = cursor.charFormat();
		charFormat.clearProperty(QTextFormat::FontFixedPitch);
		cursor.setCharFormat(charFormat);
	}
	cursor.endEditBlock();
}

void XhtmlIm::onColor()
{
	QTextCursor cursor = getCursor();
	QTextCharFormat charFormat = cursor.charFormat();
	Action *action = qobject_cast<Action *>(sender());
	int type = action->data(ADR_COLOR_TYPE).toInt();
	QColor color = QColorDialog::getColor((type==CT_FOREGROUND?charFormat.foreground():charFormat.background()).color(), action->parentWidget()->window());
	if (!color.isValid())
		return;
	QTextCharFormat newCharFormat;
	if (type==CT_FOREGROUND)
		newCharFormat.setForeground(color);
	else
		newCharFormat.setBackground(color);
	mergeFormatOnWordOrSelection(cursor, newCharFormat);
}

QTextCursor XhtmlIm::getCursor(bool ASelectWholeDocument, bool ASelect)
{
	QTextCursor cursor = FCurrentMessageEditWidget->textEdit()->textCursor();
	if (FCurrentCursorPosition != -1)
	{
		if (FCurrentCursorPosition < cursor.selectionStart() || FCurrentCursorPosition > cursor.selectionEnd())
			cursor.setPosition(FCurrentCursorPosition);
		if (!cursor.hasSelection() && ASelect)
			cursor.select(QTextCursor::WordUnderCursor);
	}
	else
		if (ASelectWholeDocument)
			cursor.select(QTextCursor::Document);

	return cursor;
}

void XhtmlIm::mergeFormatOnWordOrSelection(QTextCursor ACursor, const QTextCharFormat &AFormat)
{
	if (ACursor.hasSelection())
	{
		ACursor.beginEditBlock();
		ACursor.mergeCharFormat(AFormat);
		ACursor.endEditBlock();
	}
	else
		FCurrentMessageEditWidget->textEdit()->mergeCurrentCharFormat(AFormat);
}


void XhtmlIm::clearFormatOnWordOrSelection()
{
	QTextCharFormat emptyCharFormat;
	QTextCursor cursor = getCursor(true);
	cursor.beginEditBlock();
	cursor.setCharFormat(emptyCharFormat);
	cursor.endEditBlock();
	FCurrentMessageEditWidget->textEdit()->setCurrentCharFormat(emptyCharFormat);
}

void XhtmlIm::onChatWindowCreated(IMessageChatWindow *AWindow)
{
	updateChatWindowActions(Options::node(OPV_XHTML_EDITORTOOLBAR).value().toBool(), AWindow);
	connect(AWindow->address()->instance(), SIGNAL(addressChanged(Jid, Jid)), SLOT(onAddressChanged(Jid,Jid)));
	connect(AWindow->viewWidget()->instance(), SIGNAL(viewContextMenu(QPoint, Menu *)),
											   SLOT(onViewContextMenu(QPoint, Menu*)));

}

void XhtmlIm::onViewContextMenu(const QPoint &APosition, Menu *AMenu)
{
	if (IMessageViewWidget *viewWidget=qobject_cast<IMessageViewWidget *>(sender()))
	{
		QTextCharFormat format=viewWidget->textFormatAt(APosition);
		if (format.isImageFormat())
		{
			QUrl imageUrl=QUrl::fromEncoded(format.toImageFormat().name().toLatin1());
			if (!FValidSchemes.contains(imageUrl.scheme())) // Invalid scheme - assuming local file!
				imageUrl=QUrl::fromLocalFile(imageUrl.toString());

			QActionGroup *group = new QActionGroup(AMenu);
			Action *action=new Action(AMenu);
			action->setText(tr("Save image..."));
			action->setData(Action::DR_Parametr1, imageUrl);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onImageSave()));
			AMenu->addAction(action);

			action=new Action(AMenu);
			action->setText(tr("Open image in the system"));
			action->setData(Action::DR_Parametr1, imageUrl);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onImageOpen()));
			AMenu->addAction(action);

			action=new Action(AMenu);
			QString html = viewWidget->textFragmentAt(APosition).toHtml();
			action->setText(tr("Copy image"));
			action->setData(Action::DR_Parametr1, imageUrl);
			action->setData(Action::DR_Parametr2, viewWidget->imageAt(APosition));
			action->setData(Action::DR_Parametr3, html);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onImageCopy()));
			AMenu->addAction(action);

			if (imageUrl.scheme()=="ftp" ||
				imageUrl.scheme()=="http" ||
				imageUrl.scheme()=="https" )
			{
				action=new Action(AMenu);
				action->setText(tr("Copy image link"));
				action->setData(Action::DR_Parametr1, imageUrl);
				action->setActionGroup(group);
				connect(action, SIGNAL(triggered()), SLOT(onImageCopyLink()));
				AMenu->addAction(action);
			}
		}
	}
}

void XhtmlIm::onImageCopy()
{
	if (Action *action=qobject_cast<Action *>(sender()))
	{
		QMimeData *mimeData = new QMimeData();

		QList<QUrl> urls;
		urls.append(action->data(Action::DR_Parametr1).toUrl());
		mimeData->setUrls(urls);

		mimeData->setImageData(action->data(Action::DR_Parametr2).value<QImage>());

		QString html     = action->data(Action::DR_Parametr3).toString();
		int first = html.indexOf("<!--StartFragment-->")+20;
		int last =  html.indexOf("<!--EndFragment-->");
		mimeData->setHtml(html.mid(first, last-first));

		QApplication::clipboard()->setMimeData(mimeData);
	}
}

void XhtmlIm::onImageCopyLink()
{
	if (Action *action=qobject_cast<Action *>(sender()))
	{
		QUrl imageUrl=action->data(Action::DR_Parametr1).toUrl();
		QApplication::clipboard()->setText(imageUrl.toString());
	}
}

void XhtmlIm::onImageSave()
{
	if (Action *action=qobject_cast<Action *>(sender()))
	{
		QUrl imageUrl=action->data(Action::DR_Parametr1).toUrl();
		QString path=imageUrl.path();

		int index1=path.lastIndexOf('\\');
		int index2=path.lastIndexOf('/');
		int index=index1>index2?index1:index2;
		if (index<0)
			index=0;
		QString fileName = path.right(path.length()-1-index);
		fileName = QFileDialog::getSaveFileName(NULL, tr("Please, choose image file"), QDir(Options::node(OPV_XHTML_IMAGESAVEDIRECTORY).value().toString()).absoluteFilePath(fileName));
		if (!fileName.isNull())
		{
			Options::node(OPV_XHTML_IMAGESAVEDIRECTORY).setValue(QFileInfo(fileName).absolutePath());
			if (FNetworkAccessManager)
				new SaveQuery(FNetworkAccessManager, fileName, imageUrl.toString());
		}
	}
}

void XhtmlIm::onImageOpen()
{
	if (Action *action=qobject_cast<Action *>(sender()))
	{
		QUrl url=action->data(Action::DR_Parametr1).toUrl();
		new ImageOpenThread(url, this);
	}
}

void XhtmlIm::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(ALang)
	if (AOrder == MWO_XHTML_M2T)
	{
		QDomElement body=AMessage.stanza().firstElement("html", NS_XHTML_IM).firstChildElement("body");
		if (!body.isNull())
		{
			QDomDocument doc;
			QDomNode imported=doc.importNode(body, true);
			doc.appendChild(imported);
			ADocument->clear();
			XmlTextDocumentParser::xmlToText(ADocument, doc);
			if (FBitsOfBinary)
			{
				QVector<QTextFormat> formats=ADocument->allFormats();
				for (QVector<QTextFormat>::const_iterator it=formats.constBegin(); it!=formats.constEnd(); it++)
					if ((*it).isImageFormat())
					{
						QUrl url((*it).toImageFormat().name());
						if (url.scheme()=="cid")
						{
							QString cid=url.path();
							if (!FBitsOfBinary->hasBinary(cid))
								FBitsOfBinary->loadBinary(cid, AMessage.to(), AMessage.from());
						}
					}
			}

		}
	}
}

void XhtmlIm::writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(ALang)
	if (AOrder == MWO_XHTML_T2M)
		if (!ADocument->isEmpty())  // Document is not empty
		{
			QVector<QTextFormat>formats=ADocument->allFormats();
			if (Options::node(OPV_XHTML_NORICHTEXT).value().toBool() &&
				formats.count()==3 && formats[0].type()==QTextFormat::CharFormat  && formats[0].propertyCount()==0 &&
									  formats[1].type()==QTextFormat::BlockFormat && formats[1].propertyCount()==0 &&
									  formats[2].type()==QTextFormat::FrameFormat && formats[2].propertyCount()==7)
				return; // No formatting!

			if (FBitsOfBinary)
				for (QTextBlock block=ADocument->begin(); block!=ADocument->end(); block=block.next())
					for (QTextBlock::iterator it=block.begin(); it!=block.end(); it++)
					{
						QTextFragment fragment=it.fragment();
						QTextCharFormat format=fragment.charFormat();
						if (format.isImageFormat())
						{
							QTextImageFormat imageFormat=format.toImageFormat();
							QUrl url(imageFormat.name());
							if (url.scheme()=="cid" || url.scheme()=="file")
							{
								QByteArray data=ADocument->resource(QTextDocument::ImageResource, url).toByteArray();
								QString type=format.property(PMimeType).toString();
								quint64 maxAge=format.property(PMaxAge).toLongLong();

								QString cid;
								if (url.scheme()=="cid")
									cid=url.path();
								else    // file
								{
									cid=FBitsOfBinary->contentIdentifier(data);
									imageFormat.setName(QString("cid:").append(cid));
									QTextCursor cursor(ADocument);
									// Select contents
									cursor.setPosition(fragment.position(), QTextCursor::MoveAnchor);
									cursor.setPosition(fragment.position()+fragment.length(), QTextCursor::KeepAnchor);
									cursor.insertImage(imageFormat);
								}
								FBitsOfBinary->saveBinary(cid, type, data, maxAge);
								if(format.property(PEmbed).toBool())
									FBitsOfBinary->saveBinary(cid, type, data, maxAge, AMessage.stanza());
							}
						}
					}
			QDomElement  html=AMessage.stanza().document().createElementNS(NS_XHTML_IM, "html");
			QDomElement  body=AMessage.stanza().document().createElementNS(NS_XHTML, "body");
			html.appendChild(body);
			AMessage.stanza().element().appendChild(html);
			XmlTextDocumentParser::textToXml(body, *ADocument);
			// Set message body
			AMessage.setBody(XmlTextDocumentParser::textToPlainText(*ADocument));
		}
}

bool XhtmlIm::messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData)
{
	Q_UNUSED(AOrder);
	Q_UNUSED(AWidget);
	Q_UNUSED(AData);
	return false;
}

bool XhtmlIm::messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData)
{
	Q_UNUSED(AWidget);

	if (AOrder == ECHO_XHTML_COPY_INSERT)
	{
		if (AWidget->isRichTextEnabled())
		{
			if (AData->hasHtml() || (AData->hasImage() && FBitsOfBinary && FBitsOfBinary->isSupported(AWidget->messageWindow()->streamJid(), AWidget->messageWindow()->contactJid())))
				return true;
			else if (AData->hasUrls())
			{
				QList<QUrl> urls=AData->urls();
				for (QList<QUrl>::const_iterator it=urls.constBegin(); it!=urls.constEnd(); it++)
				{
					if (it->scheme() == "ftp" ||
						it->scheme() == "http" ||
						it->scheme() == "https" )
						return true;

					QNetworkReply *reply=FNetworkAccessManager->get(QNetworkRequest(*it));
					if (reply)
					{
						QByteArray format = QImageReader::imageFormat(reply);
						reply->deleteLater();
						return !format.isNull();
						return true;
					}
				}

			}
		}
	}
	return false;
}

bool XhtmlIm::messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument)
{
	Q_UNUSED(AWidget);

	if (AOrder==ECHO_XHTML_COPY_INSERT &&
		messageEditContentsCanInsert(AOrder,AWidget,AData))
	{
		if (AData->hasImage())
		{
			QImage image = AData->imageData().value<QImage>();
			if (AData->hasHtml())
			{
				QDomDocument doc;
				doc.setContent(AData->html());
				QDomElement root = doc.documentElement();
				if (root.tagName()=="img" && root.hasAttribute("src"))
				{
					QTextImageFormat imageFormat;
					QUrl url = QUrl::fromEncoded(root.attribute("src").toLatin1());
					imageFormat.setName(url.toString());
					if (root.hasAttribute("alt"))
						imageFormat.setProperty(XmlTextDocumentParser::ImageAlternativeText, root.attribute("alt"));
					if (root.hasAttribute("title"))
						imageFormat.setToolTip(root.attribute("title"));
					AWidget->textEdit()->document()->addResource(QTextDocument::ImageResource, url, AData->imageData().value<QImage>());
					QTextCursor(ADocument).insertImage(imageFormat);
					return true;
				}
			}

			if (AData->hasUrls() && AData->urls().size()==1)
			{
				QUrl url = AData->urls()[0];
				QString alt;
				QTextImageFormat imageFormat;
				imageFormat.setName(url.toString());
				if (!alt.isEmpty())
					imageFormat.setProperty(XmlTextDocumentParser::ImageAlternativeText, alt);
				AWidget->textEdit()->document()->addResource(QTextDocument::ImageResource, url, AData->imageData().value<QImage>());
				QTextCursor(ADocument).insertImage(imageFormat);
				return true;
			}

			if (FBitsOfBinary && FBitsOfBinary->isSupported(AWidget->messageWindow()->streamJid(), AWidget->messageWindow()->contactJid()))
			{
				QString mimeType;
				QStringList formats=AData->formats();
				for (QStringList::const_iterator it=formats.constBegin(); it!=formats.constEnd(); it++)
					if ((*it).startsWith("image/"))
					{
						mimeType=*it;
						break;
					}

				QByteArray format;
				if (!mimeType.isEmpty())
				{
					QByteArray fmt(mimeType.mid(6).toLatin1().constData());
					if (QImageReader::supportedImageFormats().contains(fmt))
						format=fmt;
				}

				if (format.isEmpty())
				{
					format=Options::node(OPV_XHTML_DEFAULTIMAGEFORMAT).value().toByteArray();
					mimeType=QString("image/").append(format);
				}

				QTextImageFormat imageFormat;
				QImage image=AData->imageData().value<QImage>();
				QByteArray bytes;
				QBuffer buffer(&bytes);
				buffer.open(QIODevice::WriteOnly);
				if (image.save(&buffer, format)) // writes image into ba in PNG format
				{
					buffer.close();
					QString contentId=FBitsOfBinary->contentIdentifier(bytes);
					quint64 maxAge = Options::node(OPV_XHTML_MAXAGE).value().toLongLong();
					QString uri=QString("cid:").append(contentId);
					imageFormat.setName(uri);
					imageFormat.setProperty(PMaxAge, maxAge);
					imageFormat.setProperty(PMimeType, mimeType);
					imageFormat.setProperty(PEmbed, bytes.size()<= Options::node(OPV_XHTML_EMBEDSIZE).value().toInt());
					AWidget->textEdit()->document()->addResource(QTextDocument::ImageResource, QUrl(uri), bytes);
					QTextCursor(ADocument).insertImage(imageFormat);
					return true;
				}
			}
		}
		else if (AData->hasHtml())
		{
			QString html=AData->html();
			fixHtml(html);
			QTextCursor(ADocument).insertHtml(html);
			return true;
		}
	}
	return false;
}

bool XhtmlIm::messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded)
{
	Q_UNUSED(AOrder);
	Q_UNUSED(AWidget);
	Q_UNUSED(APosition);
	Q_UNUSED(ARemoved);
	Q_UNUSED(AAdded);
	return false;
}

void XhtmlIm::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_XHTML_EDITORTOOLBAR)
		updateMessageWindows(ANode.value().toBool());
}

void XhtmlIm::onBobUrlOpen(QUrl AUrl)
{
	QString cid=AUrl.path();
	if (FBitsOfBinary->hasBinary(cid))
	{
		QString     type;
		QByteArray  data;
		quint64     maxAge;
		if (FBitsOfBinary->loadBinary(cid, type, data, maxAge))
		{
			QDir temp = QDir::temp();
			if (temp.isReadable())
			{
				QString tmpFileName(cid);
				tmpFileName.append('.').append(type.mid(6));
				QFile tmpFile(temp.filePath(tmpFileName));
				if (!tmpFile.exists() || tmpFile.size()!=data.size())    // Exists already
				{
					tmpFile.open(QFile::WriteOnly);
					tmpFile.write(data);
					tmpFile.close();
				}
				QUrl url=QUrl::fromLocalFile(tmpFile.fileName());
				QDesktopServices::openUrl(url);
			}
		}
	}
}

void XhtmlIm::updateUnitsComboBox(QComboBox *AComboBox, int AValue)
{
	AComboBox->setItemText(0, tr("second(s)", "", AValue));
	AComboBox->setItemText(1, tr("minute(s)", "", AValue));
	AComboBox->setItemText(2, tr("hour(s)", "", AValue));
	AComboBox->setItemText(3, tr("day(s)", "", AValue));
	AComboBox->setItemText(4, tr("week(s)", "", AValue));
	AComboBox->setItemText(5, tr("month(s)", "", AValue));
	AComboBox->setItemText(6, tr("year(s)", "", AValue));
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_xhtmlim, XhtmlIm)
#endif

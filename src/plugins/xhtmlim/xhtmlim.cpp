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
#define ADR_CAPITALIZATION_TYPE Action::DR_Parametr1
#define ADR_SPECIAL_SYMBOL  Action::DR_Parametr1
#define ADR_COLOR_TYPE		Action::DR_Parametr1
#define ADR_INDENT			Action::DR_Parametr1
#define ADR_ALIGN_TYPE      Action::DR_Parametr1
#define ADR_LIST_TYPE       Action::DR_Parametr1
#define ADR_FORMATTING_TYPE Action::DR_Parametr1

#define DT_BOLD      1
#define DT_ITALIC    2
#define DT_UNDERLINE 3
#define DT_OVERLINE	 4
#define DT_STRIKEOUT 5

#define CPT_MIXED      0
#define CPT_SMALLCAPS  1
#define CPT_ALLUPPER   2
#define CPT_ALLLOWER   3
#define CPT_CAPITALIZE 4

#define CT_FOREGROUND 0
#define CT_BACKGROUND 1

#define INDENT_LESS	0
#define INDENT_MORE	1

#define FMT_NORMAL          0
#define FMT_HEADING1        1
#define FMT_HEADING2        2
#define FMT_HEADING3        3
#define FMT_HEADING4        4
#define FMT_HEADING5        5
#define FMT_HEADING6        6
#define FMT_PREFORMAT       7

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
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_OVERLINE, tr("Overline"), tr("Ctrl+Y", "Overline"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_STRIKEOUT, tr("Strikeout"), tr("Ctrl+S", "Strikeout"), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSMIXED, tr("Mixed case"), QString(), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSSMALL, tr("Small caps"), QString(), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSCAPITALIZE, tr("Capitalize"), QString(), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSALLUPPER, tr("All uppercase"), QString(), Shortcuts::WindowShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSALLLOWER, tr("All lowercase"), QString(), Shortcuts::WindowShortcut);
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
	Options::setDefaultValue(OPV_XHTML_EDITORMENU, true);
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
		if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
			widgets.insertMulti(OWO_XHTML_EDITORMENU, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_XHTML_EDITORMENU), tr("Allow rich text edit pop-up menu"), AParent));
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
	if (FCurrentMessageEditWidget)
	{
		if (Options::node(OPV_XHTML_EDITORMENU).value().toBool())
		{
			bool chatWindow(qobject_cast<IMessageChatWindow *>(FCurrentMessageEditWidget->messageWindow()->instance()));
			QTextCursor cursor = FCurrentMessageEditWidget->textEdit()->cursorForPosition(APosition);
			FCurrentCursorPosition = cursor.atEnd()?-1:cursor.position();
			if (FCurrentCursorPosition != -1)
				cursor.setPosition(FCurrentCursorPosition);
			QTextCharFormat charFormat = cursor.charFormat();

			Menu *menu = new Menu(AMenu);
			menu->setTitle(tr("Text format"));
			menu->setIcon(QIcon::fromTheme("format-text", FIconStorage->getIcon(XHI_FORMAT)));
			AMenu->addAction(menu->menuAction(),AG_XHTMLIM_FORMATTING);

			Action *font=new Action(menu);
			font->setIcon(QIcon::fromTheme("format-text-font", FIconStorage->getIcon(XHI_TEXT_FONT)));
			font->setText(tr("Font"));
			font->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FONT);
			font->setPriority(QAction::LowPriority);
			connect(font, SIGNAL(triggered(bool)), this, SLOT(onSelectFont()));
			font->setCheckable(false);
			menu->addAction(font, AG_XHTMLIM_FONT);

			// *** Style submenu ***
			Menu *style = new Menu(menu);
			style->setTitle(tr("Style"));
			style->setIcon(QIcon::fromTheme("format-text-style",FIconStorage->getIcon(XHI_TEXT_STYLE)));

			//  Bold
			Action *bold = new Action(style);
			bold->setIcon(QIcon::fromTheme("format-text-bold",FIconStorage->getIcon(XHI_TEXT_BOLD)));
			bold->setText(tr("Bold"));
			bold->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_BOLD);
			bold->setData(ADR_DECORATION_TYPE, DT_BOLD);
			bold->setCheckable(true);
			bold->setChecked(charFormat.fontWeight()>QFont::DemiBold);
			connect(bold, SIGNAL(triggered(bool)), SLOT(onSelectDecoration(bool)));
			style->addAction(bold, AG_XHTMLIM_FONT);

			//  Italic
			Action *italic = new Action(style);
			italic->setIcon(QIcon::fromTheme("format-text-italic",FIconStorage->getIcon(XHI_TEXT_ITALIC)));
			italic->setText(tr("Italic"));
			italic->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ITALIC);
			italic->setData(ADR_DECORATION_TYPE, DT_ITALIC);
			connect(italic, SIGNAL(triggered(bool)), SLOT(onSelectDecoration(bool)));
			italic->setCheckable(true);
			italic->setChecked(charFormat.fontItalic());
			style->addAction(italic, AG_XHTMLIM_FONT);

			Action *underline=new Action(style);
			underline->setIcon(QIcon::fromTheme("format-text-underline", FIconStorage->getIcon(XHI_TEXT_UNDERLINE)));
			underline->setText(tr("Underline"));
			underline->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_UNDERLINE);
			underline->setData(ADR_DECORATION_TYPE, DT_UNDERLINE);
			connect(underline, SIGNAL(triggered(bool)), this, SLOT(onSelectDecoration(bool)));
			underline->setCheckable(true);
			underline->setChecked(charFormat.fontUnderline());
			style->addAction(underline, AG_XHTMLIM_FONT);

			Action *overline=new Action(style);
			overline->setIcon(QIcon::fromTheme("format-text-overline", FIconStorage->getIcon(XHI_TEXT_OVERLINE)));
			overline->setText(tr("Overline"));
			overline->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_OVERLINE);
			overline->setData(ADR_DECORATION_TYPE, DT_OVERLINE);
			connect(overline, SIGNAL(triggered(bool)), this, SLOT(onSelectDecoration(bool)));
			overline->setCheckable(true);
			overline->setChecked(charFormat.fontOverline());
			style->addAction(overline, AG_XHTMLIM_FONT);

			Action *strikeout=new Action(style);
			strikeout->setIcon(QIcon::fromTheme("format-text-strikeout", FIconStorage->getIcon(XHI_TEXT_STRIKEOUT)));
			strikeout->setText(tr("Strikeout"));
			strikeout->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_STRIKEOUT);
			strikeout->setData(ADR_DECORATION_TYPE, DT_STRIKEOUT);
			connect(strikeout, SIGNAL(triggered(bool)), this, SLOT(onSelectDecoration(bool)));
			strikeout->setCheckable(true);
			strikeout->setChecked(charFormat.fontStrikeOut());
			style->addAction(strikeout, AG_XHTMLIM_FONT);

			menu->addAction(style->menuAction(), AG_XHTMLIM_FONT);

			// *** Capitalization submenu ***
			Menu *capitalization = new Menu(menu);
			capitalization->setTitle(tr("Capitalization"));
			capitalization->setIcon(QIcon::fromTheme("format-text-capitalization", FIconStorage->getIcon(XHI_CAPS_MIXED)));

			//  Mixed case
			Action *mixed = new Action(capitalization);
			mixed->setIcon(QIcon::fromTheme("format-text-capitalization-mixedcase",FIconStorage->getIcon(XHI_CAPS_MIXED)));
			mixed->setText(tr("Mixed case"));
			mixed->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSMIXED);
			mixed->setData(ADR_CAPITALIZATION_TYPE, CPT_MIXED);
			mixed->setCheckable(true);
			if (charFormat.fontCapitalization()==QFont::MixedCase)
			{
				capitalization->setIcon(mixed->icon());
				if (charFormat.hasProperty(QTextFormat::FontCapitalization))
					mixed->setChecked(true);
			}
			connect(mixed, SIGNAL(triggered()), SLOT(onSelectCapitalization()));
			capitalization->addAction(mixed, AG_XHTMLIM_FONT);

			//  Small caps
			Action *smallCaps = new Action(capitalization);
			smallCaps->setIcon(QIcon::fromTheme("format-text-capitalization-smallcaps",FIconStorage->getIcon(XHI_CAPS_SMALLCAPS)));
			smallCaps->setText(tr("Small caps"));
			smallCaps->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSSMALL);
			smallCaps->setData(ADR_CAPITALIZATION_TYPE, CPT_SMALLCAPS);
			smallCaps->setCheckable(true);
			if (charFormat.fontCapitalization()==QFont::SmallCaps)
			{
				capitalization->setIcon(smallCaps->icon());
				if (charFormat.hasProperty(QTextFormat::FontCapitalization))
					smallCaps->setChecked(true);
			}
			connect(smallCaps, SIGNAL(triggered()), SLOT(onSelectCapitalization()));
			capitalization->addAction(smallCaps, AG_XHTMLIM_FONT);

			//  All uppercase
			Action *allUppercase = new Action(capitalization);
			allUppercase->setIcon(QIcon::fromTheme("format-text-capitalization-alluppercase",FIconStorage->getIcon(XHI_CAPS_ALLUPPER)));
			allUppercase->setText(tr("All uppercase"));
			allUppercase->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSALLUPPER);
			allUppercase->setData(ADR_CAPITALIZATION_TYPE, CPT_ALLUPPER);
			allUppercase->setCheckable(true);
			if (charFormat.fontCapitalization()==QFont::AllUppercase)
			{
				capitalization->setIcon(allUppercase->icon());
				if (charFormat.hasProperty(QTextFormat::FontCapitalization))
					allUppercase->setChecked(true);
			}
			connect(allUppercase, SIGNAL(triggered()), SLOT(onSelectCapitalization()));
			capitalization->addAction(allUppercase, AG_XHTMLIM_FONT);

			//  All lowercase
			Action *allLowercase = new Action(capitalization);
			allLowercase->setIcon(QIcon::fromTheme("format-text-capitalization-alllowercase",FIconStorage->getIcon(XHI_CAPS_ALLLOWER)));
			allLowercase->setText(tr("All lowercase"));
			allLowercase->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSALLLOWER);
			allLowercase->setData(ADR_CAPITALIZATION_TYPE, CPT_ALLLOWER);
			allLowercase->setCheckable(true);
			if (charFormat.fontCapitalization()==QFont::AllLowercase)
			{
				capitalization->setIcon(allLowercase->icon());
				if (charFormat.hasProperty(QTextFormat::FontCapitalization))
					allLowercase->setChecked(true);
			}
			connect(allLowercase, SIGNAL(triggered()), SLOT(onSelectCapitalization()));
			capitalization->addAction(allLowercase, AG_XHTMLIM_FONT);

			//  All lowercase
			Action *capitalize = new Action(capitalization);
			capitalize->setIcon(QIcon::fromTheme("format-text-capitalization-capitalize",FIconStorage->getIcon(XHI_CAPS_CAPITALIZE)));
			capitalize->setText(tr("Capitalize"));
			capitalize->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CAPSCAPITALIZE);
			capitalize->setData(ADR_CAPITALIZATION_TYPE, CPT_CAPITALIZE);
			capitalize->setCheckable(true);
			if (charFormat.fontCapitalization()==QFont::Capitalize)
			{
				capitalization->setIcon(capitalize->icon());
				if (charFormat.hasProperty(QTextFormat::FontCapitalization))
					capitalize->setChecked(true);
			}
			connect(capitalize, SIGNAL(triggered()), SLOT(onSelectCapitalization()));
			capitalization->addAction(capitalize, AG_XHTMLIM_FONT);

			menu->addAction(capitalization->menuAction(), AG_XHTMLIM_FONT);

			//  Code
			Action *code=new Action(menu);
			code->setIcon(QIcon::fromTheme("format-text-code", FIconStorage->getIcon(XHI_CODE)));
			code->setText(tr("Code"));
			code->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CODE);
			code->setPriority(QAction::LowPriority);
			connect(code, SIGNAL(toggled(bool)), SLOT(onTextCode(bool)));
			code->setCheckable(true);
			code->setChecked(getCursor().charFormat().fontFamily()=="Courier New,courier");
			menu->addAction(code, AG_XHTMLIM_FONT);

			// Color
			Menu *color = new Menu(menu);
			color->setTitle(tr("Color"));
			color->menuAction()->setIcon(RSR_STORAGE_HTML, XHI_TEXT_COLOR);

			// Foreground
			Action *foregroundColor = new Action(menu);
			foregroundColor->setText(tr("Foreground"));
			foregroundColor->setData(ADR_COLOR_TYPE, CT_FOREGROUND);
			connect(foregroundColor, SIGNAL(triggered()), SLOT(onColor()));
			color->addAction(foregroundColor);

			// Background
			Action *backgroundColor = new Action(menu);
			backgroundColor->setText(tr("Background"));
			backgroundColor->setData(ADR_COLOR_TYPE, CT_BACKGROUND);
			connect(backgroundColor, SIGNAL(triggered()), SLOT(onColor()));
			color->addAction(backgroundColor);

			menu->addAction(color->menuAction(), AG_XHTMLIM_FONT);

			//  *** Insert ***
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

			// *** Special characters ***
			Menu *special = new Menu(menu);
			special->setTitle(tr("Insert special symbol"));
			special->menuAction()->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NBSP);
			special->menuAction()->setData(ADR_SPECIAL_SYMBOL, QChar::Nbsp);
			connect(special->menuAction(), SIGNAL(triggered()), SLOT(onInsertSpecial()));

			QActionGroup *group=new QActionGroup(special);
			// NBSP
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

			// New line
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

			//  *** Indentation ***
			// Indent
			Action *indentLess= new Action(this);
			indentLess->setIcon(QIcon::fromTheme("format-indent-less", FIconStorage->getIcon(XHI_OUTDENT)));
			indentLess->setText(tr("Decrease indent"));
			indentLess->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INDENTDECREASE);
			indentLess->setPriority(QAction::LowPriority);
			indentLess->setCheckable(false);
			indentLess->setData(ADR_INDENT, INDENT_LESS);
			connect(indentLess, SIGNAL(triggered()), this, SLOT(onIndentChange()));
			menu->addAction(indentLess, AG_XHTMLIM_INDENT);

			// Outndent
			Action *indentMore=new Action(this);
			indentMore->setIcon(QIcon::fromTheme("format-indent-more",FIconStorage->getIcon(XHI_INDENT)));
			indentMore->setText(tr("Increase indent"));
			indentMore->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INDENTINCREASE);
			indentMore->setPriority(QAction::LowPriority);
			indentMore->setCheckable(false);
			indentMore->setData(ADR_INDENT, INDENT_MORE);
			connect(indentMore, SIGNAL(triggered()), this, SLOT(onIndentChange()));
			menu->addAction(indentMore, AG_XHTMLIM_INDENT);

			//  *** Block formatting ***
			QTextBlockFormat blockFormat = cursor.blockFormat();
			//  Alignment			
			Qt::Alignment alignment = blockFormat.alignment();
			Menu *align = new Menu(menu);
			align->setTitle(tr("Text align"));
			align->menuAction()->setCheckable(true);
			connect(align->menuAction(), SIGNAL(triggered()), SLOT(onTextAlign()));

			group=new QActionGroup(align);
			action = new Action(group);
			action->setText(tr("Left"));
			action->setIcon(RSR_STORAGE_HTML, XHI_ALIGN_LEFT);
			action->setData(ADR_ALIGN_TYPE, int(Qt::AlignLeft|Qt::AlignAbsolute));
			action->setCheckable(true);
			if (alignment&Qt::AlignLeft)
			{
				if (blockFormat.hasProperty(QTextFormat::BlockAlignment))
					action->setChecked(true);
				align->setIcon(RSR_STORAGE_HTML, XHI_ALIGN_LEFT);
			}
			action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNLEFT);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
			align->addAction(action, AG_XHTMLIM_ALIGN);

			action = new Action(group);
			action->setText(tr("Center"));
			action->setIcon(RSR_STORAGE_HTML,XHI_ALIGN_CENTER);
			action->setData(ADR_ALIGN_TYPE, Qt::AlignHCenter);
			action->setCheckable(true);
			if (alignment&Qt::AlignHCenter)
			{
				if (blockFormat.hasProperty(QTextFormat::BlockAlignment))
					action->setChecked(true);
				align->setIcon(RSR_STORAGE_HTML, XHI_ALIGN_CENTER);
			}
			action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNCENTER);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
			align->addAction(action, AG_XHTMLIM_ALIGN);

			action = new Action(group);
			action->setText(tr("Right"));
			action->setIcon(RSR_STORAGE_HTML,XHI_ALIGN_RIGHT);
			action->setData(ADR_ALIGN_TYPE, int(Qt::AlignRight|Qt::AlignAbsolute));
			action->setCheckable(true);
			if (alignment&Qt::AlignRight)
			{
				if (blockFormat.hasProperty(QTextFormat::BlockAlignment))
					action->setChecked(true);
				align->setIcon(RSR_STORAGE_HTML, XHI_ALIGN_RIGHT);
			}
			action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNRIGHT);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
			align->addAction(action, AG_XHTMLIM_ALIGN);

			action = new Action(group);
			action->setText(tr("Justify"));
			action->setIcon(RSR_STORAGE_HTML,XHI_ALIGN_JUSTIFY);
			action->setData(ADR_ALIGN_TYPE, Qt::AlignJustify);
			action->setCheckable(true);
			if (alignment&Qt::AlignJustify)
			{
				if (blockFormat.hasProperty(QTextFormat::BlockAlignment))
					action->setChecked(true);
				align->setIcon(RSR_STORAGE_HTML, XHI_ALIGN_JUSTIFY);
			}
			action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNJUSTIFY);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
			align->addAction(action, AG_XHTMLIM_ALIGN);

			menu->addAction(align->menuAction(), AG_XHTMLIM_PARAGRAPH);

			// Text list
			int listStyle;
			if (blockFormat.isListFormat())
			{
				QTextListFormat listFormat = blockFormat.toListFormat();
				listStyle = listFormat.style();
			}
			else
				listStyle = 1;

			Menu *list = new Menu(menu);
			list->setTitle(tr("List"));
			list->setIcon(RSR_STORAGE_HTML, XHI_LIST);
			list->menuAction()->setEnabled(true);
			list->menuAction()->setData(ADR_LIST_TYPE, QTextListFormat::ListDisc);

			group=new QActionGroup(list);
			action = new Action(group);
			action->setText(tr("Disc"));
			action->setIcon(RSR_STORAGE_HTML, XHI_LIST_BULLET_DISC);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListDisc);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListDisc)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Circle"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_BULLET_CIRCLE);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListCircle);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListCircle)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Square"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_BULLET_SQUARE);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListSquare);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListSquare)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Decimal"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_DECIMAL);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListDecimal);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListDecimal)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Alpha lower"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ALPHA_LOW);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListLowerAlpha);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListLowerAlpha)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Alpha upper"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ALPHA_UP);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListUpperAlpha);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListUpperAlpha)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Roman lower"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ROMAN_LOW);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListLowerRoman);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListLowerRoman)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Roman upper"));
			action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ROMAN_UP);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListUpperRoman);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListUpperRoman)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_LIST);

			action = new Action(group);
			action->setText(tr("Definition list"));
			action->setIcon(RSR_STORAGE_HTML, XHI_LIST_DEFINITION);
			action->setData(ADR_LIST_TYPE, QTextListFormat::ListStyleUndefined);
			action->setCheckable(true);
			if (listStyle==QTextListFormat::ListStyleUndefined)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
			list->addAction(action, AG_XHTMLIM_DEFLIST);
			menu->addAction(list->menuAction(), AG_XHTMLIM_PARAGRAPH);


			// Special formatting
			Menu *format = new Menu(menu);
			format->setTitle(tr("Formatting type"));
			format->setIcon(RSR_STORAGE_HTML,XHI_FORMATTYPE);
			format->menuAction()->setCheckable(true);
			int type = checkBlockFormat(getCursor());
			connect(format->menuAction(), SIGNAL(triggered()), SLOT(onSetFormat()));

			group=new QActionGroup(format);
			action = new Action(group);
			action->setText(tr("Preformatted text"));
			action->setIcon(RSR_STORAGE_HTML, XHI_PREFORMAT);
			action->setData(ADR_FORMATTING_TYPE, FMT_PREFORMAT);
			action->setCheckable(true);
			if (type == FMT_PREFORMAT)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMAT);

			action = new Action(group);
			action->setText(tr("Heading 1"));
			action->setIcon(RSR_STORAGE_HTML, XHI_HEADING1);
			action->setData(ADR_FORMATTING_TYPE, FMT_HEADING1);
			action->setCheckable(true);
			if (type == FMT_HEADING1)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMATHEADING);

			action = new Action(group);
			action->setText(tr("Heading 2"));
			action->setIcon(RSR_STORAGE_HTML, XHI_HEADING2);
			action->setData(ADR_FORMATTING_TYPE, FMT_HEADING2);
			action->setCheckable(true);
			if (type == FMT_HEADING2)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMATHEADING);

			action = new Action(group);
			action->setText(tr("Heading 3"));
			action->setIcon(RSR_STORAGE_HTML, XHI_HEADING3);
			action->setData(ADR_FORMATTING_TYPE, FMT_HEADING3);
			action->setCheckable(true);
			if (type == FMT_HEADING3)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMATHEADING);

			action = new Action(group);
			action->setText(tr("Heading 4"));
			action->setIcon(RSR_STORAGE_HTML, XHI_HEADING4);
			action->setData(ADR_FORMATTING_TYPE, FMT_HEADING4);
			action->setCheckable(true);
			if (type == FMT_HEADING4)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMATHEADING);

			action = new Action(group);
			action->setText(tr("Heading 5"));
			action->setIcon(RSR_STORAGE_HTML, XHI_HEADING5);
			action->setData(ADR_FORMATTING_TYPE, FMT_HEADING5);
			action->setCheckable(true);
			if (type == FMT_HEADING5)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMATHEADING);

			action = new Action(group);
			action->setText(tr("Heading 6"));
			action->setIcon(RSR_STORAGE_HTML, XHI_HEADING6);
			action->setData(ADR_FORMATTING_TYPE, FMT_HEADING6);
			action->setCheckable(true);
			if (type == FMT_HEADING6)
				action->setChecked(true);
			action->setPriority(QAction::LowPriority);
			action->setActionGroup(group);
			connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
			format->addAction(action, AG_XHTMLIM_FORMATHEADING);
			menu->addAction(format->menuAction(), AG_XHTMLIM_PARAGRAPH);

			// *** Special commands **
			// Clear formatting
			Action *removeFormat=new Action(menu);
			removeFormat->setIcon(QIcon::fromTheme("format-text-clear", FIconStorage->getIcon(XHI_REMOVEFORMAT)));
			removeFormat->setText(tr("Remove format"));
			removeFormat->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATREMOVE);
			removeFormat->setPriority(QAction::LowPriority);
			connect(removeFormat, SIGNAL(triggered()), this, SLOT(onRemoveFormat()));
			removeFormat->setCheckable(false);
			menu->addAction(removeFormat, AG_XHTMLIM_SPECIAL);

			// Auto-reset formatting
			if (chatWindow)
			{
				Action *formatAutoReset = new Action(this);
				formatAutoReset->setIcon(QIcon::fromTheme("format-rich-text", FIconStorage->getIcon(XHI_NORICHTEXT)));
				formatAutoReset->setText(tr("Reset formatting on message send"));
				formatAutoReset->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATAUTORESET);
				formatAutoReset->setPriority(QAction::LowPriority);
				connect(formatAutoReset, SIGNAL(toggled(bool)), SLOT(onResetFormat(bool)));
				formatAutoReset->setCheckable(true);
				formatAutoReset->setChecked(Options::node(OPV_XHTML_FORMATAUTORESET).value().toBool());
				menu->addAction(formatAutoReset, AG_XHTMLIM_SPECIAL);
			}

			menu->setEnabled(FCurrentMessageEditWidget->isRichTextEnabled());
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
	QTextCharFormat charFormat;
	switch (action->data(ADR_DECORATION_TYPE).toInt())
	{
		case DT_BOLD:
			charFormat.setFontWeight(ASelected?QFont::Bold:QFont::Normal);
			break;
		case DT_ITALIC:
			charFormat.setFontItalic(ASelected);
			break;
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
	mergeFormatOnWordOrSelection(getCursor(), charFormat);
}

void XhtmlIm::onSelectCapitalization()
{
	Action *action = qobject_cast<Action *>(sender());
	QTextCharFormat charFormat;
	switch (action->data(ADR_CAPITALIZATION_TYPE).toInt())
	{
		case CPT_ALLLOWER:
			charFormat.setFontCapitalization(QFont::AllLowercase);
			break;
		case CPT_ALLUPPER:
			charFormat.setFontCapitalization(QFont::AllUppercase);
			break;
		case CPT_CAPITALIZE:
			charFormat.setFontCapitalization(QFont::Capitalize);
			break;
		case CPT_SMALLCAPS:
			charFormat.setFontCapitalization(QFont::SmallCaps);
			break;
		case CPT_MIXED:
			charFormat.setFontCapitalization(QFont::MixedCase);
			break;
	}
	mergeFormatOnWordOrSelection(getCursor(), charFormat);
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

	if (charFmtCurrent.isImageFormat())
	{
		QTextImageFormat imageFormat=charFmtCurrent.toImageFormat();
		cursor.select(QTextCursor::WordUnderCursor);
		imageUrl = QUrl::fromEncoded(imageFormat.name().toLatin1());
		QVariant imageResource = cursor.document()->resource(QTextDocument::ImageResource, imageUrl);
		if (imageResource.type()==QVariant::ByteArray)
			imageData=imageResource.toByteArray();
		else if (imageUrl.scheme()=="data")
		{
			QList<QString> parts=imageUrl.path().split(';');
			if (parts.size()==2 && parts[0].startsWith("image/"))
			{
				parts = parts[1].split(',');
				if (parts.size()==2 && parts[0]=="base64")
					imageData = QByteArray::fromBase64(parts[1].toLatin1());
			}
		}
		size.setWidth(imageFormat.width());
		size.setHeight(imageFormat.height());
		alt=imageFormat.property(XmlTextDocumentParser::ImageAlternativeText).toString();
	}

	Action *action=qobject_cast<Action *>(sender());
	InsertImage *insertImage = new InsertImage(this, FNetworkAccessManager, imageData, imageUrl, size, alt, action->parentWidget()->window());

	insertImage->setWindowIcon(FIconStorage->getIcon(XHI_INSERT_IMAGE));
	if(!supportBoB)
		insertImage->ui->pbBrowse->hide();
	if(insertImage->exec() == QDialog::Accepted)
	{
		if(!insertImage->getUrl().isEmpty())
		{
			cursor.beginEditBlock();
			QTextImageFormat imageFormat;
			QString          alt=insertImage->getAlternativeText();
			if (!alt.isEmpty())
				imageFormat.setProperty(XmlTextDocumentParser::ImageAlternativeText, alt);
			if (!insertImage->physResize())
			{
				if(insertImage->newHeight()!=insertImage->originalHeight())
					imageFormat.setHeight(insertImage->newHeight());
				if(insertImage->newWidth()!=insertImage->originalWidth())
					imageFormat.setWidth(insertImage->newWidth());
			}
			if(insertImage->isRemote())
			{
				QUrl url=insertImage->getUrl();
				imageFormat.setName(url.toEncoded());
				cursor.document()->addResource(QTextDocument::ImageResource, url, insertImage->getImageData());
				cursor.insertImage(imageFormat);
			}
			else
				if(supportBoB)
				{
					QByteArray imageData=insertImage->getImageData();
					QString contentId=FBitsOfBinary->contentIdentifier(imageData);
					QString uri=QString("cid:").append(contentId);
					imageFormat.setName(uri);
					imageFormat.setProperty(XhtmlIm::PMaxAge, insertImage->getMaxAge());
					imageFormat.setProperty(XhtmlIm::PMimeType, insertImage->getFileType());
					imageFormat.setProperty(XhtmlIm::PEmbed, insertImage->embed());
					cursor.document()->addResource(QTextDocument::ImageResource, QUrl(uri), imageData);
					cursor.insertImage(imageFormat);
				}
			cursor.endEditBlock();
		}
	}
	insertImage->deleteLater();
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
		charFormat.setFontFamily("Courier New,courier");
		mergeFormatOnWordOrSelection(cursor, charFormat);
	}
	else
	{
		QTextCharFormat charFormat = cursor.charFormat();
		charFormat.clearProperty(QTextFormat::FontFamily);
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

void XhtmlIm::onIndentChange()
{
	Action *action=qobject_cast<Action *>(sender());
	QTextCursor cursor = getCursor(false, false);
	bool increase=(action->data(ADR_INDENT)==INDENT_MORE);
	QTextBlockFormat blockFmt = cursor.blockFormat();
	if (cursor.currentList())
	{
		if (cursor.currentList()->format().style()==QTextListFormat::ListStyleUndefined)
			blockFmt.setIndent(increase);
	}
	else
	{
		qreal indentWidth=FCurrentMessageEditWidget->textEdit()->document()->indentWidth();
		qreal indent=blockFmt.textIndent();
		if (increase)
			blockFmt.setTextIndent(indent+indentWidth);
		else
			if (indent>0)
				blockFmt.setTextIndent(indent-indentWidth);
	}
	cursor.setBlockFormat(blockFmt);
}

void XhtmlIm::onTextAlign()
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Qt::AlignmentFlag align = Qt::AlignmentFlag(action->data(ADR_ALIGN_TYPE).toInt());
		QTextCursor cursor = getCursor(false, false);
		QTextBlockFormat format=cursor.blockFormat();
		if (format.hasProperty(QTextFormat::BlockAlignment) && format.alignment()==align)
			format.clearProperty(QTextFormat::BlockAlignment);
		else
			format.setAlignment(align);
		cursor.setBlockFormat(format);
	}
}

void XhtmlIm::onInsertList()
{
	Action *ac = qobject_cast<Action *>(sender());
	if (ac)
	{
		QTextListFormat::Style style = (QTextListFormat::Style)ac->data(ADR_LIST_TYPE).toInt();// = QTextListFormat::ListStyleUndefined;
//		FMenuList->setIcon(ac->icon());
//		FMenuList->menuAction()->setData(ADR_LIST_TYPE, style);
		QTextCursor cursor = getCursor();
		if (style >=QTextListFormat::ListUpperRoman && style <=QTextListFormat::ListStyleUndefined)
		{
			cursor.beginEditBlock();
			QTextListFormat listFormat;
			QTextList       *list=cursor.currentList();
			if (list)   //  Have list at current position
			{
				listFormat = list->format();
				int indent=listFormat.indent();
				if (listFormat.style()==QTextListFormat::ListStyleUndefined)
					indent++;
				listFormat.setStyle(style);
				if (style==QTextListFormat::ListStyleUndefined)
					indent--;
				listFormat.setIndent(indent);
				if (list->itemNumber(cursor.block())==0 && cursor.atBlockStart())    // First block in the list
					list->setFormat(listFormat);               //  Just update list format
				else                                        // Not first block
				{
					indent++;
					listFormat.setIndent(indent);  //  Create a sublist
					if (!cursor.atBlockStart() || cursor.block().previous().textList()!=list)
						cursor.insertBlock();
					cursor.createList(listFormat);
				}
			}
			else        // No list at current position
			{
				int indent=style==QTextListFormat::ListStyleUndefined?0:1;
				listFormat.setIndent(indent);
				listFormat.setStyle(style);    // Just set its style
				cursor.createList(listFormat); // and create a root list
			}
			cursor.endEditBlock();
		}
		else
		{
			QTextBlockFormat blockFormat;
			blockFormat.setObjectIndex(-1);
			cursor.mergeBlockFormat(blockFormat);
		}
	}
}

void XhtmlIm::onSetFormat()
{
	Action *action = qobject_cast<Action *>(sender());
//	if (ac!=FMenuFormat->menuAction())
//		FActionLastFormat=ac;
	int formatType = action->data(ADR_FORMATTING_TYPE).toInt();
	QTextCursor cursor = getCursor();
	int currentFormatType=checkBlockFormat(cursor);

	cursor.beginEditBlock();
	QTextCharFormat blockCharFormat;
	QTextBlockFormat blockFormat;

	if (currentFormatType!=formatType)
	{
		if (formatType==FMT_PREFORMAT)
		{
			blockCharFormat.setFontFixedPitch(true);
			blockFormat.setProperty(QTextFormat::BlockNonBreakableLines, true);
		}
		else
		{
			blockCharFormat.setProperty(QTextFormat::FontSizeAdjustment, 4-formatType);
			blockCharFormat.setFontWeight(QFont::Bold);
		}

		int first, last;
		if (cursor.position()<cursor.anchor())
		{
			first=cursor.position();
			last=cursor.anchor();
		}
		else
		{
			first=cursor.anchor();
			last=cursor.position();
		}
		cursor.setPosition(first);
		cursor.movePosition(QTextCursor::StartOfBlock);
		QTextBlock block;
		for (block=cursor.block(); !block.contains(last); block=block.next());

		cursor.setPosition(block.position(), QTextCursor::KeepAnchor);
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		cursor.mergeCharFormat(blockCharFormat);
	}
	else
	{
		QSet<QTextFormat::Property> properties;
		properties.insert(QTextFormat::FontSizeAdjustment);
		properties.insert(QTextFormat::FontWeight);
		clearBlockProperties(cursor.block(), properties);
	}
	cursor.setBlockCharFormat(blockCharFormat);
	cursor.setBlockFormat(blockFormat);
	cursor.endEditBlock();
//	updateCurrentBlock(cursor);
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

int XhtmlIm::checkBlockFormat(const QTextCursor &ACursor)
{
	QTextCharFormat  charFormat = ACursor.blockCharFormat();
	QTextBlockFormat format = ACursor.blockFormat();
	int header=XmlTextDocumentParser::header(charFormat);
	if (header)
		return header;
	else if (format.boolProperty(QTextFormat::BlockNonBreakableLines) && charFormat.boolProperty(QTextFormat::FontFixedPitch))
		return FMT_PREFORMAT;
	return FMT_NORMAL;
}

void XhtmlIm::clearBlockProperties(const QTextBlock &ATextBlock, const QSet<QTextFormat::Property> &AProperties)
{
	QTextCursor cursor(ATextBlock);
	for (QTextBlock::iterator it=ATextBlock.begin(); it!=ATextBlock.end(); it++)
	{
		QTextFragment fragment=it.fragment();
		// Select fragment
		cursor.setPosition(fragment.position());
		cursor.setPosition(fragment.position()+fragment.length(), QTextCursor::KeepAnchor);
		QTextCharFormat charFormat=fragment.charFormat();
		for (QSet<QTextFormat::Property>::const_iterator it=AProperties.begin(); it!=AProperties.end(); it++)
			charFormat.clearProperty(*it);
		cursor.setCharFormat(charFormat);
	}
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
							QUrl imageUrl(imageFormat.name());
							if (imageUrl.scheme()=="cid" || imageUrl.scheme()=="file" || imageUrl.scheme()=="data")
							{
								QByteArray imageData;
								QVariant imageResource = ADocument->resource(QTextDocument::ImageResource, imageUrl);
								if (imageResource.type()==QVariant::ByteArray)
									imageData=imageResource.toByteArray();
								else if (imageUrl.scheme()=="data")
								{
									QList<QString> parts=imageUrl.path().split(';');
									if (parts.size()==2 && parts[0].startsWith("image/"))
									{
										format.setProperty(PMaxAge, Options::node(OPV_XHTML_MAXAGE).value().toLongLong());
										format.setProperty(PMimeType, parts[0]);
										parts = parts[1].split(',');
										if (parts.size()==2 && parts[0]=="base64")
											imageData = QByteArray::fromBase64(parts[1].toLatin1());
										format.setProperty(PEmbed, imageData.size()<= Options::node(OPV_XHTML_EMBEDSIZE).value().toInt());
									}
								}

								if (!imageData.isEmpty())
								{
									QString type	= format.property(PMimeType).toString();
									quint64 maxAge	= format.property(PMaxAge).toLongLong();

									QString cid;
									if (imageUrl.scheme()=="cid")
										cid=imageUrl.path();
									else    // file or data
									{
										cid=FBitsOfBinary->contentIdentifier(imageData);
										imageFormat.setName(QString("cid:").append(cid));
										QTextCursor cursor(ADocument);
										// Select contents
										cursor.setPosition(fragment.position(), QTextCursor::MoveAnchor);
										cursor.setPosition(fragment.position()+fragment.length(), QTextCursor::KeepAnchor);
										cursor.insertImage(imageFormat);
									}
									FBitsOfBinary->saveBinary(cid, type, imageData, maxAge);
									if(format.property(PEmbed).toBool())
										FBitsOfBinary->saveBinary(cid, type, imageData, maxAge, AMessage.stanza());
								}
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

	if (AOrder==ECHO_XHTML_COPY_INSERT && messageEditContentsCanInsert(AOrder,AWidget,AData))
	{
		if (AData->hasImage())
		{
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
				else
				{
					QString html=AData->html();
					fixHtml(html);
					QTextCursor(ADocument).insertHtml(html);
		//			XmlTextDocumentParser::htmlToText(ADocument, html);
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
//			XmlTextDocumentParser::htmlToText(ADocument, html);
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

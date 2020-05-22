#include "messagewidgets.h"

#include <QPair>
#include <QBuffer>
#include <QMimeData>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <QClipboard>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include <definitions/messageviewurlhandlerorders.h>
#include <definitions/messageeditcontentshandlerorders.h>
#include <utils/widgetmanager.h>
#include <utils/textmanager.h>
#include <utils/shortcuts.h>
#include <utils/options.h>

#define ADR_ME_WINDOW			Action::DR_Parametr1 // *** <<< eyeCU >>> ***
#define ADR_QUOTE_WINDOW        Action::DR_Parametr1
#define ADR_CONTEXT_DATA        Action::DR_Parametr1

MessageWidgets::MessageWidgets()
{
	FOptionsManager = NULL;
	FMainWindow = NULL;
}

MessageWidgets::~MessageWidgets()
{
	FCleanupHandler.clear();
}

void MessageWidgets::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Message Widgets Manager");
	APluginInfo->description = tr("Allows other modules to use standard widgets for messaging");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
}

bool MessageWidgets::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IMainWindowPlugin").value(0,NULL);
	if (plugin)
	{
		IMainWindowPlugin *mainWindowPlugin = qobject_cast<IMainWindowPlugin *>(plugin->instance());
		if (mainWindowPlugin)
			FMainWindow = mainWindowPlugin->mainWindow();
	}

	connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
	connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
	connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));

	return true;
}

bool MessageWidgets::initObjects()
{
	Shortcuts::declareGroup(SCTG_TABWINDOW, tr("Tab window"), SGO_TABWINDOW);
	Shortcuts::declareShortcut(SCT_TABWINDOW_CLOSETAB, tr("Close tab"), tr("Ctrl+W","Close tab"));
	Shortcuts::declareShortcut(SCT_TABWINDOW_CLOSEOTHERTABS, tr("Close other tabs"), tr("Ctrl+Shift+W","Close other tabs"));
	Shortcuts::declareShortcut(SCT_TABWINDOW_DETACHTAB, tr("Detach tab to separate window"), QKeySequence::UnknownKey);
	Shortcuts::declareShortcut(SCT_TABWINDOW_NEXTTAB, tr("Next tab"), QKeySequence::NextChild);
	Shortcuts::declareShortcut(SCT_TABWINDOW_PREVTAB, tr("Previous tab"), QKeySequence::PreviousChild);
	Shortcuts::declareShortcut(SCT_TABWINDOW_CLOSEWINDOW, QString::null, tr("Esc","Close tab window"));

	for (int tabNumber=1; tabNumber<=10; tabNumber++)
		Shortcuts::declareShortcut(QString(SCT_TABWINDOW_QUICKTAB).arg(tabNumber), QString::null, tr("Alt+%1","Show tab").arg(tabNumber % 10));

	Shortcuts::declareGroup(SCTG_MESSAGEWINDOWS, tr("Message windows"), SGO_MESSAGEWINDOWS);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_CLOSEWINDOW, QString::null, tr("Esc","Close message window"));
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_QUOTE, tr("Quote selected text"), tr("Ctrl+Q","Quote selected text"));
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_ME, tr("Toggle /me command"), tr("Alt+M","Toggle /me command"));
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_EDITNEXTMESSAGE, tr("Edit next message"), tr("Ctrl+Down","Edit next message"), Shortcuts::WidgetShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_EDITPREVMESSAGE, tr("Edit previous message"), tr("Ctrl+Up","Edit previous message"), Shortcuts::WidgetShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_SENDCHATMESSAGE, tr("Send chat message"), tr("Return","Send chat message"), Shortcuts::WidgetShortcut);
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_SENDNORMALMESSAGE, tr("Send single message"), tr("Ctrl+Return","Send single message"), Shortcuts::WidgetShortcut);

	insertViewUrlHandler(MVUHO_MESSAGEWIDGETS_DEFAULT,this);
	insertEditContentsHandler(MECHO_MESSAGEWIDGETS_COPY_INSERT,this);

	return true;
}

bool MessageWidgets::initSettings()
{
	Options::setDefaultValue(OPV_MESSAGES_COMBINEWITHROSTER,true);
	Options::setDefaultValue(OPV_MESSAGES_EDITORAUTORESIZE,true);
	Options::setDefaultValue(OPV_MESSAGES_EDITORMINIMUMLINES,1);
	Options::setDefaultValue(OPV_MESSAGES_EDITORBASEFONTSIZE,0.0);
	Options::setDefaultValue(OPV_MESSAGES_CLEANCHATTIMEOUT,30);

	Options::setDefaultValue(OPV_MESSAGES_SHOWSTATUS,true);
	Options::setDefaultValue(OPV_MESSAGES_ARCHIVESTATUS,false);

	Options::setDefaultValue(OPV_MESSAGES_TABWINDOWS_ENABLE,false);
	Options::setDefaultValue(OPV_MESSAGES_TABWINDOW_NAME,tr("Tab Window"));
	Options::setDefaultValue(OPV_MESSAGES_TABWINDOW_TABSCLOSABLE,true);
	Options::setDefaultValue(OPV_MESSAGES_TABWINDOW_TABSBOTTOM,false);

	if (FOptionsManager)
	{
		IOptionsDialogNode messagesNode = { ONO_MESSAGES, OPN_MESSAGES, MNI_NORMALMHANDLER_MESSAGE, tr("Messages") };
		FOptionsManager->insertOptionsDialogNode(messagesNode);
		FOptionsManager->insertOptionsDialogHolder(this);
	}

	return true;
}

QMultiMap<int, IOptionsDialogWidget *> MessageWidgets::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager && ANodeId==OPN_MESSAGES)
	{
		widgets.insertMulti(OHO_MESSAGES_VIEW,FOptionsManager->newOptionsDialogHeader(tr("Message window view"),AParent));
		widgets.insertMulti(OWO_MESSAGES_LOADHISTORY,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_LOADHISTORY),tr("Load last messages from history"),AParent));
		widgets.insertMulti(OWO_MESSAGES_COMBINEWITHROSTER,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_COMBINEWITHROSTER),tr("Show message windows together with contacts list"),AParent));
		widgets.insertMulti(OWO_MESSAGES_TABWINDOWSENABLE,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_TABWINDOWS_ENABLE),tr("Show message windows in tab window"),AParent));
		widgets.insertMulti(OWO_MESSAGES_EDITORAUTORESIZE,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_EDITORAUTORESIZE),tr("Automatically resize messages input field"),AParent));
		widgets.insertMulti(OWO_MESSAGES_EDITORMINIMUMLINES,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_EDITORMINIMUMLINES),tr("Minimum number of lines in messages input field:"),AParent));

		widgets.insertMulti(OHO_MESSAGES_BEHAVIOR,FOptionsManager->newOptionsDialogHeader(tr("Message window behavior"),AParent));
		widgets.insertMulti(OWO_MESSAGES_SHOWSTATUS,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_SHOWSTATUS),tr("Show contacts status changes"),AParent));
		widgets.insertMulti(OWO_MESSAGES_ARCHIVESTATUS,FOptionsManager->newOptionsDialogWidget(Options::node(OPV_MESSAGES_ARCHIVESTATUS),tr("Save contacts status messages in history"),AParent));

		widgets.insertMulti(OHO_MESSAGES_INFOBAR_ICONS, FOptionsManager->newOptionsDialogHeader(tr("Information bar icons"), AParent)); // *** <<< eyeCU >>> ***
	}
	return widgets;
}

bool MessageWidgets::messageViewUrlOpen(int AOrder, IMessageViewWidget* APage, const QUrl &AUrl)
{
	Q_UNUSED(APage);
	Q_UNUSED(AOrder);
	return QDesktopServices::openUrl(AUrl);
}

bool MessageWidgets::messageEditContentsCreate(int AOrder, IMessageEditWidget *AWidget, QMimeData *AData)
{
	if (AOrder == MECHO_MESSAGEWIDGETS_COPY_INSERT)
	{
		QTextDocumentFragment fragment = AWidget->textEdit()->textCursor().selection();
		if (!fragment.isEmpty())
		{
			if (AWidget->isRichTextEnabled())
			{
				QBuffer buffer;
				QTextDocumentWriter writer(&buffer, "ODF");
				writer.write(fragment);
				buffer.close();
				AData->setData("application/vnd.oasis.opendocument.text", buffer.data());
				AData->setData("text/html", fragment.toHtml("utf-8").toUtf8());
			}
			AData->setText(fragment.toPlainText());
		}
	}
	return false;
}

bool MessageWidgets::messageEditContentsCanInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData)
{
	Q_UNUSED(AWidget);
	if (AOrder == MECHO_MESSAGEWIDGETS_COPY_INSERT)
		return AData->hasText() || AData->hasHtml();
	return false;
}

bool MessageWidgets::messageEditContentsInsert(int AOrder, IMessageEditWidget *AWidget, const QMimeData *AData, QTextDocument *ADocument)
{
	if (AOrder == MECHO_MESSAGEWIDGETS_COPY_INSERT)
	{
		QTextDocumentFragment fragment;
		if (AWidget->isRichTextEnabled() && AData->hasHtml())
			fragment = QTextDocumentFragment::fromHtml(AData->html().replace(QChar::Null,""));
		else if (AData->hasText())
			fragment = QTextDocumentFragment::fromPlainText(AData->text().replace(QChar::Null,""));
		else if (AData->hasHtml())
			fragment = QTextDocumentFragment::fromPlainText(QTextDocumentFragment::fromHtml(AData->html().replace(QChar::Null,"")).toPlainText());

		if (!fragment.isEmpty())
		{
			QTextCursor cursor(ADocument);
			cursor.insertFragment(fragment);
		}
	}
	return false;
}

bool MessageWidgets::messageEditContentsChanged(int AOrder, IMessageEditWidget *AWidget, int &APosition, int &ARemoved, int &AAdded)
{
	Q_UNUSED(AOrder); Q_UNUSED(AWidget); Q_UNUSED(APosition); Q_UNUSED(ARemoved); Q_UNUSED(AAdded);
	return false;
}

IMessageAddress *MessageWidgets::newAddress(const Jid &AStreamJid, const Jid &AContactJid, QObject *AParent)
{
	IMessageAddress *address = new Address(this,AStreamJid,AContactJid,AParent);
	FCleanupHandler.add(address->instance());
	emit addressCreated(address);
	return address;
}

IMessageInfoWidget *MessageWidgets::newInfoWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageInfoWidget *widget = new InfoWidget(this,AWindow,AParent);
	FCleanupHandler.add(widget->instance());
	emit infoWidgetCreated(widget);
	return widget;
}

IMessageViewWidget *MessageWidgets::newViewWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageViewWidget *widget = new ViewWidget(this,AWindow,AParent);
	connect(widget->instance(),SIGNAL(viewContextMenu(const QPoint &, Menu *)),SLOT(onViewWidgetContextMenu(const QPoint &, Menu *)));
	FCleanupHandler.add(widget->instance());
	emit viewWidgetCreated(widget);
	return widget;
}

IMessageEditWidget *MessageWidgets::newEditWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageEditWidget *widget = new EditWidget(this,AWindow,AParent);
	FCleanupHandler.add(widget->instance());
// *** <<< eyeCU <<< ***
	connect(widget->textEdit(), SIGNAL(destroyed(QObject*)), SLOT(onTextEditDestroyed(QObject*)));
// *** >>> eyeCU >>> ***
	emit editWidgetCreated(widget);
	return widget;
}

IMessageReceiversWidget *MessageWidgets::newReceiversWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageReceiversWidget *widget = new ReceiversWidget(this,AWindow,AParent);
	FCleanupHandler.add(widget->instance());
	emit receiversWidgetCreated(widget);
	return widget;
}

IMessageMenuBarWidget *MessageWidgets::newMenuBarWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageMenuBarWidget *widget = new MenuBarWidget(AWindow,AParent);
	FCleanupHandler.add(widget->instance());
	emit menuBarWidgetCreated(widget);
	return widget;
}

IMessageToolBarWidget *MessageWidgets::newToolBarWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageToolBarWidget *widget = new ToolBarWidget(AWindow,AParent);
	FCleanupHandler.add(widget->instance());
	insertToolBarQuoteAction(widget);
	insertToolBarMeAction(widget);
	emit toolBarWidgetCreated(widget);
	return widget;
}

IMessageStatusBarWidget *MessageWidgets::newStatusBarWidget(IMessageWindow *AWindow, QWidget *AParent)
{
	IMessageStatusBarWidget *widget = new StatusBarWidget(AWindow,AParent);
	FCleanupHandler.add(widget->instance());
	emit statusBarWidgetCreated(widget);
	return widget;
}

IMessageTabPageNotifier *MessageWidgets::newTabPageNotifier(IMessageTabPage *ATabPage)
{
	IMessageTabPageNotifier *notifier = new TabPageNotifier(ATabPage);
	FCleanupHandler.add(notifier->instance());
	emit tabPageNotifierCreated(notifier);
	return notifier;
}

QList<IMessageNormalWindow *> MessageWidgets::normalWindows() const
{
	return FNormalWindows;
}

IMessageNormalWindow *MessageWidgets::getNormalWindow(const Jid &AStreamJid, const Jid &AContactJid, IMessageNormalWindow::Mode AMode)
{
	IMessageNormalWindow *window = findNormalWindow(AStreamJid,AContactJid,true);
	if (window == NULL)
	{
		window = new NormalWindow(this,AStreamJid,AContactJid,AMode);
		FNormalWindows.append(window);
		WidgetManager::setWindowSticky(window->instance(),true);
		connect(window->instance(),SIGNAL(tabPageDestroyed()),SLOT(onNormalWindowDestroyed()));
		FCleanupHandler.add(window->instance());
		emit normalWindowCreated(window);
		return window;
	}
	return NULL;
}

IMessageNormalWindow *MessageWidgets::findNormalWindow(const Jid &AStreamJid, const Jid &AContactJid, bool AExact) const
{
	foreach(IMessageNormalWindow *window, FNormalWindows)
	{
		if (AExact && window->address()->availAddresses().contains(AStreamJid,AContactJid))
			return window;
		else if (!AExact && window->address()->availAddresses(true).contains(AStreamJid,AContactJid.bare()))
			return window;
	}
	return NULL;
}

QList<IMessageChatWindow *> MessageWidgets::chatWindows() const
{
	return FChatWindows;
}

IMessageChatWindow *MessageWidgets::getChatWindow(const Jid &AStreamJid, const Jid &AContactJid)
{
	IMessageChatWindow *window = findChatWindow(AStreamJid,AContactJid,true);
	if (window == NULL)
	{
		window = new ChatWindow(this,AStreamJid,AContactJid);
		FChatWindows.append(window);
		WidgetManager::setWindowSticky(window->instance(),true);
		connect(window->instance(),SIGNAL(tabPageDestroyed()),SLOT(onChatWindowDestroyed()));
		FCleanupHandler.add(window->instance());
		emit chatWindowCreated(window);
		return window;
	}
	return NULL;
}

IMessageChatWindow *MessageWidgets::findChatWindow(const Jid &AStreamJid, const Jid &AContactJid, bool AExact) const
{
	foreach(IMessageChatWindow *window, FChatWindows)
	{
		if (AExact && window->address()->availAddresses().contains(AStreamJid,AContactJid))
			return window;
		else if (!AExact && window->address()->availAddresses(true).contains(AStreamJid,AContactJid.bare()))
			return window;
	}
	return NULL;
}

QList<QUuid> MessageWidgets::tabWindowList() const
{
	QList<QUuid> list;
	foreach(const QString &tabWindowId, Options::node(OPV_MESSAGES_TABWINDOWS_ROOT).childNSpaces("window"))
		list.append(tabWindowId);
	return list;
}

QUuid MessageWidgets::appendTabWindow(const QString &AName)
{
	QUuid id = QUuid::createUuid();
	QString name = AName;
	if (name.isEmpty())
	{
		QList<QString> names;
		foreach(const QString &tabWindowId, Options::node(OPV_MESSAGES_TABWINDOWS_ROOT).childNSpaces("window"))
			names.append(Options::node(OPV_MESSAGES_TABWINDOW_ITEM,tabWindowId).value().toString());

		int i = 0;
		do
		{
			i++;
			name = tr("Tab Window %1").arg(i);
		} while (names.contains(name));
	}
	Options::node(OPV_MESSAGES_TABWINDOW_ITEM,id.toString()).setValue(name,"name");
	emit tabWindowAppended(id,name);
	return id;
}

void MessageWidgets::deleteTabWindow(const QUuid &AWindowId)
{
	if (AWindowId!=Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString() && tabWindowList().contains(AWindowId))
	{
		IMessageTabWindow *window = findTabWindow(AWindowId);
		if (window)
			window->instance()->deleteLater();
		Options::node(OPV_MESSAGES_TABWINDOWS_ROOT).removeChilds("window",AWindowId.toString());
		emit tabWindowDeleted(AWindowId);
	}
}

QString MessageWidgets::tabWindowName(const QUuid &AWindowId) const
{
	if (tabWindowList().contains(AWindowId))
		return Options::node(OPV_MESSAGES_TABWINDOW_ITEM,AWindowId.toString()).value("name").toString();
	return Options::defaultValue(OPV_MESSAGES_TABWINDOW_NAME).toString();
}

void MessageWidgets::setTabWindowName(const QUuid &AWindowId, const QString &AName)
{
	if (!AName.isEmpty() && tabWindowList().contains(AWindowId))
	{
		Options::node(OPV_MESSAGES_TABWINDOW_ITEM,AWindowId.toString()).setValue(AName,"name");
		emit tabWindowNameChanged(AWindowId,AName);
	}
}

QList<IMessageTabWindow *> MessageWidgets::tabWindows() const
{
	return FTabWindows;
}

IMessageTabWindow *MessageWidgets::getTabWindow(const QUuid &AWindowId)
{
	IMessageTabWindow *window = findTabWindow(AWindowId);
	if (!window)
	{
		window = new TabWindow(this,AWindowId);
		FTabWindows.append(window);
		WidgetManager::setWindowSticky(window->instance(),true);
		connect(window->instance(),SIGNAL(tabPageAdded(IMessageTabPage *)),SLOT(onTabWindowPageAdded(IMessageTabPage *)));
		connect(window->instance(),SIGNAL(currentTabPageChanged(IMessageTabPage *)),SLOT(onTabWindowCurrentPageChanged(IMessageTabPage *)));
		connect(window->instance(),SIGNAL(windowDestroyed()),SLOT(onTabWindowDestroyed()));
		emit tabWindowCreated(window);
	}
	return window;
}

IMessageTabWindow *MessageWidgets::findTabWindow(const QUuid &AWindowId) const
{
	foreach(IMessageTabWindow *window,FTabWindows)
		if (window->windowId() == AWindowId)
			return window;
	return NULL;
}

void MessageWidgets::assignTabWindowPage(IMessageTabPage *APage)
{
	if (!FAssignedPages.contains(APage))
	{
		FAssignedPages.append(APage);
		connect(APage->instance(),SIGNAL(tabPageDestroyed()),SLOT(onAssignedTabPageDestroyed()));
	}

	if (Options::node(OPV_MESSAGES_COMBINEWITHROSTER).value().toBool())
	{
		IMessageTabWindow *window = getTabWindow(Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString());
		window->addTabPage(APage);
	}
	else if (Options::node(OPV_MESSAGES_TABWINDOWS_ENABLE).value().toBool())
	{
		QList<QUuid> availWindows = tabWindowList();

		QUuid windowId = FPageWindows.value(APage->tabPageId());
		if (!availWindows.contains(windowId))
			windowId = Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString();
		if (!availWindows.contains(windowId))
			windowId = availWindows.value(0);

		IMessageTabWindow *window = getTabWindow(windowId);
		window->addTabPage(APage);
	}
}

QList<IMessageViewDropHandler *> MessageWidgets::viewDropHandlers() const
{
	return FViewDropHandlers;
}

void MessageWidgets::insertViewDropHandler(IMessageViewDropHandler *AHandler)
{
	if (AHandler && !FViewDropHandlers.contains(AHandler))
		FViewDropHandlers.append(AHandler);
}

void MessageWidgets::removeViewDropHandler(IMessageViewDropHandler *AHandler)
{
	if (FViewDropHandlers.contains(AHandler))
		FViewDropHandlers.removeAll(AHandler);
}

QMultiMap<int, IMessageViewUrlHandler *> MessageWidgets::viewUrlHandlers() const
{
	return FViewUrlHandlers;
}

void MessageWidgets::insertViewUrlHandler(int AOrder, IMessageViewUrlHandler *AHandler)
{
	if (AHandler && !FViewUrlHandlers.contains(AOrder,AHandler))
		FViewUrlHandlers.insertMulti(AOrder,AHandler);
}

void MessageWidgets::removeViewUrlHandler(int AOrder, IMessageViewUrlHandler *AHandler)
{
	if (FViewUrlHandlers.contains(AOrder,AHandler))
		FViewUrlHandlers.remove(AOrder,AHandler);
}

QMultiMap<int, IMessageEditSendHandler *> MessageWidgets::editSendHandlers() const
{
	return FEditSendHandlers;
}

void MessageWidgets::insertEditSendHandler(int AOrder, IMessageEditSendHandler *AHandler)
{
	if (AHandler && !FEditSendHandlers.contains(AOrder,AHandler))
		FEditSendHandlers.insertMulti(AOrder,AHandler);
}

void MessageWidgets::removeEditSendHandler(int AOrder, IMessageEditSendHandler *AHandler)
{
	if (FEditSendHandlers.contains(AOrder,AHandler))
		FEditSendHandlers.remove(AOrder,AHandler);
}

QMultiMap<int, IMessageEditContentsHandler *> MessageWidgets::editContentsHandlers() const
{
	return FEditContentsHandlers;
}

void MessageWidgets::insertEditContentsHandler(int AOrder, IMessageEditContentsHandler *AHandler)
{
	if (AHandler && !FEditContentsHandlers.contains(AOrder,AHandler))
		FEditContentsHandlers.insertMulti(AOrder,AHandler);
}

void MessageWidgets::removeEditContentsHandler(int AOrder, IMessageEditContentsHandler *AHandler)
{
	if (FEditContentsHandlers.contains(AOrder,AHandler))
		FEditContentsHandlers.remove(AOrder,AHandler);
}

void MessageWidgets::deleteTabWindows()
{
	foreach(IMessageTabWindow *window, tabWindows())
		delete window->instance();
}

void MessageWidgets::insertToolBarQuoteAction(IMessageToolBarWidget *AWidget)
{
	Action *quoteAction = createQuouteAction(AWidget->messageWindow(),AWidget->instance());
	if (quoteAction)
	{
		AWidget->toolBarChanger()->insertAction(quoteAction,TBG_MWTBW_MESSAGEWIDGETS_QUOTE);
		AWidget->toolBarChanger()->actionHandle(quoteAction)->setVisible(quoteAction->isVisible());
		connect(AWidget->messageWindow()->instance(),SIGNAL(widgetLayoutChanged()),SLOT(onMessageWindowWidgetLayoutChanged()));
	}
}

Action *MessageWidgets::createQuouteAction(IMessageWindow *AWindow, QObject *AParent)
{
	if (AWindow->viewWidget() && AWindow->editWidget())
	{
		Action *quoteAction = new Action(AParent);
		quoteAction->setData(ADR_QUOTE_WINDOW,(qint64)AWindow->instance());
		quoteAction->setText(tr("Quote Selected Text"));
		quoteAction->setToolTip(tr("Quote selected text"));
		quoteAction->setIcon(RSR_STORAGE_MENUICONS, MNI_MESSAGEWIDGETS_QUOTE);
		quoteAction->setShortcutId(SCT_MESSAGEWINDOWS_QUOTE);
		quoteAction->setVisible(AWindow->viewWidget()->isVisibleOnWindow() && AWindow->editWidget()->isVisibleOnWindow());
		connect(quoteAction,SIGNAL(triggered(bool)),SLOT(onQuoteActionTriggered(bool)));
		return quoteAction;
	}
	return NULL;
}
// *** <<< eyeCU <<< ***
void MessageWidgets::insertToolBarMeAction(IMessageToolBarWidget *AWidget)
{
	Action *meAction = createMeAction(AWidget->messageWindow(),AWidget->instance());
	if (meAction)
	{
		AWidget->toolBarChanger()->insertAction(meAction,TBG_MWTBW_MESSAGEWIDGETS_ME);
		AWidget->toolBarChanger()->actionHandle(meAction)->setVisible(meAction->isVisible());
		connect(AWidget->messageWindow()->instance(),SIGNAL(widgetLayoutChanged()),SLOT(onMessageWindowWidgetLayoutChanged()));
	}
}

Action *MessageWidgets::createMeAction(IMessageWindow *AWindow, QObject *AParent)
{
	if (AWindow->viewWidget() && AWindow->editWidget())
	{
		Action *meAction = new Action(AParent);
		meAction->setCheckable(true);
		meAction->setData(ADR_ME_WINDOW,(qint64)AWindow->instance());
		meAction->setText("/me");
		meAction->setToolTip(tr("Toggle /me command"));
		meAction->setIcon(RSR_STORAGE_MENUICONS, MNI_MESSAGEWIDGETS_ME);
		meAction->setShortcutId(SCT_MESSAGEWINDOWS_ME);
		meAction->setVisible(AWindow->viewWidget()->isVisibleOnWindow() && AWindow->editWidget()->isVisibleOnWindow());
		FMeActions.insert(AWindow->editWidget()->textEdit(), meAction);
		connect(AWindow->editWidget()->textEdit(), SIGNAL(textChanged()), SLOT(onTextChanged()));
		connect(meAction,SIGNAL(triggered(bool)),SLOT(onMeActionTriggered(bool)));
		return meAction;
	}
	return NULL;
}
// *** >>> eyeCU >>> ***
void MessageWidgets::onViewWidgetContextMenu(const QPoint &APosition, Menu *AMenu)
{
	IMessageViewWidget *widget = qobject_cast<IMessageViewWidget *>(sender());

	QTextDocumentFragment textSelection = widget!=NULL ? widget->selection() : QTextDocumentFragment();
	QTextDocumentFragment textFragment = widget!=NULL ? widget->textFragmentAt(APosition) : QTextDocumentFragment();
	QString href  = TextManager::getTextFragmentHref(textFragment.isEmpty() ? textSelection : textFragment);

	QUrl link = href;
	if (link.isValid())
	{
		bool isMailto = link.scheme()=="mailto";

		Action *urlAction = new Action(AMenu);
		urlAction->setText(isMailto ? tr("Send mail") : tr("Open link"));
		urlAction->setData(ADR_CONTEXT_DATA,href);
		connect(urlAction,SIGNAL(triggered(bool)),SLOT(onViewContextUrlActionTriggered(bool)));
		AMenu->addAction(urlAction,AG_MWVWCM_MESSAGEWIDGETS_URL,true);
		AMenu->setDefaultAction(urlAction);

		Action *copyHrefAction = new Action(AMenu);
		copyHrefAction->setText(tr("Copy address"));
		copyHrefAction->setData(ADR_CONTEXT_DATA,isMailto ? link.path() : href);
		connect(copyHrefAction,SIGNAL(triggered(bool)),SLOT(onViewContextCopyActionTriggered(bool)));
		AMenu->addAction(copyHrefAction,AG_MWVWCM_MESSAGEWIDGETS_COPY,true);
	}
	
	if (!textSelection.isEmpty())
	{
		Action *copyAction = new Action(AMenu);
		copyAction->setText(tr("Copy"));
		copyAction->setShortcut(QKeySequence::Copy);
		copyAction->setData(ADR_CONTEXT_DATA,textSelection.toHtml());
		connect(copyAction,SIGNAL(triggered(bool)),SLOT(onViewContextCopyActionTriggered(bool)));
		AMenu->addAction(copyAction,AG_MWVWCM_MESSAGEWIDGETS_COPY,true);

		Action *quoteAction = createQuouteAction(widget->messageWindow(),AMenu);
		if (quoteAction)
			AMenu->addAction(quoteAction,AG_MWVWCM_MESSAGEWIDGETS_QUOTE,true);

		QString plainSelection = textSelection.toPlainText().trimmed();
		Action *searchAction = new Action(AMenu);
		searchAction->setText(tr("Search on Google '%1'").arg(TextManager::getElidedString(plainSelection,Qt::ElideRight,30)));
		searchAction->setData(ADR_CONTEXT_DATA, plainSelection);
		connect(searchAction,SIGNAL(triggered(bool)),SLOT(onViewContextSearchActionTriggered(bool)));
		AMenu->addAction(searchAction,AG_MWVWCM_MESSAGEWIDGETS_SEARCH,true);
	}
}

void MessageWidgets::onViewContextCopyActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QString html = action->data(ADR_CONTEXT_DATA).toString();
		QMimeData *data = new QMimeData;
		data->setHtml(html);
		data->setText(QTextDocumentFragment::fromHtml(html).toPlainText());
		QApplication::clipboard()->setMimeData(data);
	}
}

void MessageWidgets::onViewContextUrlActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
		QDesktopServices::openUrl(action->data(ADR_CONTEXT_DATA).toString());
}

void MessageWidgets::onViewContextSearchActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		QString domain = tr("google.com","Your google domain");
		QUrl url = QString("http://www.%1/search").arg(domain);
#if QT_VERSION >= 0x050000
		QUrlQuery query;
		query.setQueryItems(QList<QPair<QString,QString> >() << qMakePair<QString,QString>(QString("q"),action->data(ADR_CONTEXT_DATA).toString()));
		url.setQuery(query);
#else
		url.setQueryItems(QList<QPair<QString,QString> >() << qMakePair<QString,QString>(QString("q"),action->data(ADR_CONTEXT_DATA).toString()));
#endif
		QDesktopServices::openUrl(url);
	}
}

void MessageWidgets::onMessageWindowWidgetLayoutChanged()
{
	IMessageWindow *window = qobject_cast<IMessageWindow *>(sender());
	if (window && window->toolBarWidget())
	{
		QAction *quoteActionHandle = window->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_MESSAGEWIDGETS_QUOTE).value(0);
		if (quoteActionHandle)
			quoteActionHandle->setVisible(window->viewWidget()->isVisibleOnWindow() && window->editWidget()->isVisibleOnWindow());

		QAction *meActionHandle = window->toolBarWidget()->toolBarChanger()->groupItems(TBG_MWTBW_MESSAGEWIDGETS_ME).value(0);
		if (meActionHandle)
			meActionHandle->setVisible(window->viewWidget()->isVisibleOnWindow() && window->editWidget()->isVisibleOnWindow());
	}
}

void MessageWidgets::onQuoteActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	IMessageWindow *window = action!=NULL ? qobject_cast<IMessageWindow *>((QWidget *)action->data(ADR_QUOTE_WINDOW).toLongLong()) : NULL;
	if (window && window->viewWidget() && window->viewWidget()->messageStyle() && window->editWidget())
	{
		QTextDocumentFragment fragment = window->viewWidget()->messageStyle()->selection(window->viewWidget()->styleWidget());
		fragment = TextManager::getTrimmedTextFragment(window->editWidget()->prepareTextFragment(fragment),!window->editWidget()->isRichTextEnabled());
		TextManager::insertQuotedFragment(window->editWidget()->textEdit()->textCursor(),fragment);
		window->editWidget()->textEdit()->setFocus();
	}
}
// *** <<< eyeCU <<< ***
void MessageWidgets::onMeActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	IMessageWindow *window = action!=NULL ? qobject_cast<IMessageWindow *>((QWidget *)action->data(ADR_QUOTE_WINDOW).toLongLong()) : NULL;
	if (window && window->viewWidget() && window->viewWidget()->messageStyle() && window->editWidget())
	{
		QTextCursor cursor = window->editWidget()->textEdit()->textCursor();
		cursor.movePosition(QTextCursor::Start);
		if (window->editWidget()->textEdit()->toPlainText().startsWith("/me "))
		{
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 4);
			cursor.deleteChar();
		}
		else
		{
			cursor.setCharFormat(QTextCharFormat());
			cursor.insertText("/me ");
		}
		window->editWidget()->textEdit()->setFocus();
	}
}

void MessageWidgets::onTextChanged()
{
	QTextEdit *textEdit = qobject_cast<QTextEdit *>(sender());
	if (textEdit)
		FMeActions.value(textEdit)->setChecked(textEdit->toPlainText().startsWith("/me "));
}

void MessageWidgets::onTextEditDestroyed(QObject *AObject)
{
	FMeActions.remove(qobject_cast<QTextEdit *>(AObject));
}
// *** >>> eyeCU >>> ***
void MessageWidgets::onAssignedTabPageDestroyed()
{
	FAssignedPages.removeAll(qobject_cast<IMessageTabPage *>(sender()));
}

void MessageWidgets::onNormalWindowDestroyed()
{
	IMessageNormalWindow *window = qobject_cast<IMessageNormalWindow *>(sender());
	if (window)
	{
		FNormalWindows.removeAt(FNormalWindows.indexOf(window));
		emit normalWindowDestroyed(window);
	}
}

void MessageWidgets::onChatWindowDestroyed()
{
	IMessageChatWindow *window = qobject_cast<IMessageChatWindow *>(sender());
	if (window)
	{
		FChatWindows.removeAt(FChatWindows.indexOf(window));
		emit chatWindowDestroyed(window);
	}
}

void MessageWidgets::onTabWindowPageAdded(IMessageTabPage *APage)
{
	if (!Options::node(OPV_MESSAGES_COMBINEWITHROSTER).value().toBool())
	{
		IMessageTabWindow *window = qobject_cast<IMessageTabWindow *>(sender());
		if (window)
		{
			if (window->windowId() != Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString())
				FPageWindows.insert(APage->tabPageId(), window->windowId());
			else
				FPageWindows.remove(APage->tabPageId());
		}
	}
}

void MessageWidgets::onTabWindowCurrentPageChanged(IMessageTabPage *APage)
{
	if (Options::node(OPV_MESSAGES_COMBINEWITHROSTER).value().toBool() && !Options::node(OPV_MESSAGES_TABWINDOWS_ENABLE).value().toBool())
	{
		IMessageTabWindow *window = qobject_cast<IMessageTabWindow *>(sender());
		if (window && window->windowId()==Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString())
		{
			for (int index=0; index<window->tabPageCount(); index++)
			{
				IMessageTabPage *page = window->tabPage(index);
				if (page != APage)
				{
					index--;
					page->closeTabPage();
				}
			}
		}
	}
}

void MessageWidgets::onTabWindowDestroyed()
{
	IMessageTabWindow *window = qobject_cast<IMessageTabWindow *>(sender());
	if (window)
	{
		FTabWindows.removeAt(FTabWindows.indexOf(window));
		emit tabWindowDestroyed(window);
	}
}

void MessageWidgets::onOptionsOpened()
{
	if (tabWindowList().isEmpty())
		appendTabWindow(tr("Main Tab Window"));

	if (!tabWindowList().contains(Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString()))
		Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).setValue(tabWindowList().value(0).toString());

	QByteArray data = Options::fileValue("messages.tab-window-pages").toByteArray();
	QDataStream stream(data);
	stream >> FPageWindows;

	onOptionsChanged(Options::node(OPV_MESSAGES_COMBINEWITHROSTER));
	onOptionsChanged(Options::node(OPV_MESSAGES_TABWINDOWS_ENABLE));
}

void MessageWidgets::onOptionsClosed()
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << FPageWindows;
	Options::setFileValue(data,"messages.tab-window-pages");

	deleteTabWindows();
}

void MessageWidgets::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_MESSAGES_TABWINDOWS_ENABLE)
	{
		if (Options::node(OPV_MESSAGES_COMBINEWITHROSTER).value().toBool())
		{
			IMessageTabWindow *window = findTabWindow(Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString());
			if (window)
				window->setTabBarVisible(ANode.value().toBool());
		}
		else if (ANode.value().toBool())
		{
			foreach(IMessageTabPage *page, FAssignedPages)
				assignTabWindowPage(page);

			foreach(IMessageTabWindow *window, tabWindows())
				window->showWindow();
		}
		else
		{
			foreach(IMessageTabWindow *window, tabWindows())
				while(window->currentTabPage())
					window->detachTabPage(window->currentTabPage());
		}
	}
	else if (FMainWindow && ANode.path()==OPV_MESSAGES_COMBINEWITHROSTER)
	{
		foreach(IMessageTabPage *page, FAssignedPages)
			assignTabWindowPage(page);

		IMessageTabWindow *window = ANode.value().toBool()
			? getTabWindow(Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString())
			: findTabWindow(Options::node(OPV_MESSAGES_TABWINDOWS_DEFAULT).value().toString());

		if (window != NULL)
		{
			if (ANode.value().toBool())
			{
				window->setTabBarVisible(Options::node(OPV_MESSAGES_TABWINDOWS_ENABLE).value().toBool());
				window->setAutoCloseEnabled(false);
				FMainWindow->mainCentralWidget()->appendCentralPage(window);
			}
			else if (Options::node(OPV_MESSAGES_TABWINDOWS_ENABLE).value().toBool())
			{
				window->setTabBarVisible(true);
				window->setAutoCloseEnabled(true);
				FMainWindow->mainCentralWidget()->removeCentralPage(window);
				if (window->tabPageCount() > 0)
					window->showWindow();
				else
					window->instance()->deleteLater();
			}
			else
			{
				while(window->currentTabPage())
					window->detachTabPage(window->currentTabPage());
				window->instance()->deleteLater();
			}
		}
	}
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_messagewidgets, MessageWidgets)
#endif

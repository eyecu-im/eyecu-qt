#include <QLayout>
#include <QBoxLayout>
#include <QMainWindow>
#include <QTextObject>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
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
#include <definitions/xhtmlicons.h>

#include <utils/textmanager.h>
#include <utils/animatedtextbrowser.h>
#include <utils/action.h>
#include <XmlTextDocumentParser>

#include "xhtmlim.h"
#include "savequery.h"
#include "resourceretriever.h"
#include "imageopenthread.h"

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
	Shortcuts::declareShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATAUTOREMOVE, tr("Toggle remove formatting on message send"), tr("Alt+A", "Toggle remove formatting on message send"), Shortcuts::WindowShortcut);
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
			addRichTextEditToolbar(ANormalWindow->messageWidgetsBox(), MCWW_RICHTEXTTOOLBARWIDGET, ANormalWindow->editWidget(), true);
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
//            addRichTextEditToolbar(AWindow->messageWidgetsBox(), MNWW_RICHTEXTTOOLBARWIDGET, AWindow->editWidget(), false);
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
	Q_UNUSED(ANode)
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

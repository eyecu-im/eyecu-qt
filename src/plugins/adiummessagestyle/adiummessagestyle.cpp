#include "adiummessagestyle.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QMimeData>
#include <QWebFrame>
#include <QTextBlock>
#include <QWebElement>
#include <QByteArray>
#include <QClipboard>
#include <QStringList>
#include <QTextCursor>
#include <QWebSettings>
#include <QDomDocument>
#include <QApplication>
#include <QTextDocument>
#include <QWebHitTestResult>
#include <definitions/resources.h>
#include <utils/filestorage.h>
#include <utils/textmanager.h>
#include <utils/logger.h>
#include <utils/pluginhelper.h>
#include <utils/qt4qt5compat.h>

#define SCROLL_TIMEOUT                      100
#define CONTENT_TIMEOUT                     10

#define CONSECUTIVE_TIMEOUT                 2*60

#define SHARED_STYLE_PATH                   RESOURCES_DIR "/" RSR_STORAGE_ADIUMMESSAGESTYLES "/" FILE_STORAGE_SHARED_DIR
#define STYLE_CONTENTS_PATH                 "Contents"
#define STYLE_RESOURCES_PATH                STYLE_CONTENTS_PATH "/Resources"

#define APPEND_MESSAGE_WITH_SCROLL          "checkIfScrollToBottomIsNeeded(); appendMessage(\"%1\"); scrollToBottomIfNeeded();"
#define APPEND_NEXT_MESSAGE_WITH_SCROLL     "checkIfScrollToBottomIsNeeded(); appendNextMessage(\"%1\"); scrollToBottomIfNeeded();"
#define APPEND_MESSAGE                      "appendMessage(\"%1\");"
#define APPEND_NEXT_MESSAGE                 "appendNextMessage(\"%1\");"
#define APPEND_MESSAGE_NO_SCROLL            "appendMessageNoScroll(\"%1\");"
#define APPEND_NEXT_MESSAGE_NO_SCROLL       "appendNextMessageNoScroll(\"%1\");"
#define REPLACE_LAST_MESSAGE                "replaceLastMessage(\"%1\");"

#define TOPIC_MAIN_DIV	                    "<div id=\"topic\"></div>"
#define TOPIC_INDIVIDUAL_WRAPPER            "<span id=\"topicEdit\" ondblclick=\"this.setAttribute('contentEditable', true); this.focus();\">%1</span>"

// *** <<< eyeCU >>> ***

QString AdiumMessageStyle::FSharedPath = QString::null;

AdiumMessageStyle::AdiumMessageStyle(const QString &AStylePath, QNetworkAccessManager *ANetworkAccessManager, QObject *AParent) : QObject(AParent)
{
	if (FSharedPath.isEmpty())
	{
		if (QDir::isRelativePath(SHARED_STYLE_PATH))
			FSharedPath = qApp->applicationDirPath() + "/" SHARED_STYLE_PATH;
		else
			FSharedPath = SHARED_STYLE_PATH;
	}

	FInfo = styleInfo(AStylePath);
	FVariants = styleVariants(AStylePath);
	FResourcePath = AStylePath + "/" STYLE_RESOURCES_PATH;
	FNetworkAccessManager = ANetworkAccessManager;
// *** <<< eyeCU <<< ***
	FConsistentColorGeneration = PluginHelper::pluginInstance<IConsistentColorGeneration>();
// *** >>> eyeCU >>> ***
	FScrollTimer.setSingleShot(true);
	connect(&FScrollTimer,SIGNAL(timeout()),SLOT(onScrollTimerTimeout()));

	FContentTimer.setSingleShot(true);
	connect(&FContentTimer,SIGNAL(timeout()),SLOT(onContentTimerTimeout()));

	connect(AParent,SIGNAL(styleWidgetAdded(IMessageStyle *, QWidget *)),SLOT(onStyleWidgetAdded(IMessageStyle *, QWidget *)));

	initStyleSettings();
	loadTemplates();
	loadSenderColors();
}

AdiumMessageStyle::~AdiumMessageStyle()
{

}

bool AdiumMessageStyle::isValid() const
{
	return !FIn_ContentHTML.isEmpty() && !styleId().isEmpty();
}

QString AdiumMessageStyle::styleId() const
{
	return FInfo.value(MSIV_NAME).toString();
}

QList<QWidget *> AdiumMessageStyle::styleWidgets() const
{
	return FWidgetStatus.keys();
}

QWidget *AdiumMessageStyle::createWidget(const IMessageStyleOptions &AOptions, QWidget *AParent)
{
	StyleViewer *view = new StyleViewer(AParent);
	if (FNetworkAccessManager)
		view->page()->setNetworkAccessManager(FNetworkAccessManager);
	changeOptions(view,AOptions,true);
	return view;
}

QString AdiumMessageStyle::senderColorById(const QString &ASenderId) const
{
	return FSenderColors.isEmpty()?FConsistentColorGeneration?FConsistentColorGeneration->calculateColor(ASenderId, float(1), float(0.5)).name()
															 :"0x808080"
								  :FSenderColors.at(qHash(ASenderId) % FSenderColors.count());
}

QTextDocumentFragment AdiumMessageStyle::selection(QWidget *AWidget) const
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	if (view && view->hasSelection())
		return QTextDocumentFragment::fromHtml(view->selectedHtml());
	return QTextDocumentFragment();
}

QTextCharFormat AdiumMessageStyle::textFormatAt(QWidget *AWidget, const QPoint &APosition) const
{
	QTextDocumentFragment fragment = textFragmentAt(AWidget,APosition);
	if (!fragment.isEmpty())
	{
		QTextDocument doc;
		QTextCursor cursor(&doc);
		cursor.insertFragment(fragment);
		cursor.setPosition(0);
		return cursor.charFormat();
	}
	return QTextCharFormat();
}

QTextDocumentFragment AdiumMessageStyle::textFragmentAt(QWidget *AWidget, const QPoint &APosition) const
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	if (view)
	{
		QWebHitTestResult result = hitTest(AWidget,APosition);
		if (result.linkUrl().isValid())
			return QTextDocumentFragment::fromHtml(result.linkElement().toOuterXml());
		else if (!result.element().isNull())
			return QTextDocumentFragment::fromHtml(result.element().toOuterXml());
		else if (!result.enclosingBlockElement().isNull())
			return QTextDocumentFragment::fromHtml(result.enclosingBlockElement().toOuterXml());
	}
	return QTextDocumentFragment();
}

bool AdiumMessageStyle::changeOptions(QWidget *AWidget, const IMessageStyleOptions &AOptions, bool AClear)
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	if (view && AOptions.styleId==styleId())
	{
		bool isNewView = !FWidgetStatus.contains(view);
		if (AClear || isNewView)
		{
			WidgetStatus &wstatus = FWidgetStatus[view];
			wstatus.ready = false;
			wstatus.failed = false;
			wstatus.lastKind = -1;
			wstatus.lastId = QString::null;
			wstatus.lastTime = QDateTime();
			wstatus.scrollStarted = false;
			wstatus.pending.clear();
			wstatus.options = AOptions.extended;

			if (isNewView)
			{
				wstatus.reset = 0;
				view->installEventFilter(this);
				connect(view,SIGNAL(linkClicked(const QUrl &)),SLOT(onLinkClicked(const QUrl &)));
				connect(view,SIGNAL(loadFinished(bool)),SLOT(onStyleWidgetLoadFinished(bool)));
				connect(view,SIGNAL(destroyed(QObject *)),SLOT(onStyleWidgetDestroyed(QObject *)));
				emit widgetAdded(view);
			}

			wstatus.reset++;
			QString html = makeStyleTemplate(AOptions);
			fillStyleKeywords(html,AOptions);
			view->setHtml(html);
		}
		else
		{
			FWidgetStatus[view].lastKind = -1;
			setVariant(view,AOptions.extended.value(MSO_VARIANT).toString());
		}

		int fontSize = AOptions.extended.value(MSO_FONT_SIZE).toInt();
		QString fontFamily = AOptions.extended.value(MSO_FONT_FAMILY).toString();
		view->page()->settings()->setFontSize(QWebSettings::DefaultFontSize, fontSize!=0 ? fontSize : QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize));
		view->page()->settings()->setFontFamily(QWebSettings::StandardFont, !fontFamily.isEmpty() ? fontFamily : QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont));

		emit optionsChanged(view,AOptions,AClear);
		return true;
	}
	else if (view == NULL)
	{
		REPORT_ERROR("Failed to change adium style options: Invalid style view");
	}
	return false;
}

bool AdiumMessageStyle::appendContent(QWidget *AWidget, const QString &AHtml, const IMessageStyleContentOptions &AOptions)
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	if (view)
	{
		WidgetStatus &wstatus = FWidgetStatus[view];
		if (!wstatus.failed)
		{
			QString content = makeContentTemplate(AOptions,wstatus);
			fillContentKeywords(content,AOptions,wstatus);

			content.replace("%message%",prepareMessage(AHtml,AOptions));
			if (AOptions.kind == IMessageStyleContentOptions::KindTopic)
				content.replace("%topic%",QString(TOPIC_INDIVIDUAL_WRAPPER).arg(AHtml));

			escapeStringForScript(content);
			QString script = scriptForAppendContent(AOptions,wstatus).arg(content);

			wstatus.lastKind = AOptions.kind;
			wstatus.lastId = AOptions.senderId;
			wstatus.lastTime = AOptions.time;
			wstatus.pending.append(script);

			FContentTimer.start(CONTENT_TIMEOUT);
			emit contentAppended(view,AHtml,AOptions);

			return true;
		}
	}
	else
	{
		REPORT_ERROR("Failed to append adium style content: Invalid view");
	}
	return false;
}

// *** <<< eyeCU <<< ***
QImage AdiumMessageStyle::imageAt(QWidget *AWidget, const QPoint &APosition) const
{
	QWebElement element(hitTest(AWidget, APosition).element());
	if (element.tagName().toUpper()=="IMG")
	{
		QImage image(element.geometry().width(), element.geometry().height(), QImage::Format_ARGB32);
		QPainter painter(&image);
		element.render(&painter);
		painter.end();
		return image;
	}
	return QImage();
}

static QWebElement findRecursively(const QWebElement &AParent, const QString &AId, const QString &ATagName=QString())
{
	for (QWebElement e = AParent.firstChild(); !e.isNull(); e=e.nextSibling())
	{
		if ((AId.isEmpty() || e.attribute("id") == AId) &&
			(ATagName.isEmpty() || e.tagName() == ATagName))
			return e;
		else
		{
			QWebElement e1 = findRecursively(e, AId, ATagName);
			if (!e1.isNull())
				return e1;
		}
	}

	return QWebElement();
}

bool AdiumMessageStyle::setImageUrl(QWidget *AWidget, const QString &AObjectId, const QString &AUrl)
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	if (view)
	{
		QWebFrame *frame = view->page()->currentFrame();
		if (frame)
		{
			QWebElement img = findRecursively(frame->documentElement(), AObjectId, "IMG");
			if (img.isNull())
				REPORT_ERROR(QString("Failed to set image name: Image with ID:%1 not found!").arg(AObjectId));
			else
			{
				img.setAttribute("src", AUrl);
				return true;
			}
		}
		else
		{
			REPORT_ERROR("Failed to set image name: No current frame");
		}
	}
	else
	{
		REPORT_ERROR("Failed to set image name: Invalid view");
	}
	return false;
}

bool AdiumMessageStyle::setObjectTitle(QWidget *AWidget, const QString &AObjectId, const QString &ATitle)
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	if (view)
	{
		QWebFrame *frame = view->page()->currentFrame();
		if (frame)
		{
			QWebElement e = findRecursively(frame->documentElement(), AObjectId);
			if (!e.isNull())
			{
				e.setAttribute("title", ATitle);
				return true;
			}
			else
			{
				REPORT_ERROR("Failed to set object title: Element with specified ID not found!");
			}
		}
		else
		{
			REPORT_ERROR("Failed to set object title: No current frame");
		}
	}
	else
	{
		REPORT_ERROR("Failed to set object title: Invalid view");
	}
	return false;
}
// *** >>> eyeCU >>> ***

int AdiumMessageStyle::version() const
{
	return FInfo.value(MSIV_VERSION,0).toInt();
}

QMap<QString, QVariant> AdiumMessageStyle::infoValues() const
{
	return FInfo;
}

QList<QString> AdiumMessageStyle::variants() const
{
	return FVariants;
}

QList<QString> AdiumMessageStyle::styleVariants(const QString &AStylePath)
{
	QList<QString> files;
	if (!AStylePath.isEmpty())
	{
		QDir dir(AStylePath + "/" STYLE_RESOURCES_PATH "/Variants");
		files = dir.entryList(QStringList("*.css"),QDir::Files,QDir::Name);
		for (int i=0; i<files.count();i++)
			files[i].chop(4);
	}
	else
	{
		REPORT_ERROR("Failed to get adium style variants: Style path is empty");
	}
	return files;
}

QMap<QString, QVariant> AdiumMessageStyle::styleInfo(const QString &AStylePath)
{
	QMap<QString, QVariant> info;
	QFile file(AStylePath + "/" STYLE_CONTENTS_PATH "/Info.plist");
	if (!AStylePath.isEmpty() && file.open(QFile::ReadOnly))
	{
		QString xmlError;
		QDomDocument doc;
		if (doc.setContent(&file,true,&xmlError))
		{
			QDomElement elem = doc.documentElement().firstChildElement("dict").firstChildElement("key");
			while (!elem.isNull())
			{
				QString key = elem.text();
				if (!key.isEmpty())
				{
					elem = elem.nextSiblingElement();
					if (elem.tagName() == "string")
						info.insert(key,elem.text());
					else if (elem.tagName() == "integer")
						info.insert(key,elem.text().toInt());
					else if (elem.tagName() == "true")
						info.insert(key,true);
					else if (elem.tagName() == "false")
						info.insert(key,false);
				}
				elem = elem.nextSiblingElement("key");
			}
		}
		else
		{
			LOG_ERROR(QString("Failed to load adium style info from file content: %1").arg(xmlError));
		}
	}
	else if (!AStylePath.isEmpty())
	{
		LOG_ERROR(QString("Failed to load adium style info from file: %1").arg(file.errorString()));
	}
	else
	{
		REPORT_ERROR("Failed to get adium style info: Style path is empty");
	}
	return info;
}

void AdiumMessageStyle::initStyleSettings()
{
	FCombineConsecutive = !FInfo.value(MSIV_DISABLE_COMBINE_CONSECUTIVE,false).toBool();
	FAllowCustomBackground = !FInfo.value(MSIV_DISABLE_CUSTOM_BACKGROUND,false).toBool();
}

void AdiumMessageStyle::loadTemplates()
{
	FIn_ContentHTML =      loadFileData(FResourcePath+"/Content.html",QString::null);

	FIn_ContentHTML =      loadFileData(FResourcePath+"/Incoming/Content.html",FIn_ContentHTML);
	FIn_NextContentHTML =  loadFileData(FResourcePath+"/Incoming/NextContent.html",FIn_ContentHTML);
	FIn_ContextHTML =      loadFileData(FResourcePath+"/Incoming/Context.html",FIn_ContentHTML);
	FIn_NextContextHTML =  loadFileData(FResourcePath+"/Incoming/NextContext.html",FIn_NextContentHTML);

	FOut_ContentHTML =     loadFileData(FResourcePath+"/Outgoing/Content.html",FIn_ContentHTML);
	FOut_NextContentHTML = loadFileData(FResourcePath+"/Outgoing/NextContent.html",FOut_ContentHTML);
	FOut_ContextHTML =     loadFileData(FResourcePath+"/Outgoing/Context.html",FOut_ContentHTML);
	FOut_NextContextHTML = loadFileData(FResourcePath+"/Outgoing/NextContext.html",FOut_NextContentHTML);

	FTopicHTML =           loadFileData(FResourcePath+"/Topic.html",QString::null);
	FStatusHTML =          loadFileData(FResourcePath+"/Status.html",FIn_ContentHTML);
	FMeCommandHTML =       loadFileData(FResourcePath+"/MeCommand.html",QString::null);
}

void AdiumMessageStyle::loadSenderColors()
{
	QFile colors(FResourcePath + "/Incoming/SenderColors.txt");
	if (colors.open(QFile::ReadOnly))
		FSenderColors = QString::fromUtf8(colors.readAll()).split(':',QString::SkipEmptyParts);
}

QString AdiumMessageStyle::loadFileData(const QString &AFileName, const QString &DefValue) const
{
	QFile file(AFileName);
	if (file.open(QFile::ReadOnly))
	{
		QByteArray html = file.readAll();
		return QString::fromUtf8(html.data(),html.size());
	}
	else if (file.exists())
	{
		LOG_ERROR(QString("Failed to load adium style data from file=%1: %2").arg(AFileName,file.errorString()));
	}
	return DefValue;
}

void AdiumMessageStyle::escapeStringForScript(QString &AText) const
{
	AText.replace("\\","\\\\");
	AText.replace("\"","\\\"");
	AText.replace("\n","");
	AText.replace("\r","<br>");
}

QString AdiumMessageStyle::makeStyleTemplate(const IMessageStyleOptions &AOptions)
{
	FUsingCustomTemplate = true;
	QString htmlFileName = FResourcePath+"/Template.html";
	if (!QFile::exists(htmlFileName))
	{
		FUsingCustomTemplate = false;
		htmlFileName = FSharedPath+"/Template.html";
	}

	QString html = loadFileData(htmlFileName,QString::null);
	if (!html.isEmpty())
	{
		QString headerHTML;
		if (AOptions.extended.value(MSO_HEADER_TYPE).toInt() == AdiumMessageStyle::HeaderTopic)
			headerHTML = TOPIC_MAIN_DIV;
		else if (AOptions.extended.value(MSO_HEADER_TYPE).toInt() == AdiumMessageStyle::HeaderNormal)
			headerHTML =  loadFileData(FResourcePath+"/Header.html",QString::null);
		QString footerHTML = loadFileData(FResourcePath+"/Footer.html",QString::null);

		QString variant = AOptions.extended.value(MSO_VARIANT).toString();
		if (!FVariants.contains(variant))
			variant = FInfo.value(MSIV_DEFAULT_VARIANT,"../main").toString();
		variant = QDir::cleanPath(QString("Variants/%1.css").arg(variant));

		html.replace(html.indexOf("%@"),2,QUrl::fromLocalFile(FResourcePath).toString()+"/");
		if (!FUsingCustomTemplate || version()>=3)
			html.replace(html.indexOf("%@"),2, version()>=3 ? "@import url( \"main.css\" );" : "");
		html.replace(html.indexOf("%@"),2,variant);
		html.replace(html.indexOf("%@"),2,headerHTML);
		html.replace(html.indexOf("%@"),2,footerHTML);
	}
	else
	{
		LOG_ERROR(QString("Failed to make adium style template, id=%1, file=%2: Template is empty").arg(styleId(),htmlFileName));
	}
	return html;
}

void AdiumMessageStyle::fillStyleKeywords(QString &AHtml, const IMessageStyleOptions &AOptions) const
{
	AHtml.replace("%chatName%",AOptions.extended.value(MSO_CHAT_NAME).toString());
	AHtml.replace("%timeOpened%",HTML_ESCAPE(AOptions.extended.value(MSO_START_DATE_TIME).toDateTime().time().toString()));
	AHtml.replace("%dateOpened%",HTML_ESCAPE(AOptions.extended.value(MSO_START_DATE_TIME).toDateTime().date().toString()));
	AHtml.replace("%sourceName%",AOptions.extended.value(MSO_ACCOUNT_NAME).toString());
	AHtml.replace("%destinationName%",AOptions.extended.value(MSO_CHAT_NAME).toString());
	AHtml.replace("%destinationDisplayName%",AOptions.extended.value(MSO_CHAT_NAME).toString());
	AHtml.replace("%outgoingIconPath%",AOptions.extended.value(MSO_SELF_AVATAR,"outgoing_icon.png").toString());
	AHtml.replace("%incomingIconPath%",AOptions.extended.value(MSO_CONTACT_AVATAR,"incoming_icon.png").toString());
	AHtml.replace("%outgoingColor%",AOptions.extended.value(MSO_SELF_COLOR).toString());
	AHtml.replace("%incomingColor%",AOptions.extended.value(MSO_CONTACT_COLOR).toString());
	AHtml.replace("%serviceIconPath%", AOptions.extended.value(MSO_SERVICE_ICON_PATH).toString());
	AHtml.replace("%serviceIconImg%", QString("<img class=\"serviceIcon\" src=\"%1\">").arg(AOptions.extended.value(MSO_SERVICE_ICON_PATH,"outgoing_icon.png").toString()));

	QString background;
	if (FAllowCustomBackground)
	{
		if (!AOptions.extended.value(MSO_BG_IMAGE_FILE).toString().isEmpty())
		{
			int imageLayout = AOptions.extended.value(MSO_BG_IMAGE_LAYOUT).toInt();
			if (imageLayout == ImageNormal)
				background.append("background-image: url('%1'); background-repeat: no-repeat; background-attachment:fixed;");
			else if (imageLayout == ImageCenter)
				background.append("background-image: url('%1'); background-position: center; background-repeat: no-repeat; background-attachment:fixed;");
			else if (imageLayout == ImageTitle)
				background.append("background-image: url('%1'); background-repeat: repeat;");
			else if (imageLayout == ImageTitleCenter)
				background.append("background-image: url('%1'); background-repeat: repeat; background-position: center;");
			else if (imageLayout == ImageScale)
				background.append("background-image: url('%1'); -webkit-background-size: 100% 100%; background-size: 100% 100%; background-attachment: fixed;");
			background = background.arg(AOptions.extended.value(MSO_BG_IMAGE_FILE).toString());
		}
		if (!AOptions.extended.value(MSO_BG_COLOR).toString().isEmpty())
		{
			QColor color(AOptions.extended.value(MSO_BG_COLOR).toString());
			if (!color.isValid())
				color.setNamedColor("#"+AOptions.extended.value(MSO_BG_COLOR).toString());
			if (color.isValid())
			{
				int r,g,b,a;
				color.getRgb(&r,&g,&b,&a);
				background.append(QString("background-color: rgba(%1, %2, %3, %4);").arg(r).arg(g).arg(b).arg(qreal(a)/255.0));
			}
		}
	}
	AHtml.replace("==bodyBackground==", background);
}

void AdiumMessageStyle::setVariant(StyleViewer *AView, const QString &AVariant)
{
	QString variantName = !FVariants.contains(AVariant) ? FInfo.value(MSIV_DEFAULT_VARIANT,QString("../main")).toString() : AVariant;
	QString variantFile = QDir::cleanPath(QString("Variants/%1.css").arg(variantName));

	escapeStringForScript(variantFile);
	QString script = QString("setStylesheet(\"%1\",\"%2\");").arg("mainStyle").arg(variantFile);

	AView->page()->mainFrame()->evaluateJavaScript(script);
}

QWebHitTestResult AdiumMessageStyle::hitTest(QWidget *AWidget, const QPoint &APosition) const
{
	StyleViewer *view = qobject_cast<StyleViewer *>(AWidget);
	QWebFrame *frame = view!=NULL ? view->page()->frameAt(APosition) : NULL;
	return frame!=NULL ? frame->hitTestContent(APosition) : QWebHitTestResult();
}

bool AdiumMessageStyle::isConsecutive(const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const
{
	if (!FCombineConsecutive)
		return false;

	if (AOptions.kind != IMessageStyleContentOptions::KindMessage)
		return false;
	if (AOptions.senderId.isEmpty())
		return false;

	if (AStatus.lastKind != AOptions.kind)
		return false;
	if (AStatus.lastId != AOptions.senderId)
		return false;
	if (AStatus.lastTime.secsTo(AOptions.time) > CONSECUTIVE_TIMEOUT)
		return false;

	return true;
}

QString AdiumMessageStyle::makeContentTemplate(const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const
{
	QString html;
	if (false && AOptions.kind == IMessageStyleContentOptions::KindTopic && !FTopicHTML.isEmpty())
	{
		html = FTopicHTML;
	}
	else if (AOptions.kind == IMessageStyleContentOptions::KindStatus && !FStatusHTML.isEmpty())
	{
		html = FStatusHTML;
	}
	else if (AOptions.kind==IMessageStyleContentOptions::KindMeCommand && (!FMeCommandHTML.isEmpty() || !FStatusHTML.isEmpty()))
	{
		html = !FMeCommandHTML.isEmpty() ? FMeCommandHTML : FStatusHTML;
	}
	else
	{
		bool consecutive = isConsecutive(AOptions,AStatus);
		if (AOptions.type & IMessageStyleContentOptions::TypeHistory)
		{
			if (AOptions.direction == IMessageStyleContentOptions::DirectionIn)
				html = consecutive ? FIn_NextContextHTML : FIn_ContextHTML;
			else
				html = consecutive ? FOut_NextContextHTML : FOut_ContextHTML;
		}
		else if (AOptions.direction == IMessageStyleContentOptions::DirectionIn)
		{
			html = consecutive ? FIn_NextContentHTML : FIn_ContentHTML;
		}
		else
		{
			html = consecutive ? FOut_NextContentHTML : FOut_ContentHTML;
		}
	}
	return html;
}

void AdiumMessageStyle::fillContentKeywords(QString &AHtml, const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const
{
	bool isDirectionIn = AOptions.direction==IMessageStyleContentOptions::DirectionIn;

	QStringList messageClasses;
	if (isConsecutive(AOptions,AStatus))
		messageClasses << MSMC_CONSECUTIVE;

	if (AOptions.kind==IMessageStyleContentOptions::KindMeCommand)
		messageClasses << (!FMeCommandHTML.isEmpty() ? MSMC_MECOMMAND : MSMC_STATUS);
	else if (AOptions.kind == IMessageStyleContentOptions::KindStatus)
		messageClasses << MSMC_STATUS;
	else
		messageClasses << MSMC_MESSAGE;

	if (isDirectionIn)
		messageClasses << MSMC_INCOMING;
	else
		messageClasses << MSMC_OUTGOING;

	if (AOptions.type & IMessageStyleContentOptions::TypeGroupchat)
		messageClasses << MSMC_GROUPCHAT;
	if (AOptions.type & IMessageStyleContentOptions::TypeHistory)
		messageClasses << MSMC_HISTORY;
	if (AOptions.type & IMessageStyleContentOptions::TypeEvent)
		messageClasses << MSMC_EVENT;
	if (AOptions.type & IMessageStyleContentOptions::TypeMention)
		messageClasses << MSMC_MENTION;
	if (AOptions.type & IMessageStyleContentOptions::TypeNotification)
		messageClasses << MSMC_NOTIFICATION;

	QString messageStatus;
	if (AOptions.status == IMessageStyleContentOptions::StatusOnline)
		messageStatus = MSSK_ONLINE;
	else if (AOptions.status == IMessageStyleContentOptions::StatusOffline)
		messageStatus = MSSK_OFFLINE;
	else if (AOptions.status == IMessageStyleContentOptions::StatusAway)
		messageStatus = MSSK_AWAY;
	else if (AOptions.status == IMessageStyleContentOptions::StatusAwayMessage)
		messageStatus = MSSK_AWAY_MESSAGE;
	else if (AOptions.status == IMessageStyleContentOptions::StatusReturnAway)
		messageStatus = MSSK_RETURN_AWAY;
	else if (AOptions.status == IMessageStyleContentOptions::StatusIdle)
		messageStatus = MSSK_IDLE;
	else if (AOptions.status == IMessageStyleContentOptions::StatusReturnIdle)
		messageStatus = MSSK_RETURN_IDLE;
	else if (AOptions.status == IMessageStyleContentOptions::StatusDateSeparator)
		messageStatus = MSSK_DATE_SEPARATOR;
	else if (AOptions.status == IMessageStyleContentOptions::StatusJoined)
		messageStatus = MSSK_CONTACT_JOINED;
	else if (AOptions.status == IMessageStyleContentOptions::StatusLeft)
		messageStatus = MSSK_CONTACT_LEFT;
	else if (AOptions.status == IMessageStyleContentOptions::StatusError)
		messageStatus = MSSK_ERROR;
	else if (AOptions.status == IMessageStyleContentOptions::StatusTimeout)
		messageStatus = MSSK_TIMED_OUT;
	else if (AOptions.status == IMessageStyleContentOptions::StatusEncryption)
		messageStatus = MSSK_ENCRYPTION;
	else if (AOptions.status == IMessageStyleContentOptions::StatusFileTransferBegan)
		messageStatus = MSSK_FILETRANSFER_BEGAN;
	else if (AOptions.status == IMessageStyleContentOptions::StatusFileTransferComplete)
		messageStatus = MSSK_FILETRANSFER_COMPLETE;
	if (!messageStatus.isEmpty())
		messageClasses << messageStatus;

	AHtml.replace("%messageClasses%", messageClasses.join(" "));

	//AHtml.replace("%messageDirection%", AOptions.isAlignLTR ? "ltr" : "rtl" );
	AHtml.replace("%senderStatusIcon%",AOptions.senderIcon);
	AHtml.replace("%shortTime%", HTML_ESCAPE(AOptions.time.toString(tr("hh:mm"))));
	AHtml.replace("%service%",QString::null);

	QString avatar = AOptions.senderAvatar;
	if (!QFile::exists(avatar))
	{
		avatar = isDirectionIn ? "Incoming/buddy_icon.png" : "Outgoing/buddy_icon.png";
		if (!isDirectionIn && !QFile::exists(FResourcePath+"/"+avatar))
			avatar = "Incoming/buddy_icon.png";
	}
	AHtml.replace("%userIconPath%",QUrl::fromLocalFile(avatar).toString()); // *** <<< eyeCU >>> ***
	QString timeFormat = !AOptions.timeFormat.isEmpty() ? AOptions.timeFormat : tr("hh:mm:ss");
	QString time = HTML_ESCAPE(AOptions.time.toString(timeFormat));
	AHtml.replace("%time%", time);

	QRegExp timeRegExp("%time\\{([^}]*)\\}%");
	for (int pos=0; pos!=-1; pos = timeRegExp.indexIn(AHtml, pos))
		if (!timeRegExp.cap(0).isEmpty())
			AHtml.replace(pos, timeRegExp.cap(0).length(), time);

	QString senderColor = AOptions.senderColor;
	if (senderColor.isEmpty())
	{
		if (isDirectionIn)
			senderColor = AStatus.options.value(MSO_CONTACT_COLOR).toString();
		else
			senderColor = AStatus.options.value(MSO_SELF_COLOR).toString();
	}
	AHtml.replace("%senderColor%",senderColor);

	QRegExp scolorRegExp("%senderColor\\{([^}]*)\\}%");
	for (int pos=0; pos!=-1; pos = scolorRegExp.indexIn(AHtml, pos))
		if (!scolorRegExp.cap(0).isEmpty())
			AHtml.replace(pos, scolorRegExp.cap(0).length(), senderColor);

	if (AOptions.kind == IMessageStyleContentOptions::KindStatus)
	{
		AHtml.replace("%status%",messageStatus);
		AHtml.replace("%statusSender%",AOptions.senderName);
	}
	else
	{
		QString linkedSenderName = AOptions.senderNameLinked.isEmpty()?AOptions.senderName:AOptions.senderNameLinked;
// *** <<< eyeCU <<< ***
		AHtml.replace("%senderScreenName%", QString::null);
		TextManager::substituteHtmlText(AHtml, "%sender%", AOptions.senderName, linkedSenderName);
		TextManager::substituteHtmlText(AHtml, "%senderDisplayName%", AOptions.senderName, linkedSenderName);
// *** >>> eyeCU >>> ***
		AHtml.replace("%senderPrefix%",QString::null);

		QString rgbaColor;
		QColor bgColor(AOptions.textBGColor);
		QRegExp colorRegExp("%textbackgroundcolor\\{([^}]*)\\}%");
		for (int pos=0; pos!=-1; pos = colorRegExp.indexIn(AHtml, pos))
		{
			if (!colorRegExp.cap(0).isEmpty())
			{
				if (bgColor.isValid())
				{
					int r,g,b;
					bool ok = false;
					qreal a = colorRegExp.cap(1).toDouble(&ok);
					bgColor.setAlphaF(ok ? a : 1.0);
					bgColor.getRgb(&r,&g,&b);
					rgbaColor = QString("rgba(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(a);
				}
				else if (rgbaColor.isEmpty())
				{
					rgbaColor = "inherit";
				}
				AHtml.replace(pos, colorRegExp.cap(0).length(), rgbaColor);
			}
		}
	}
}

QString AdiumMessageStyle::scriptForAppendContent(const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const
{
	QString script;
	bool consecutive = isConsecutive(AOptions,AStatus);
	if (!FUsingCustomTemplate && version() >= 4)
	{
		if (AOptions.noScroll)
			script = consecutive ? APPEND_NEXT_MESSAGE_NO_SCROLL : APPEND_MESSAGE_NO_SCROLL;
		else
			script = consecutive ? APPEND_NEXT_MESSAGE : APPEND_MESSAGE;
	}
	else if (version() >= 3)
	{
		if (AOptions.noScroll)
			script = consecutive ? APPEND_NEXT_MESSAGE_NO_SCROLL : APPEND_MESSAGE_NO_SCROLL;
		else
			script = consecutive ? APPEND_NEXT_MESSAGE : APPEND_MESSAGE;
	}
	else if (version() >= 1)
	{
		script = consecutive ? APPEND_NEXT_MESSAGE : APPEND_MESSAGE;
	}
	else
	{
		if (FUsingCustomTemplate)
			script = consecutive ? APPEND_NEXT_MESSAGE_WITH_SCROLL : APPEND_MESSAGE_WITH_SCROLL;
		else
			script = consecutive ? APPEND_NEXT_MESSAGE : APPEND_MESSAGE;
	}
	return script;
}

QString AdiumMessageStyle::prepareMessage(const QString &AHtml, const IMessageStyleContentOptions &AOptions) const
{
	if (AOptions.kind==IMessageStyleContentOptions::KindMeCommand && FMeCommandHTML.isEmpty())
	{
		QTextDocument doc;
		doc.setHtml(AHtml);
		QTextCursor cursor(&doc);
		cursor.insertHtml(QString("<i>*&nbsp;%1</i>&nbsp;").arg(AOptions.senderName));
		return TextManager::getDocumentBody(doc);
	}
	return AHtml;
}

bool AdiumMessageStyle::eventFilter(QObject *AWatched, QEvent *AEvent)
{
	if (AEvent->type() == QEvent::Resize)
	{
		StyleViewer *view = qobject_cast<StyleViewer *>(AWatched);
		if (FWidgetStatus.contains(view))
		{
			WidgetStatus &wstatus = FWidgetStatus[view];
			QWebFrame *frame = view->page()->mainFrame();
			if (!wstatus.scrollStarted && frame->scrollBarValue(Qt::Vertical)==frame->scrollBarMaximum(Qt::Vertical))
			{
				wstatus.scrollStarted = true;
				FScrollTimer.start(SCROLL_TIMEOUT);
			}
		}
	}
	return QObject::eventFilter(AWatched,AEvent);
}


void AdiumMessageStyle::onScrollTimerTimeout()
{
	for (QMap<QWidget*,WidgetStatus>::iterator it = FWidgetStatus.begin(); it!= FWidgetStatus.end(); ++it)
	{
		if (it->scrollStarted)
		{
			StyleViewer *view = qobject_cast<StyleViewer *>(it.key());
			QWebFrame *frame = view->page()->mainFrame();
			frame->evaluateJavaScript("alignChat(false);");
			frame->setScrollBarValue(Qt::Vertical,frame->scrollBarMaximum(Qt::Vertical));
			it->scrollStarted = false;
		}
	}
}

void AdiumMessageStyle::onContentTimerTimeout()
{
	for(QMap<QWidget *, WidgetStatus>::iterator it=FWidgetStatus.begin(); it!=FWidgetStatus.end(); ++it)
	{
		if (it->ready && !it->pending.isEmpty())
		{
			StyleViewer *view = qobject_cast<StyleViewer *>(it.key());
			view->page()->mainFrame()->evaluateJavaScript(it->pending.takeFirst());
			FContentTimer.start(CONTENT_TIMEOUT);
		}
	}
}

void AdiumMessageStyle::onLinkClicked(const QUrl &AUrl)
{
	StyleViewer *view = qobject_cast<StyleViewer *>(sender());
	emit urlClicked(view,AUrl);
}

void AdiumMessageStyle::onStyleWidgetAdded(IMessageStyle *AStyle, QWidget *AWidget)
{
	if (AStyle!=this && FWidgetStatus.contains(AWidget))
	{
		AWidget->removeEventFilter(this);
		FWidgetStatus.remove(AWidget);
		emit widgetRemoved(AWidget);
	}
}

void AdiumMessageStyle::onStyleWidgetDestroyed(QObject *AObject)
{
	FWidgetStatus.remove((QWidget *)AObject);
	emit widgetRemoved((QWidget *)AObject);
}

void AdiumMessageStyle::onStyleWidgetLoadFinished(bool AOk)
{
	StyleViewer *view = qobject_cast<StyleViewer *>(sender());
	if (view)
	{
		WidgetStatus &wstatus = FWidgetStatus[view];
		if (--wstatus.reset == 0)
		{
			if (AOk)
			{
				wstatus.ready = true;
				FContentTimer.start(CONTENT_TIMEOUT);
				view->page()->mainFrame()->evaluateJavaScript("alignChat(false);");
			}
			else
			{
				wstatus.failed = true;
				view->setHtml(QString("<html><body>%1</body></html>").arg(tr("Failed to load message style. Press clear window button to retry.")));
				REPORT_ERROR(QString("Failed to load adium style template, styleId=%1").arg(styleId()));
			}
		}
		else if (wstatus.reset < 0)
		{
			wstatus.reset = 0;
		}
	}
}

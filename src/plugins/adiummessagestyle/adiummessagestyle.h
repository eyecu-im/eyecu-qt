#ifndef ADIUMMESSAGESTYLE_H
#define ADIUMMESSAGESTYLE_H

#include <QList>
#include <QTimer>
#include <QWebView>
#include <QNetworkAccessManager>
#include <interfaces/imessagestylemanager.h>
#include <interfaces/iconsistentcolorgeneration.h>
#include "styleviewer.h"

//Message Style Info Values
#define MSIV_VERSION                        "MessageViewVersion"
#define MSIV_NAME                           "CFBundleName"
#define MSIV_IDENTIFIER                     "CFBundleIdentifier"
#define MSIV_INFO_STRING                    "CFBundleGetInfoString"
#define MSIV_DEFAULT_VARIANT                "DefaultVariant"
#define MSIV_NO_VARIANT_NAME                "DisplayNameForNoVariant"
#define MSIV_DEFAULT_FONT_FAMILY            "DefaultFontFamily"
#define MSIV_DEFAULT_FONT_SIZE              "DefaultFontSize"
#define MSIV_SHOW_USER_ICONS                "ShowsUserIcons"
#define MSIV_ALLOW_TEXT_COLORS              "AllowTextColors"
#define MSIV_DEFAULT_SELF_COLOR             "DefaultSelfColor"
#define MSIV_DEFAULT_CONTACT_COLOR          "DefaultContactColor"
#define MSIV_DEFAULT_BACKGROUND_COLOR       "DefaultBackgroundColor"
#define MSIV_DISABLE_CUSTOM_BACKGROUND      "DisableCustomBackground"
#define MSIV_DISABLE_COMBINE_CONSECUTIVE    "DisableCombineConsecutive"
#define MSIV_BACKGROUND_TRANSPARENT         "DefaultBackgroundIsTransparent"
#define MSIV_IMAGE_MASK                     "ImageMask"

//Message Status Message Classes
#define MSMC_MESSAGE                        "message"
#define MSMC_EVENT                          "event"
#define MSMC_STATUS                         "status"
#define MSMC_NOTIFICATION                   "notification"
#define MSMC_HISTORY                        "history"
#define MSMC_CONSECUTIVE                    "consecutive"
#define MSMC_OUTGOING                       "outgoing"
#define MSMC_INCOMING                       "incoming"
#define MSMC_GROUPCHAT                      "groupchat"
#define MSMC_MENTION                        "mention"
#define MSMC_AUTOREPLY                      "autoreply"
#define MSMC_MECOMMAND                      "me_command"

//Message Style Status Keywords
#define MSSK_ONLINE                         "online"
#define MSSK_OFFLINE                        "offline"
#define MSSK_AWAY                           "away"
#define MSSK_AWAY_MESSAGE                   "away_message"
#define MSSK_RETURN_AWAY                    "return_away"
#define MSSK_IDLE                           "idle"
#define MSSK_RETURN_IDLE                    "return_idle"
#define MSSK_DATE_SEPARATOR                 "date_separator"
#define MSSK_CONTACT_JOINED                 "contact_joined"
#define MSSK_CONTACT_LEFT                   "contact_left"
#define MSSK_ERROR                          "error"
#define MSSK_TIMED_OUT                      "timed_out"
#define MSSK_ENCRYPTION                     "encryption"
#define MSSK_FILETRANSFER_BEGAN             "fileTransferBegan"
#define MSSK_FILETRANSFER_COMPLETE          "fileTransferComplete"

//Message Style Options
#define MSO_VARIANT                         "variant"
#define MSO_FONT_FAMILY                     "fontFamily"
#define MSO_FONT_SIZE                       "fontSize"
#define MSO_HEADER_TYPE                     "headerType"
#define MSO_CHAT_NAME                       "chatName"
#define MSO_ACCOUNT_NAME                    "accountName"
#define MSO_START_DATE_TIME                 "startDateTime"
#define MSO_SELF_NAME                       "selfName"
#define MSO_SELF_AVATAR                     "selfAvatar"
#define MSO_SELF_COLOR                      "selfColor"
#define MSO_CONTACT_NAME                    "contactName"
#define MSO_CONTACT_AVATAR                  "contactAvatar"
#define MSO_CONTACT_COLOR                   "contactColor"
#define MSO_SERVICE_ICON_PATH               "serviceIconPath"
#define MSO_BG_COLOR                        "bgColor"
#define MSO_BG_IMAGE_FILE                   "bgImageFile"
#define MSO_BG_IMAGE_LAYOUT                 "bgImageLayout"

class AdiumMessageStyle :
	public QObject,
	public IMessageStyle
{
	Q_OBJECT;
	Q_INTERFACES(IMessageStyle);
public:
	enum HeaderType {
		HeaderNone,
		HeaderNormal,
		HeaderTopic
	};
	enum BGImageLayout {
		ImageNormal,
		ImageCenter,
		ImageTitle,
		ImageTitleCenter,
		ImageScale
	};
	struct WidgetStatus {
		int reset;
		bool ready;
		bool failed;
		int lastKind;
		QString lastId;
		QDateTime lastTime;
		bool scrollStarted;
		QList<QString> pending;
		QMap<QString, QVariant> options;
	};
public:
	AdiumMessageStyle(const QString &AStylePath, QNetworkAccessManager *ANetworkAccessManager, QObject *AParent);
	~AdiumMessageStyle();
	//IMessageStyle
	virtual QObject *instance() { return this; }
	virtual bool isValid() const;
	virtual QString styleId() const;
	virtual QList<QWidget *> styleWidgets() const;
	virtual QWidget *createWidget(const IMessageStyleOptions &AOptions, QWidget *AParent);
	virtual QString senderColorById(const QString &ASenderId) const;
	virtual QTextDocumentFragment selection(QWidget *AWidget) const;
	virtual QTextCharFormat textFormatAt(QWidget *AWidget, const QPoint &APosition) const;
	virtual QTextDocumentFragment textFragmentAt(QWidget *AWidget, const QPoint &APosition) const;    
	virtual bool changeOptions(QWidget *AWidget, const IMessageStyleOptions &AOptions, bool AClear = true);
	virtual bool appendContent(QWidget *AWidget, const QString &AHtml, const IMessageStyleContentOptions &AOptions);
// *** <<< eyeCU <<< ***
	virtual QImage imageAt(QWidget *AWidget, const QPoint &APosition) const;
	virtual bool setImageUrl(QWidget *AWidget, const QString &AObjectId, const QString &AUrl);
	virtual bool setObjectTitle(QWidget *AWidget, const QString &AObjectId, const QString &ATitle);
// *** >>> eyeCU >>> ***
	//AdiumMessageStyle
	virtual int version() const;
	virtual QMap<QString, QVariant> infoValues() const;
	virtual QList<QString> variants() const;
signals:
	void widgetAdded(QWidget *AWidget) const;
	void widgetRemoved(QWidget *AWidget) const;
	void optionsChanged(QWidget *AWidget, const IMessageStyleOptions &AOptions, bool AClear) const;
	void contentAppended(QWidget *AWidget, const QString &AHtml, const IMessageStyleContentOptions &AOptions) const;
	void urlClicked(QWidget *AWidget, const QUrl &AUrl) const;
public:
	static QList<QString> styleVariants(const QString &AStylePath);
	static QMap<QString, QVariant> styleInfo(const QString &AStylePath);
protected:
	void initStyleSettings();
	void loadTemplates();
	void loadSenderColors();
	QString loadFileData(const QString &AFileName, const QString &DefValue) const;
protected:
	void escapeStringForScript(QString &AText) const;
	QString makeStyleTemplate(const IMessageStyleOptions &AOptions);
	void fillStyleKeywords(QString &AHtml, const IMessageStyleOptions &AOptions) const;
	void setVariant(StyleViewer *AView, const QString &AVariant);
protected:
	QWebHitTestResult hitTest(QWidget *AWidget, const QPoint &APosition) const;
	bool isConsecutive(const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const;
	QString makeContentTemplate(const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const;
	void fillContentKeywords(QString &AHtml, const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const;
	QString scriptForAppendContent(const IMessageStyleContentOptions &AOptions, const WidgetStatus &AStatus) const;
	QString prepareMessage(const QString &AHtml, const IMessageStyleContentOptions &AOptions) const;
protected:
	bool eventFilter(QObject *AWatched, QEvent *AEvent);
protected slots:
	void onScrollTimerTimeout();
	void onContentTimerTimeout();
	void onLinkClicked(const QUrl &AUrl);
	void onStyleWidgetAdded(IMessageStyle *AStyle, QWidget *AWidget);
	void onStyleWidgetLoadFinished(bool AOk);
	void onStyleWidgetDestroyed(QObject *AObject);
private:
	QTimer FScrollTimer;
	QTimer FContentTimer;
	bool FCombineConsecutive;
	bool FUsingCustomTemplate;
	bool FAllowCustomBackground;
private:
	QString FTopicHTML;
	QString FStatusHTML;
	QString FMeCommandHTML;
	QString FIn_ContentHTML;
	QString FIn_NextContentHTML;
	QString FIn_ContextHTML;
	QString FIn_NextContextHTML;
	QString FOut_ContentHTML;
	QString FOut_NextContentHTML;
	QString FOut_ContextHTML;
	QString FOut_NextContextHTML;
private:
	QString FResourcePath;
	QList<QString> FVariants;
	QList<QString> FSenderColors;
	QMap<QString, QVariant> FInfo;
	QMap<QWidget *, WidgetStatus> FWidgetStatus;
	QNetworkAccessManager *FNetworkAccessManager;
	IConsistentColorGeneration *FConsistentColorGeneration; // *** <<< eyeCU >>> ***
private:
	static QString FSharedPath;
};

#endif // ADIUMMESSAGESTYLE_H

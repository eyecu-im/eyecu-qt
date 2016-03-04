#ifndef EDITHTML_H
#define EDITHTML_H

#include <QFontComboBox>
#include <QTextFragment>
#include <ColorToolButton>
#include <QNetworkAccessManager>

#include <interfaces/imessagewidgets.h>
#include <interfaces/ibitsofbinary.h>

#include <definitions/optionvalues.h>

#include <utils/options.h>

class XhtmlIm;

class EditHtml: public QToolBar
{
    Q_OBJECT
public:
	EditHtml(IMessageEditWidget *AEditWidget, bool AEnableFormatAutoReset, IBitsOfBinary *BOB, QNetworkAccessManager *ANetworkAccessManager, XhtmlIm *AXhtmlIm, QWidget *parent=NULL);
	ToolBarChanger *changer() const {return FToolBarChanger;}

private:
	ToolBarChanger		*FToolBarChanger;
	IMessageEditWidget	*FEditWidget;
	QTextEdit			*FTextEdit;
	IconStorage			*FIconStorage;
	IBitsOfBinary		*FBitsOfBinary;
	XhtmlIm				*FXhtmlIm;

private:
	QFontComboBox		*FCmbFont;
	QComboBox			*FCmbFontSize;

	Action  *FActionAutoRemoveFormat,
			*FActionRemoveFormat,
            *FActionInsertImage,
            *FActionInsertLink,
            *FActionInsertNewline,
            *FActionInsertNbsp,
            *FActionSetTitle,
            *FActionTextBold,
            *FActionTextItalic,
            *FActionTextUnderline,
            *FActionTextOverline,
            *FActionTextStrikeout,
            *FActionTextCode,
            *FActionIndentMore,
            *FActionIndentLess;

	Menu	*FMenuCapitalization;
	Action  *FActionMixed;
	Action	*FActionSmallCaps;
	Action	*FActionAllUppercase;
	Action	*FActionAllLowercase;
	Action	*FActionCapitalize;

    Menu    *FMenuAlign;
    Action  *FActionLastAlign;

    Menu    *FMenuList;
    Action  *FActionLastList;

    Menu    *FMenuFormat;
    Action  *FActionLastFormat;

    ColorToolButton *FColorToolButton;
    QNetworkAccessManager *FNetworkAccessManager;
    QTextList *FCurrentList;
    int       FCurrentListItemCount;
    int       FCurrentListItemNumber;
    int       FCurrentPosition;
    int       FCurrentCharacterCount;
    int       FCurrentIndent;
    bool      FTextChangedProcessing;
    bool      FNewListItemCreated;

protected:
    void setupFontActions(bool AEnableReset);
    void setupTextActions();
    void fontChanged(const QTextCharFormat &ACharFormat);

    void setupFileActions();
    void mergeFormatOnWordOrSelection(const QTextCharFormat &AFormat);
    void clearFormatOnWordOrSelection(bool AWholeDocument=false);

    void selectFont();
    void selectForegroundColor();
    void selectBackgroundColor();
    void updateCurrentList(const QTextCursor &ACursor);
    void updateCurrentBlock(const QTextCursor &ACursor);

    static QTextFragment getTextFragment(const QTextCursor &ACursor);
    static void selectTextFragment(QTextCursor &ACursor);

protected slots:
    void onResetFormat(bool AStatus);
    void onRemoveFormat();
    void onInsertImage();
    void onSetToolTip();
    void onInsertLink();
	void onSelectDecoration(bool ASelected);
	void onSelectCapitalization();
    void onTextCode(bool AChecked);
    void onCurrentFontFamilyChanged(const QFont &AFont);
    void onCurrentFontSizeChanged(const QString &ASize);
    void onInsertList();
    void onSetFormat();
    void onInsertSpecial();
    void onTextAlign();
    void onColorClicked(bool ABackground);
    void onIndentChange();

    void onTextChanged();
    void onCursorPositionChanged();
    void onCurrentCharFormatChanged(const QTextCharFormat &ACharFormat);

	void onShortcutActivated(const QString &AId, QWidget *AWidget);
    void onMessageSent();

protected slots:
	void onOptionsChanged(const OptionsNode &ANode);
};

#endif // EDITHTML_H

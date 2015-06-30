#ifndef EDITHTML_H
#define EDITHTML_H

#include <QFontComboBox>
#include <QTextFragment>
#include <ColorToolButton>

#include <interfaces/imessagewidgets.h>
#include <interfaces/ibitsofbinary.h>

#include <definitions/optionvalues.h>

#include <utils/options.h>

#include "insertimage.h"
#include "addlink.h"

class XhtmlIm;

class EditHtml :
    public QToolBar
{
    Q_OBJECT
public:
	EditHtml(IMessageEditWidget *AEditWidget, bool AEnableFormatAutoReset, IBitsOfBinary *BOB, QNetworkAccessManager *ANetworkAccessManager, XhtmlIm *AXhtmlIm, QWidget *parent=NULL);

private:
	IMessageEditWidget	*FEditWidget;
	QTextEdit			*FTextEdit;
	IconStorage			*FIconStorage;
	IBitsOfBinary		*FBitsOfBinary;
	XhtmlIm				*FXhtmlIm;

private:
    QFontComboBox *FCmbFont;
    QComboBox     *FCmbSize;

    Action  *FActionResetFormat,
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

    Menu    *FMenuSpecial;

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

    static int  checkBlockFormat(const QTextCursor &ACursor);
    static void clearBlockProperties(const QTextBlock &ATextBlock, const QSet<QTextFormat::Property> &AProperties);
    static QTextFragment getTextFragment(const QTextCursor &ACursor);
    static void selectTextFragment(QTextCursor &ACursor);

protected slots:
    void onResetFormat(bool AStatus);
    void onRemoveFormat();
    void onInsertImage();
    void onSetToolTip();
    void onInsertLink();
    void onTextBold();
    void onTextItalic();
    void onTextUnderline();
    void onTextOverline();
    void onTextStrikeout();
    void onTextCode();
    void onCurrentFontFamilyChanged(const QFont &AFont);
//    void onCurrentFontIndexChanged(int AIndex);
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
};

#endif // EDITHTML_H

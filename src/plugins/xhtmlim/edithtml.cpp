#include <QPainter>

#include <QImageReader>
#include <QImageWriter>
#include <QColorDialog>
#include <QTextList>
#include <QTextImageFormat>
#include <QFontDialog>
#include <QBuffer>
#include <XmlTextDocumentParser>

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/xhtmlicons.h>
#include <definitions/shortcuts.h>
#include <definitions/actiongroups.h>
#include <definitions/optionvalues.h>

#include "utils/action.h"

#include "edithtml.h"
#include "xhtmlim.h"
#include "settooltip.h"

#define ADR_ALIGN_TYPE      Action::DR_Parametr1
#define ADR_LIST_TYPE       Action::DR_Parametr1
#define ADR_FORMATTING_TYPE Action::DR_Parametr1
#define ADR_SPECIAL_SYMBOL  Action::DR_Parametr1

#define FMT_NORMAL          0
#define FMT_HEADING1        1
#define FMT_HEADING2        2
#define FMT_HEADING3        3
#define FMT_HEADING4        4
#define FMT_HEADING5        5
#define FMT_HEADING6        6
#define FMT_PREFORMAT       7

EditHtml::EditHtml(IMessageEditWidget *AEditWidget, bool AEnableFormatAutoReset, IBitsOfBinary *BOB , QNetworkAccessManager *ANetworkAccessManager, XhtmlIm *AXhtmlIm, QWidget *parent) :
    QToolBar(parent),
    FEditWidget(AEditWidget),
    FTextEdit(AEditWidget->textEdit()),
    FIconStorage(NULL),
    FBitsOfBinary(BOB),
	FXhtmlIm(AXhtmlIm),
    FNetworkAccessManager(ANetworkAccessManager),
    FCurrentList(NULL),
    FCurrentListItemCount(0),
    FCurrentListItemNumber(0),
    FCurrentPosition(0),
    FCurrentCharacterCount(0),
    FCurrentIndent(0),
    FTextChangedProcessing(false),
    FNewListItemCreated(false)
{
    setIconSize(QSize(16,16));
    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_HTML);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    setupFontActions(AEnableFormatAutoReset);
    setupTextActions();

    AEditWidget->setRichTextEnabled(true); // When XHTML enabled, it should accept rich text!
    AEditWidget->textEdit()->acceptDrops();
    connect(FTextEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)), SLOT(onCurrentCharFormatChanged(QTextCharFormat)));
    connect(FTextEdit, SIGNAL(textChanged()), SLOT(onTextChanged()));
    connect(FTextEdit, SIGNAL(cursorPositionChanged()), SLOT(onCursorPositionChanged()));

    if (AEnableFormatAutoReset)
        connect(AEditWidget->instance(), SIGNAL(messageSent()), SLOT(onMessageSent()));

    onCurrentCharFormatChanged(FTextEdit->currentCharFormat());    
    updateCurrentBlock(FTextEdit->textCursor());
}

void EditHtml::setupFontActions(bool AEnableReset)
{
//-----
    if (AEnableReset)
    {
        FActionResetFormat = new Action(this);
		FActionResetFormat->setIcon(QIcon::fromTheme("message format", FIconStorage->getIcon(XHI_FORMAT_PLAIN)));
        FActionResetFormat->setText(tr("Reset formatting on message send"));
        //removeFormat->setShortcut(Qt::CTRL + Qt::Key_B);
        FActionResetFormat->setPriority(QAction::LowPriority);
        connect(FActionResetFormat, SIGNAL(toggled(bool)), SLOT(onResetFormat(bool)));
        FActionResetFormat->setCheckable(true);
        addAction(FActionResetFormat);
    }
//-----

    FCmbFont = new QFontComboBox(this);

    connect(FCmbFont, SIGNAL(currentFontChanged(QFont)), SLOT(onCurrentFontFamilyChanged(QFont)));
//    connect(FCmbFont, SIGNAL(currentIndexChanged(int)), SLOT(onCurrentFontIndexChanged(int)));
    addWidget(FCmbFont);

    FCmbSize = new QComboBox(this);
    addWidget(FCmbSize);
    FCmbSize->setEditable(true);

    QList<int> standardSizes=QFontDatabase::standardSizes();
    for(QList<int>::const_iterator it=standardSizes.begin(); it!=standardSizes.end(); it++ )
        FCmbSize->addItem(QString::number(*it));
    FCmbSize->setCurrentIndex(FCmbSize->findText(QString::number(QApplication::font().pointSize())));
    connect(FCmbSize, SIGNAL(currentIndexChanged(QString)), SLOT(onCurrentFontSizeChanged(QString)));

//  *** Special options ***
//  Insert link
    FActionInsertLink=new Action(this);
    FActionInsertLink->setIcon(QIcon::fromTheme("insert-link",IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK)));
    FActionInsertLink->setText(tr("Insert link"));
    FActionInsertLink->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTLINK);
    FActionInsertLink->setPriority(QAction::LowPriority);
    FActionInsertLink->setCheckable(false);
    connect(FActionInsertLink, SIGNAL(triggered()), SLOT(onInsertLink()));
    addAction(FActionInsertLink);

//  Insert image
    FActionInsertImage=new Action(this);
	FActionInsertImage->setIcon(QIcon::fromTheme("insert-image",FIconStorage->getIcon(XHI_INSERT_IMAGE)));
    FActionInsertImage->setText(tr("Insert image"));
    FActionInsertImage->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGE);
    FActionInsertImage->setPriority(QAction::LowPriority);
    connect(FActionInsertImage, SIGNAL(triggered()), SLOT(onInsertImage()));
    FActionInsertImage->setCheckable(false);
    addAction(FActionInsertImage);

//  Set tool tip
    FActionSetTitle=new Action(this);
	FActionSetTitle->setIcon(QIcon::fromTheme("set-tooltip", FIconStorage->getIcon(XHI_SET_TOOLTIP)));
    FActionSetTitle->setText(tr("Set tool tip"));
    FActionSetTitle->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_SETTOOLTIP);
    FActionSetTitle->setPriority(QAction::LowPriority);
    connect(FActionSetTitle, SIGNAL(triggered()), SLOT(onSetToolTip()));
    FActionSetTitle->setCheckable(true);
    addAction(FActionSetTitle);

    // Special formatting
    FMenuSpecial = new Menu(this);
    FMenuSpecial->setTitle(tr("Insert special symbol"));
	FMenuSpecial->menuAction()->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NBSP);
    FMenuSpecial->menuAction()->setData(ADR_SPECIAL_SYMBOL, QChar::Nbsp);
    connect(FMenuSpecial->menuAction(), SIGNAL(triggered()), SLOT(onInsertSpecial()));
    QActionGroup *group=new QActionGroup(FMenuSpecial);

    Action *action = new Action(group);
    action->setText(tr("Non-breaking space"));
	action->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NBSP);
    action->setData(ADR_SPECIAL_SYMBOL, QChar::Nbsp);
    action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNBSP);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertSpecial()));
    FMenuSpecial->addAction(action, AG_DEFAULT);

    action = new Action(group);
    action->setText(tr("New line"));
	action->setIcon(RSR_STORAGE_HTML, XHI_INSERT_NEWLINE);
    action->setData(ADR_SPECIAL_SYMBOL, QChar::LineSeparator);
    action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTNEWLINE);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertSpecial()));
    FMenuSpecial->addAction(action, AG_DEFAULT);
    addAction(FMenuSpecial->menuAction());

//-----
    addSeparator();
//-----
//  *** Font options ***
//  Bold
    FActionTextBold=new Action(this);
	FActionTextBold->setIcon(QIcon::fromTheme("format-text-bold",FIconStorage->getIcon(XHI_TEXT_BOLD)));
    FActionTextBold->setText(tr("Bold"));
    FActionTextBold->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_BOLD);
    FActionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    FActionTextBold->setFont(bold);
    FActionTextBold->setCheckable(true);
    connect(FActionTextBold, SIGNAL(triggered()), SLOT(onTextBold()));
    addAction(FActionTextBold);

//  Italic
    FActionTextItalic=new Action(this);
	FActionTextItalic->setIcon(QIcon::fromTheme("format-text-italic",FIconStorage->getIcon(XHI_TEXT_ITALIC)));
    FActionTextItalic->setText(tr("Italic"));
    FActionTextItalic->setPriority(QAction::LowPriority);
    FActionTextItalic->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ITALIC);
    QFont italic;
    italic.setItalic(true);
    FActionTextItalic->setFont(italic);
    connect(FActionTextItalic, SIGNAL(triggered()), SLOT(onTextItalic()));
    FActionTextItalic->setCheckable(true);
    addAction(FActionTextItalic);

//  Underline
    FActionTextUnderline=new Action(this);
	FActionTextUnderline->setIcon(QIcon::fromTheme("format-text-underline",FIconStorage->getIcon(XHI_TEXT_UNDERLINE)));
    FActionTextUnderline->setText(tr("Underline"));
    FActionTextUnderline->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_UNDERLINE);
    FActionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    FActionTextUnderline->setFont(underline);
    connect(FActionTextUnderline, SIGNAL(triggered()), SLOT(onTextUnderline()));
    FActionTextUnderline->setCheckable(true);
    addAction(FActionTextUnderline);

    //  Overline
    FActionTextOverline=new Action(this);
	FActionTextOverline->setIcon(QIcon::fromTheme("format-text-overline", FIconStorage->getIcon(XHI_TEXT_OVERLINE)));
    FActionTextOverline->setText(tr("Overline"));
    FActionTextOverline->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_OVERLINE);
    FActionTextOverline->setPriority(QAction::LowPriority);
    QFont overline;
    overline.setOverline(true);
    FActionTextOverline->setFont(overline);
    connect(FActionTextOverline, SIGNAL(triggered()), SLOT(onTextOverline()));
    FActionTextOverline->setCheckable(true);
    addAction(FActionTextOverline);

//  Striketrough
    FActionTextStrikeout=new Action(this);
	FActionTextStrikeout->setIcon(QIcon::fromTheme("format-text-strikethrough",FIconStorage->getIcon(XHI_TEXT_STRIKEOUT)));
    FActionTextStrikeout->setText(tr("Strikethrough"));
    FActionTextStrikeout->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_STRIKEOUT);
    FActionTextStrikeout->setPriority(QAction::LowPriority);
    connect(FActionTextStrikeout, SIGNAL(triggered()), SLOT(onTextStrikeout()));
    FActionTextStrikeout->setCheckable(true);
    addAction(FActionTextStrikeout);

//  Code
    FActionTextCode=new Action(this);
	FActionTextCode->setIcon(QIcon::fromTheme("format-text-code", FIconStorage->getIcon(XHI_CODE)));
    FActionTextCode->setText(tr("Code"));
    FActionTextCode->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_CODE);
    FActionTextCode->setPriority(QAction::LowPriority);
    connect(FActionTextCode, SIGNAL(triggered()), SLOT(onTextCode()));
    FActionTextCode->setCheckable(true);
    addAction(FActionTextCode);
}

void EditHtml::setupTextActions()
{
    FColorToolButton = new ColorToolButton(this);
    FColorToolButton->setToolTip(tr("Color"));
    connect(FColorToolButton, SIGNAL(click(bool)), SLOT(onColorClicked(bool)));
    addWidget(FColorToolButton);

    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FOREGROUNDCOLOR, FEditWidget->instance()->parentWidget());
    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_BACKGROUNDCOLOR, FEditWidget->instance()->parentWidget());
    Shortcuts::insertWidgetShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_FONT, FEditWidget->instance()->parentWidget());
    connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));

//-----
    FActionRemoveFormat=new Action(this);
	FActionRemoveFormat->setIcon(QIcon::fromTheme("format-text-clear", FIconStorage->getIcon(XHI_FORMAT_CLEAR)));
    FActionRemoveFormat->setText(tr("Remove format"));
    FActionRemoveFormat->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_FORMATREMOVE);
    FActionRemoveFormat->setPriority(QAction::LowPriority);
    connect(FActionRemoveFormat, SIGNAL(triggered()), this, SLOT(onRemoveFormat()));
    FActionRemoveFormat->setCheckable(false);
    addAction(FActionRemoveFormat);
    addSeparator();
//------------------------
    FActionIndentLess= new Action(this);
	FActionIndentLess->setIcon(QIcon::fromTheme("format-indent-less", FIconStorage->getIcon(XHI_OUTDENT)));
    FActionIndentLess->setText(tr("Decrease indent"));
    FActionIndentLess->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INDENTDECREASE);
    FActionIndentLess->setPriority(QAction::LowPriority);
    FActionIndentLess->setCheckable(false);
    connect(FActionIndentLess, SIGNAL(triggered()), this, SLOT(onIndentChange()));
    addAction(FActionIndentLess);

    FActionIndentMore=new Action(this);
	FActionIndentMore->setIcon(QIcon::fromTheme("format-indent-more",FIconStorage->getIcon(XHI_INDENT)));
    FActionIndentMore->setText(tr("Increase indent"));
    FActionIndentMore->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_INDENTINCREASE);
    FActionIndentMore->setPriority(QAction::LowPriority);
    FActionIndentMore->setCheckable(false);
    connect(FActionIndentMore, SIGNAL(triggered()), this, SLOT(onIndentChange()));
    addAction(FActionIndentMore);

    addSeparator();

//  Alignment    
    FMenuAlign = new Menu(this);
    FMenuAlign->setTitle(tr("Text align"));
    FMenuAlign->menuAction()->setCheckable(true);
    connect(FMenuAlign->menuAction(), SIGNAL(triggered()), SLOT(onTextAlign()));

    QActionGroup *group=new QActionGroup(FMenuAlign);
    Action *action = new Action(group);
    action->setText(tr("Left"));
	action->setIcon(RSR_STORAGE_HTML, XHI_ALIGN_LEFT);
    action->setData(ADR_ALIGN_TYPE, int(Qt::AlignLeft|Qt::AlignAbsolute));
    action->setCheckable(true);
    action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNLEFT);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
    FMenuAlign->addAction(action, AG_XHTMLIM_ALIGN);
    FActionLastAlign = action;

    action = new Action(group);
    action->setText(tr("Center"));
	action->setIcon(RSR_STORAGE_HTML,XHI_ALIGN_CENTER);
    action->setData(ADR_ALIGN_TYPE, Qt::AlignHCenter);
    action->setCheckable(true);
    action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNCENTER);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
    FMenuAlign->addAction(action, AG_XHTMLIM_ALIGN);

    action = new Action(group);
    action->setText(tr("Right"));
	action->setIcon(RSR_STORAGE_HTML,XHI_ALIGN_RIGHT);
    action->setData(ADR_ALIGN_TYPE, int(Qt::AlignRight|Qt::AlignAbsolute));
    action->setCheckable(true);
    action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNRIGHT);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
    FMenuAlign->addAction(action, AG_XHTMLIM_ALIGN);

    action = new Action(group);
    action->setText(tr("Justify"));
	action->setIcon(RSR_STORAGE_HTML,XHI_ALIGN_JUSTIFY);
    action->setData(ADR_ALIGN_TYPE, Qt::AlignJustify);
    action->setCheckable(true);
    action->setShortcutId(SCT_MESSAGEWINDOWS_XHTMLIM_ALIGNJUSTIFY);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onTextAlign()));
    FMenuAlign->addAction(action, AG_XHTMLIM_ALIGN);

    addAction(FMenuAlign->menuAction());

    // Text list
    FMenuList = new Menu(this);
    FMenuList->setTitle(tr("List"));
	FMenuList->setIcon(RSR_STORAGE_HTML, XHI_LIST_BULLET_DISC);
    FMenuList->menuAction()->setEnabled(true);
    FMenuList->menuAction()->setData(ADR_LIST_TYPE, QTextListFormat::ListDisc);
    connect(FMenuList->menuAction(), SIGNAL(triggered()), SLOT(onInsertList()));

    group=new QActionGroup(FMenuList);
    action = new Action(group);
    action->setText(tr("Disc"));
	action->setIcon(RSR_STORAGE_HTML, XHI_LIST_BULLET_DISC);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListDisc);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);
    FActionLastList = action;

    action = new Action(group);
    action->setText(tr("Circle"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_BULLET_CIRCLE);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListCircle);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Square"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_BULLET_SQUARE);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListSquare);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Decimal"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_DECIMAL);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListDecimal);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Alpha lower"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ALPHA_LOW);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListLowerAlpha);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Alpha upper"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ALPHA_UP);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListUpperAlpha);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Roman lower"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ROMAN_LOW);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListLowerRoman);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Roman upper"));
	action->setIcon(RSR_STORAGE_HTML,XHI_LIST_ORDER_ROMAN_UP);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListUpperRoman);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_LIST);

    action = new Action(group);
    action->setText(tr("Definition list"));
	action->setIcon(RSR_STORAGE_HTML, XHI_LIST_DEFINITION);
    action->setData(ADR_LIST_TYPE, QTextListFormat::ListStyleUndefined);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onInsertList()));
    FMenuList->addAction(action, AG_XHTMLIM_DEFLIST);
    addAction(FMenuList->menuAction());

    // Special formatting
    FMenuFormat = new Menu(this);
    FMenuFormat->setTitle(tr("Formatting type"));
    FMenuFormat->menuAction()->setCheckable(true);
    connect(FMenuFormat->menuAction(), SIGNAL(triggered()), SLOT(onSetFormat()));

    group=new QActionGroup(FMenuFormat);
    action = new Action(group);
    action->setText(tr("Preformatted text"));
	action->setIcon(RSR_STORAGE_HTML, XHI_PREFORMAT);
    action->setData(ADR_FORMATTING_TYPE, FMT_PREFORMAT);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMAT);
    FActionLastFormat = action;

    action = new Action(group);
    action->setText(tr("Heading 1"));
	action->setIcon(RSR_STORAGE_HTML, XHI_HEADING1);
    action->setData(ADR_FORMATTING_TYPE, FMT_HEADING1);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMATHEADING);

    action = new Action(group);
    action->setText(tr("Heading 2"));
	action->setIcon(RSR_STORAGE_HTML, XHI_HEADING2);
    action->setData(ADR_FORMATTING_TYPE, FMT_HEADING2);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMATHEADING);

    action = new Action(group);
    action->setText(tr("Heading 3"));
	action->setIcon(RSR_STORAGE_HTML, XHI_HEADING3);
    action->setData(ADR_FORMATTING_TYPE, FMT_HEADING3);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMATHEADING);

    action = new Action(group);
    action->setText(tr("Heading 4"));
	action->setIcon(RSR_STORAGE_HTML, XHI_HEADING4);
    action->setData(ADR_FORMATTING_TYPE, FMT_HEADING4);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMATHEADING);

    action = new Action(group);
    action->setText(tr("Heading 5"));
	action->setIcon(RSR_STORAGE_HTML, XHI_HEADING5);
    action->setData(ADR_FORMATTING_TYPE, FMT_HEADING5);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMATHEADING);

    action = new Action(group);
    action->setText(tr("Heading 6"));
	action->setIcon(RSR_STORAGE_HTML, XHI_HEADING6);
    action->setData(ADR_FORMATTING_TYPE, FMT_HEADING6);
    action->setCheckable(true);
    action->setPriority(QAction::LowPriority);
    action->setActionGroup(group);
    connect(action, SIGNAL(triggered()), SLOT(onSetFormat()));
    FMenuFormat->addAction(action, AG_XHTMLIM_FORMATHEADING);    
    addAction(FMenuFormat->menuAction());
}

//----slots-----
void EditHtml::onMessageSent()
{
    if (!FActionResetFormat->isChecked())
        clearFormatOnWordOrSelection(true);
}

void EditHtml::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AId==SCT_MESSAGEWINDOWS_XHTMLIM_FOREGROUNDCOLOR)
        selectForegroundColor();
    else if (AId==SCT_MESSAGEWINDOWS_XHTMLIM_BACKGROUNDCOLOR)
        selectBackgroundColor();
    else if (AId==SCT_MESSAGEWINDOWS_XHTMLIM_FONT)
        selectFont();
/*
    else if (AId==SCT_MESSAGEWINDOWS_XHTMLIM_INDENTDECREASE)
    {
        QTextCursor cursor = FEditWidget->textEdit()->textCursor();
        if (cursor.atBlockStart())
            FActionIndentMore->trigger();
        else
            cursor.insertText("\t");
    }
    else if (AId==SCT_MESSAGEWINDOWS_XHTMLIM_INDENTDECREASE)
        FActionIndentLess->trigger();
*/
}

void EditHtml::onColorClicked(bool ABackground)
{
    if(ABackground)
        selectBackgroundColor();
    else
        selectForegroundColor();
}

void EditHtml::onResetFormat(bool AStatus)
{
    if(!AStatus)
    {
		FActionResetFormat->setIcon(FIconStorage->getIcon(XHI_FORMAT_PLAIN));
        FActionResetFormat->setToolTip(tr("Reset formatting on message send"));
    }
    else
    {
		FActionResetFormat->setIcon(FIconStorage->getIcon(XHI_FORMAT_RICH));
        FActionResetFormat->setToolTip(tr("Keep formatting within chat session"));
    }
}

void EditHtml::onInsertImage()
{
    QUrl        imageUrl;
    QByteArray  imageData;
    QTextCursor cursor = FTextEdit->textCursor();
    QTextCharFormat charFmtCurrent=cursor.charFormat();
    QSize       size;
    QString     alt;

    bool supportBoB=FBitsOfBinary && FBitsOfBinary->isSupported(FEditWidget->messageWindow()->streamJid(),FEditWidget->messageWindow()->contactJid());

    if (!cursor.hasSelection())
        if (charFmtCurrent.isImageFormat())
        {
            QTextImageFormat imageFormat=charFmtCurrent.toImageFormat();
            cursor.select(QTextCursor::WordUnderCursor);
			imageUrl = QUrl::fromEncoded(imageFormat.name().toLatin1());
            imageData=FTextEdit->document()->resource(QTextDocument::ImageResource, imageUrl).toByteArray();
            size.setWidth(imageFormat.width());
            size.setHeight(imageFormat.height());
            alt=imageFormat.property(XmlTextDocumentParser::ImageAlternativeText).toString();
        }

	InsertImage *inserImage = new InsertImage(FXhtmlIm, FNetworkAccessManager, imageData, imageUrl, size, alt);

	inserImage->setWindowIcon(FIconStorage->getIcon(XHI_INSERT_IMAGE));
    if(!supportBoB)
        inserImage->ui->pbBrowse->hide();
    if(inserImage->exec() == QDialog::Accepted)
    {
        if(!inserImage->getUrl().isEmpty())
        {
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
                    QByteArray selectedFormat=inserImage->getSelectedFormat();
                    if (inserImage->physResize() ||
                       (!selectedFormat.isNull() &&
                        inserImage->getOriginalFormat()!=selectedFormat))
                    {
                        QBuffer buffer(&imageData);
                        buffer.open(QIODevice::ReadOnly);
                        QImageReader reader(&buffer, inserImage->getOriginalFormat());
                        QImage image=reader.read();
                        buffer.close();

                        imageData.clear();
                        buffer.open(QIODevice::WriteOnly);
                        if (selectedFormat.isEmpty())
                            selectedFormat=inserImage->getOriginalFormat();
                        QImageWriter writer(&buffer, selectedFormat);
                        writer.write(inserImage->physResize()?image.scaled(inserImage->newWidth(), inserImage->newHeight(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
                                                             :image);
                        buffer.close();
                    }
                    QString contentId=FBitsOfBinary->contentIdentifier(imageData);
                    QString uri=QString("cid:").append(contentId);
                    imageFormat.setName(uri);
                    imageFormat.setProperty(XhtmlIm::PMaxAge, inserImage->getMaxAge());
                    imageFormat.setProperty(XhtmlIm::PMimeType, inserImage->getFileType());
                    imageFormat.setProperty(XhtmlIm::PEmbed, inserImage->embed());
                    cursor.document()->addResource(QTextDocument::ImageResource, QUrl(uri), imageData);
                    cursor.insertImage(imageFormat);
                }
        }
    }
    inserImage->deleteLater();
}

void EditHtml::onSetToolTip()
{
    QTextCursor cursor = FTextEdit->textCursor();
    QTextCharFormat charFormat=cursor.charFormat();
    if (!charFormat.hasProperty(QTextFormat::TextToolTip) &&
        !cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    Action *action=qobject_cast<Action *>(sender());
    int toolTipType = charFormat.intProperty(XmlTextDocumentParser::ToolTipType);

    SetToolTip *setToolTip = new SetToolTip(toolTipType, charFormat.toolTip(), action->parentWidget()->parentWidget());
    if(setToolTip->exec() == QDialog::Accepted)
    {
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
    }
    setToolTip->deleteLater();
}

void EditHtml::onInsertLink()
{
    QTextCursor cursor = FTextEdit->textCursor();

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

    AddLink *addLink = new AddLink(tr("Add link"),
                                   IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK),
                                   QUrl::fromEncoded(charFmtCurrent.anchorHref().toLatin1()), cursor.selectedText(), action->parentWidget()->parentWidget());
    switch (addLink->exec())
    {
        case AddLink::Add:
        {
            QTextCharFormat charFmt=charFmtCurrent;
            charFmt.setAnchor(true);
            charFmt.setAnchorHref(addLink->url().toEncoded());
            charFmt.setFontUnderline(true);
            charFmt.setForeground(QBrush(Qt::blue));
            if (needsToBeInserted)
            {
                cursor.insertText(addLink->description(), charFmt);
                cursor.insertText(" ", charFmtCurrent);
            }
            else
				cursor.mergeCharFormat(charFmt);
            break;
        }

        case AddLink::Remove:
        {
            QTextCharFormat charFmt;
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
//            charFmt.setFontFamily(charFmtCurrent.fontFamily());
//            charFmt.setFontItalic(charFmtCurrent.fontItalic());
//            charFmt.setFontStrikeOut(charFmtCurrent.fontStrikeOut());
//            charFmt.setFontWeight(charFmtCurrent.fontWeight());
//            charFmt.setFontPointSize(charFmtCurrent.fontPointSize());
//            charFmt.setBackground(charFmt.background());
//            cursor.setCharFormat(charFmt);

            break;
        }
    }
    addLink->deleteLater();
}

void EditHtml::onTextBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(FActionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void EditHtml::onTextItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(FActionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void EditHtml::onTextUnderline()
{
    QTextCharFormat fmt;
    fmt.setProperty(QTextFormat::TextUnderlineStyle, FActionTextUnderline->isChecked()?QTextCharFormat::SingleUnderline
                                                                                      :QTextCharFormat::NoUnderline);
    mergeFormatOnWordOrSelection(fmt);
}

void EditHtml::onTextOverline()
{
    QTextCharFormat fmt;
    fmt.setFontOverline(FActionTextOverline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void EditHtml::onTextStrikeout()
{
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(FActionTextStrikeout->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void EditHtml::onTextCode()
{
    QTextCharFormat fmt;
    fmt.setFontFixedPitch(FActionTextCode->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void EditHtml::onCurrentFontFamilyChanged(const QFont &AFont)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(AFont.family());
    mergeFormatOnWordOrSelection(fmt);
}
/*
void EditHtml::onCurrentFontIndexChanged(int AIndex)
{
    qDebug() << "EditHtml::onCurrentFontIndexChanged(" << AIndex << ")";
    QTextCharFormat fmt;
    switch (AIndex)
    {
        case FNT_SERIF:
//            fmt.setFontFamily("serif");
            fmt.setFontStyleHint(QFont::Serif);
            break;
        case FNT_SANSSERIF:
//            fmt.setFontFamily("sans-serif");
            fmt.setFontStyleHint(QFont::SansSerif);
            break;
// #if QT_VERSION >= 0x040700
        case FNT_CURSIVE:
//            fmt.setFontFamily("cursive");
            fmt.setFontStyleHint(QFont::Cursive);
            break;
        case FNT_FANTASY:
//            fmt.setFontFamily("fantasy");
            fmt.setFontStyleHint(QFont::Fantasy);
            break;
        case FNT_MONOSPACED:
//            fmt.setFontFamily("monospace");
            fmt.setFontStyleHint(QFont::Monospace);
            break;
        case FNT_SEPARATOR:
//            fmt.setFontFamily("monospace");
            fmt.clearProperty(QTextFormat::FontFamily);
            fmt.clearProperty(QTextFormat::FontStyleHint);
            break;

// #else
//        case FNT_MONOSPACED:
//            fmt.setFontFixedPitch(true);
//            break;
//#endif
        default:
        {
            fmt.setFontFamily(FCmbFont->currentFont().family());
            qDebug() << "fmt.fontFamily()=" << fmt.fontFamily();
//            return;
        }
    }    
    qDebug() << "fmt.fontStyleHint()=" << fmt.fontStyleHint();
    mergeFormatOnWordOrSelection(fmt);
}
*/

void EditHtml::onCurrentFontSizeChanged(const QString &ASize)
{
    qreal pointSize = ASize.toFloat();
    if (ASize.toFloat() > 0)
    {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void EditHtml::onInsertList()
{
    Action *ac = qobject_cast<Action *>(sender());
    if (ac)
    {
//        int styleIndex = ac->data(ADR_ORDER_TYPE).toInt();
        QTextListFormat::Style style = (QTextListFormat::Style)ac->data(ADR_LIST_TYPE).toInt();// = QTextListFormat::ListStyleUndefined;
        FMenuList->setIcon(ac->icon());
        FMenuList->menuAction()->setData(ADR_LIST_TYPE, style);
        QTextCursor cursor = FTextEdit->textCursor();        
        if (style >=QTextListFormat::ListUpperRoman && style <=QTextListFormat::ListStyleUndefined)
        {
//            style=(QTextListFormat::Style)styleIndex;
            cursor.beginEditBlock();            
            QTextListFormat listFmt;
            QTextList       *list=cursor.currentList();
            if (list)   //  Have list at current position
            {
                listFmt = list->format();
                int indent=listFmt.indent();
                if (listFmt.style()==QTextListFormat::ListStyleUndefined)
                    indent++;
                listFmt.setStyle(style);
                if (style==QTextListFormat::ListStyleUndefined)
                    indent--;
                listFmt.setIndent(indent);
                if (list->itemNumber(cursor.block())==0 && cursor.atBlockStart())    // First block in the list
                    list->setFormat(listFmt);               //  Just update list format
                else                                        // Not first block
                {
                    indent++;
                    listFmt.setIndent(indent);  //  Create a sublist
                    if (!cursor.atBlockStart() || cursor.block().previous().textList()!=list)
                        cursor.insertBlock();
                    cursor.createList(listFmt);
                }
            }
            else        // No list at current position
            {
                int indent=style==QTextListFormat::ListStyleUndefined?0:1;
                listFmt.setIndent(indent);
                listFmt.setStyle(style);    // Just set its style
                cursor.createList(listFmt); // and create a root list
            }
            cursor.endEditBlock();
        }
        else
        {
            // ####
            QTextBlockFormat bfmt;
            bfmt.setObjectIndex(-1);
            cursor.mergeBlockFormat(bfmt);
        }
    }
}

int EditHtml::checkBlockFormat(const QTextCursor &ACursor)
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

void EditHtml::clearBlockProperties(const QTextBlock &ATextBlock, const QSet<QTextFormat::Property> &AProperties)
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

QTextFragment EditHtml::getTextFragment(const QTextCursor &ACursor)
{
    QTextBlock block=ACursor.block();
    for (QTextBlock::iterator it=block.begin(); it!=block.end(); it++)
        if (it.fragment().contains(ACursor.position()))
            return it.fragment();
    return QTextFragment();
}

void EditHtml::selectTextFragment(QTextCursor &ACursor)
{
    QTextFragment fragment=getTextFragment(ACursor);
    if (fragment.isValid())
    {
        ACursor.setPosition(fragment.position());
        ACursor.setPosition(fragment.position()+fragment.length(), QTextCursor::KeepAnchor);
    }
}

void EditHtml::onSetFormat()
{
    Action *ac = qobject_cast<Action *>(sender());
    if (ac!=FMenuFormat->menuAction())
        FActionLastFormat=ac;
    int formatType = ac->data(ADR_FORMATTING_TYPE).toInt();
    QTextCursor cursor = FTextEdit->textCursor();
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
    updateCurrentBlock(cursor);
}

void EditHtml::onInsertSpecial()
{
    Action *ac = qobject_cast<Action *>(sender());
    QChar specialSybmol = (QChar)(ac->data(ADR_SPECIAL_SYMBOL).toInt());
    FMenuSpecial->menuAction()->setData(ADR_SPECIAL_SYMBOL, specialSybmol);
    FMenuSpecial->menuAction()->setIcon(ac->icon());
    QTextCursor cursor = FTextEdit->textCursor();
    cursor.beginEditBlock();
    cursor.insertText(specialSybmol);
    cursor.endEditBlock();
}

void EditHtml::onIndentChange()
{
    QAction *action=qobject_cast<QAction *>(sender());
    QTextCursor cursor = FTextEdit->textCursor();
    bool increase=action==FActionIndentMore;
    QTextBlockFormat blockFmt = cursor.blockFormat();
    if (cursor.currentList())
    {
        if (cursor.currentList()->format().style()==QTextListFormat::ListStyleUndefined)
            blockFmt.setIndent(increase);
    }
    else
    {
        qreal indentWidth=FTextEdit->document()->indentWidth();
        qreal indent=blockFmt.textIndent();
        if (increase)
            blockFmt.setTextIndent(indent+indentWidth);
        else
            if (indent>0)
                blockFmt.setTextIndent(indent-indentWidth);
    }
    cursor.setBlockFormat(blockFmt);
}

void EditHtml::onCursorPositionChanged()
{
    QTextCursor cursor=qobject_cast<QTextEdit *>(sender())->textCursor();
    FCurrentPosition = cursor.position();
    updateCurrentList(cursor);
    updateCurrentBlock(cursor);
}

void EditHtml::onTextChanged()
{
    if (!FTextChangedProcessing)
    {
        FTextChangedProcessing=true;
        QTextEdit *editor=qobject_cast<QTextEdit *>(sender());
        QTextCursor cursor=editor->textCursor();
        QTextBlock block=cursor.block();
        const QTextDocument *doc=block.document();

        if (cursor.position()==FCurrentPosition &&      // Position unchanged
            doc->characterCount()==FCurrentCharacterCount &&    // Text length unchanged
            FCurrentList && !block.textList())          // List changed
        {
            QTextBlockFormat format=cursor.blockFormat();
            format.setIndent(0);
            cursor.setBlockFormat(format);
            QTextList     *parentList=NULL;
            if (FCurrentIndent>1)   // A sublist
            {                       // should be moved into parent list
                for (int i=block.blockNumber()-1; i>=0; i--)
                {
                    QTextList  *list=doc->findBlockByNumber(i).textList();
                    if (!list)
                        break;
                    int indent=list->format().indent();
                    if (list->format().style()==QTextListFormat::ListStyleUndefined)
                        indent++;
                    if (indent==FCurrentIndent-1)
                    {
                        parentList=list;
                        break;
                    }
                }

                if (parentList)
                    parentList->add(block);
            }

            QSet<QTextList *>lists;
            for (QTextBlock b=block.next(); b!=doc->end(); b=b.next())
            {
                QTextList *list=b.textList();
                if (list==FCurrentList)
                    if (parentList)
                        parentList->add(b);
                    else
                    {
                        FCurrentList->remove(b);
                        QTextCursor cursor(b);
                        QTextBlockFormat format=cursor.blockFormat();
                        format.setIndent(0);
                        cursor.setBlockFormat(format);
                    }
                else if (list)
                    if (list->format().indent()>FCurrentIndent)
                        lists.insert(list);
                    else
                        break;
                else
                    break;
            }
            for (QSet<QTextList *>::iterator it=lists.begin(); it!=lists.end(); it++)
            {
                QTextListFormat format=(*it)->format();
                format.setIndent(format.indent()-1);
                (*it)->setFormat(format);
            }
        }
        else if (FNewListItemCreated &&
                FCurrentList->format().style() == QTextListFormat::ListStyleUndefined)
        {
                QTextBlockFormat format=FCurrentList->item(FCurrentListItemNumber).blockFormat();
                if (format.indent())
                    format.setIndent(0);
                else
                    format.setIndent(1);
                cursor.setBlockFormat(format);
        } else
        {
            if (Options::node(OPV_XHTML_TABINDENT).value().toBool() &&
                doc->characterCount() == FCurrentCharacterCount+1 &&
#if QT_VERSION >= 0x040700
                cursor.positionInBlock()
#else
                (cursor.position() - block.position())
#endif
                    == 1 &&
                block.text()[0]=='\t')
                {
                    cursor.beginEditBlock();
                    cursor.deletePreviousChar();
                    cursor.endEditBlock();
                    FActionIndentMore->trigger();
                }
        }
        updateCurrentList(cursor);
        updateCurrentBlock(cursor);
        FCurrentCharacterCount=doc->characterCount();
        FTextChangedProcessing=false;
    }
}

void EditHtml::onTextAlign()
{
    Action *action = qobject_cast<Action *>(sender());
    if (action)
    {
        Qt::AlignmentFlag align = Qt::AlignmentFlag(action->data(ADR_ALIGN_TYPE).toInt());
        QTextBlockFormat format=FTextEdit->textCursor().blockFormat();
        if (format.hasProperty(QTextFormat::BlockAlignment) &&
            format.alignment()==align)
        {
            format.clearProperty(QTextFormat::BlockAlignment);
            FTextEdit->textCursor().setBlockFormat(format);
        }
        else
        {
            FMenuAlign->setIcon(action->icon());
            FMenuAlign->menuAction()->setData(ADR_ALIGN_TYPE, align);
            FTextEdit->setAlignment(align);
            FActionLastAlign = action;
        }
        updateCurrentBlock(FTextEdit->textCursor());
    }
}

void EditHtml::onRemoveFormat()
{
    clearFormatOnWordOrSelection();
    updateCurrentBlock(FTextEdit->textCursor());
    QTextCharFormat currFmt=FTextEdit->currentCharFormat();
    onCurrentCharFormatChanged(currFmt);
}

void EditHtml::onCurrentCharFormatChanged(const QTextCharFormat &ACharFormat)
{
    fontChanged(ACharFormat);
    QBrush brush=ACharFormat.foreground();
    if (brush.style())
        FColorToolButton->setForegroundColor(brush.color());
    else
        FColorToolButton->setForegroundColor(FTextEdit->palette().foreground().color());
    brush=ACharFormat.background();
    if (brush.style())
        FColorToolButton->setBackgroundColor(brush.color());
    else
        FColorToolButton->setBackgroundColor(FTextEdit->palette().background().color());
    FActionSetTitle->setChecked(ACharFormat.hasProperty(QTextFormat::TextToolTip));
}

void EditHtml::fontChanged(const QTextCharFormat &ACharFormat)
{
    if (ACharFormat.hasProperty(QTextFormat::FontFamily))
        FCmbFont->setCurrentFont(ACharFormat.font());

/*
    else if (ACharFormat.hasProperty(QTextFormat::FontStyleHint))
    {
        int index;
        switch (ACharFormat.intProperty(QTextFormat::FontStyleHint))
        {
            case QFont::Serif: index=FNT_SERIF; break;
            case QFont::SansSerif: index=FNT_SANSSERIF; break;
            case QFont::Cursive: index=FNT_CURSIVE; break;
            case QFont::Monospace: index=FNT_MONOSPACED; break;
            case QFont::Fantasy: index=FNT_FANTASY; break;
        }
        FCmbFont->setCurrentIndex(index);
    }
    else
        FCmbFont->setCurrentIndex(FNT_SEPARATOR);   // No font
*/
    if (ACharFormat.hasProperty(QTextFormat::FontPointSize))
        FCmbSize->setEditText(QString::number(ACharFormat.doubleProperty(QTextFormat::FontPointSize)));
    if (ACharFormat.hasProperty(QTextFormat::FontWeight))
        FActionTextBold->setChecked(ACharFormat.fontWeight()>=QFont::Bold);
    else
        FActionTextBold->setChecked(false);
    if (ACharFormat.hasProperty(QTextFormat::FontItalic))
        FActionTextItalic->setChecked(ACharFormat.boolProperty(QTextFormat::FontItalic));
    else
        FActionTextItalic->setChecked(false);
    if (ACharFormat.hasProperty(QTextFormat::FontUnderline))
        FActionTextUnderline->setChecked(ACharFormat.boolProperty(QTextFormat::FontUnderline));
    else if (ACharFormat.hasProperty(QTextFormat::TextUnderlineStyle))
        FActionTextUnderline->setChecked(ACharFormat.intProperty(QTextFormat::TextUnderlineStyle)==QTextCharFormat::SingleUnderline);
    else
        FActionTextUnderline->setChecked(false);
    if (ACharFormat.hasProperty(QTextFormat::FontOverline))
        FActionTextOverline->setChecked(ACharFormat.boolProperty(QTextFormat::FontOverline));
    else
        FActionTextOverline->setChecked(false);
    if (ACharFormat.hasProperty(QTextFormat::FontStrikeOut))
        FActionTextStrikeout->setChecked(ACharFormat.boolProperty(QTextFormat::FontStrikeOut));
    else
        FActionTextStrikeout->setChecked(false);
    if (ACharFormat.hasProperty(QTextFormat::FontFixedPitch))
        FActionTextCode->setChecked(ACharFormat.boolProperty(QTextFormat::FontFixedPitch));
    else
        FActionTextCode->setChecked(false);
}

void EditHtml::selectForegroundColor()
{
    QColor col = QColorDialog::getColor(FTextEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    FColorToolButton->setForegroundColor(col);
}

void EditHtml::selectBackgroundColor()
{
    QColor color = QColorDialog::getColor(FTextEdit->textBackgroundColor(), this);
    if (!color.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setBackground(color);
    mergeFormatOnWordOrSelection(fmt);
    FColorToolButton->setBackgroundColor(color);
}

void EditHtml::updateCurrentList(const QTextCursor &ACursor)
{
    FNewListItemCreated = FCurrentList &&                                              // It's a list and
                       ACursor.currentList() == FCurrentList &&                      // The list is unchanged and
                       FCurrentList->count() == FCurrentListItemCount+1 &&          // Number of items in the list increased by 1 and
                       FCurrentList->itemNumber(ACursor.block()) == FCurrentListItemNumber+1; // Number of current item increased by 1

    if ((FCurrentList = ACursor.currentList()))
    {
        FCurrentIndent = FCurrentList->format().indent();
        if (ACursor.currentList()->format().style()==QTextListFormat::ListStyleUndefined)
            FCurrentIndent++;
        FCurrentListItemNumber = FCurrentList->itemNumber(ACursor.block());
        FCurrentListItemCount = FCurrentList->count();
    }
    else
    {
        FCurrentIndent = 0;
        FCurrentListItemNumber = 0;
        FCurrentListItemCount = 0;
    }
}

void EditHtml::updateCurrentBlock(const QTextCursor &ACursor)
{
    if (ACursor.currentList())
        FMenuFormat->setEnabled(false);
    else
    {
        FMenuFormat->setEnabled(true);
        QTextBlockFormat blockFormat=ACursor.blockFormat();

        int format=checkBlockFormat(ACursor);
        if (format)
        {
            FMenuList->setEnabled(false);
			QList<Action *>actions=FMenuFormat->actions();
			Action *action=NULL;
            for (QList<Action *>::iterator it=actions.begin(); it!=actions.end(); it++)
                if ((*it)->data(ADR_FORMATTING_TYPE).toInt()==format)
                {
                    action=*it;
                    break;
                }
            action->setChecked(true);
            FMenuFormat->setIcon(action->icon());
            FMenuFormat->menuAction()->setData(ADR_FORMATTING_TYPE, format);
            FMenuFormat->menuAction()->setChecked(true);
        }
        else
        {
            FMenuList->setEnabled(true);
            FMenuFormat->menuAction()->setData(ADR_FORMATTING_TYPE, FActionLastFormat->data(ADR_FORMATTING_TYPE));
            FMenuFormat->setIcon(FActionLastFormat->icon());
            FMenuFormat->menuAction()->setChecked(false);
        }

        if (blockFormat.hasProperty(QTextFormat::BlockAlignment))
        {
            Qt::Alignment align = blockFormat.alignment();
			QList<Action *>actions=FMenuAlign->actions();
			Action *action=NULL;
            for (QList<Action *>::iterator it=actions.begin(); it!=actions.end(); it++)
                if (((*it)->data(ADR_ALIGN_TYPE).toInt()&0x0F)==(align&0x0F))
                {
                    action=*it;
                    break;
                }
            action->setChecked(true);
            FMenuAlign->setIcon(action->icon());
            FMenuAlign->menuAction()->setData(ADR_ALIGN_TYPE, (int)align);
            FMenuAlign->menuAction()->setChecked(true);
        }
        else
        {
            FMenuAlign->menuAction()->setData(ADR_ALIGN_TYPE, FActionLastAlign->data(ADR_ALIGN_TYPE));
            FMenuAlign->setIcon(FActionLastAlign->icon());
            FMenuAlign->menuAction()->setChecked(false);
        }
    }
}

void EditHtml::selectFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, FEditWidget->instance()->parentWidget());
    if (ok)
    {
        QTextCharFormat fmt;
        fmt.setFont(font);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void EditHtml::mergeFormatOnWordOrSelection(const QTextCharFormat &AFormat)
{
    QTextCursor cursor = FTextEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(AFormat);
    FTextEdit->mergeCurrentCharFormat(AFormat);
}

void EditHtml::clearFormatOnWordOrSelection(bool AWholeDocument)
{
    QTextCursor cursor = FTextEdit->textCursor();
    QTextCharFormat emptyCharFormat;
    if (AWholeDocument)
    {
        cursor.select(QTextCursor::Document);
        cursor.setBlockCharFormat(emptyCharFormat);
    }
    else if (!cursor.hasSelection())
    {
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.setBlockCharFormat(emptyCharFormat);
    }
    cursor.setCharFormat(emptyCharFormat);
    FTextEdit->setCurrentCharFormat(emptyCharFormat);
}

#include "messageeditor.h"
#if QT_VERSION >= 0x050000
#include <QMimeData>
#elif QT_VERSION < 0x040800
#include <QFile>
#endif
#include <QFrame>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>

MessageEditor::MessageEditor(QWidget *AParent) : QTextEdit(AParent)
{
	FAutoResize = true;
	FMinimumLines = 1;

	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	connect(this,SIGNAL(textChanged()),SLOT(onTextChanged()));
}

MessageEditor::~MessageEditor()
{

}

bool MessageEditor::autoResize() const
{
	return FAutoResize;
}

void MessageEditor::setAutoResize(bool AResize)
{
	if (AResize != FAutoResize)
	{
		FAutoResize = AResize;
		updateGeometry();
	}
}

int MessageEditor::minimumLines() const
{
	return FMinimumLines;
}

void MessageEditor::setMinimumLines(int ALines)
{
	if (ALines != FMinimumLines)
	{
		FMinimumLines = ALines>0 ? ALines : 1;
		updateGeometry();
	}
}

QSize MessageEditor::sizeHint() const
{
	QSize sh = QTextEdit::sizeHint();
	sh.setHeight(textHeight(!FAutoResize ? FMinimumLines : 0));
	return sh;
}

QSize MessageEditor::minimumSizeHint() const
{
	QSize sh = QTextEdit::minimumSizeHint();
	sh.setHeight(textHeight(FMinimumLines));
	return sh;
}

int MessageEditor::textHeight(int ALines) const
{
	if (ALines > 0)
		return fontMetrics().height()*ALines + (frameWidth() + qRound(document()->documentMargin()))*2;
	else
		return qRound(document()->documentLayout()->documentSize().height()) + frameWidth()*2;
}

QMimeData *MessageEditor::createMimeDataFromSelection() const
{
	QMimeData *data = new QMimeData;
	emit createDataRequest(data);
	return data;
}

bool MessageEditor::canInsertFromMimeData(const QMimeData *ASource) const
{
	bool canInsert = false;
	emit canInsertDataRequest(ASource,canInsert);
	return canInsert;
}

void MessageEditor::insertFromMimeData(const QMimeData *ASource)
{
	QTextDocument doc;
	emit insertDataRequest(ASource,&doc);

	if (!doc.isEmpty())
	{
		QTextCursor cursor(&doc);
		cursor.select(QTextCursor::Document);
		if (acceptRichText())
			textCursor().insertFragment(cursor.selection());
		else
			textCursor().insertText(cursor.selection().toPlainText());
	}

	ensureCursorVisible();
    setFocus();
}
// *** <<< eyeCU <<< ***
QVariant MessageEditor::loadResource(int AType, const QUrl &AName)
{
    // Do NOT load URL with "file" scheme via proxy!
    // Load it manually instead
    if (AName.scheme()=="file")
    {
        QFile file(AName.toLocalFile());
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray imageData = file.readAll();
            file.close();
            return imageData;
        }
    }
    return QTextEdit::loadResource(AType, AName);
}
// *** >>> eyeCU >>> ***

void MessageEditor::onTextChanged()
{
	updateGeometry();
}

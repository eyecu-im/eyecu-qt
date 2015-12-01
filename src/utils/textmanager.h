#ifndef TEXTMANAGER_H
#define TEXTMANAGER_H

#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include "utilsexport.h"

class UTILS_EXPORT TextManager
{
public:
	static void    substituteHtmlText(QString &AHtml, const QString &ASourceText, const QString &ATargetTextInside, const QString &ATargetTextOutside);
	static QString getDocumentBody(const QTextDocument &ADocument);
	static QString getTextFragmentHref(const QTextDocumentFragment &AFragment);
	static void insertQuotedFragment(QTextCursor ACursor, const QTextDocumentFragment &AFragment);
	static QTextDocumentFragment getTrimmedTextFragment(const QTextDocumentFragment &AFragment, bool APlainText = false);
	static QString getElidedString(const QString &AString, Qt::TextElideMode AMode, int AMaxChars);
protected:
	static bool checkBlockStyle(const QString &AStyle);
private:
	TextManager();
};

#endif // TEXTMANAGER_H

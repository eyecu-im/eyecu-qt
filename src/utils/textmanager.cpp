#include <QTextBlock>
#include <definitions/namespaces.h>
#include <xmltextdocumentparser.h>
#include "textmanager.h"

TextManager::TextManager()
{}
// *** <<< eyeCU <<< ***
void TextManager::substituteHtmlText(QString &AHtml, const QString &ASourceText, const QString &ATargetTextInside, const QString &ATargetTextOutside)
{
	bool inside=false;
	int  j=0;
	for (int i=0; i<AHtml.length(); i=j+1)
	{
		j=AHtml.indexOf(inside?'>':'<', i);
		if (j==-1)
			j=AHtml.length();
		for (int k=AHtml.indexOf(ASourceText, i); (k<j) && (k!=-1); k=AHtml.indexOf("%sender%", i))
			if (inside)
			{
				AHtml.replace(k, ASourceText.length(), ATargetTextInside);
				i=k+ATargetTextInside.length();
			}
			else
			{
				AHtml.replace(k, ASourceText.length(), ATargetTextOutside);
				i=k+ATargetTextOutside.length();
			}
		inside=!inside;
	}

}
// *** >>> eyeCU >>> ***
QString TextManager::getDocumentBody(const QTextDocument &ADocument)
{
	QRegExp body("<body.*>(.*)</body>");
	body.setMinimal(false);
// *** <<< eyeCU <<< ***
	QDomDocument doc;
	QDomElement  html=doc.createElementNS(NS_XHTML_IM, "html");
	QDomElement  body=doc.createElementNS(NS_XHTML, "body");
	html.appendChild(body);
	doc.appendChild(html);
	XmlTextDocumentParser::textToXml(body, ADocument, true);
	if (body.firstChild().isElement() && body.firstChild().toElement().tagName()=="p")
	{
		QDomElement firstP=body.firstChildElement("p");
		if (checkBlockStyle(firstP.attribute("style")))
			firstP.setTagName("span");
	}
	QString xhtml=doc.toString(-1);	
	if (xhtml.indexOf(regExpBody)>=0)
		xhtml=regExpBody.cap(1).trimmed();
	xhtml.replace(QChar::Nbsp, "&#160;");
// *** >>> eyeCU >>> ***
	return xhtml;
}

QString TextManager::getTextFragmentHref(const QTextDocumentFragment &AFragment)
{
	QString href;

	QTextDocument doc;
	doc.setHtml(AFragment.toHtml());

	QTextBlock block = doc.firstBlock();
	while (block.isValid())
	{
		for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
		{
			if (it.fragment().charFormat().isAnchor())
			{
				if (href.isNull())
					href = it.fragment().charFormat().anchorHref();
				else if (href != it.fragment().charFormat().anchorHref())
					return QString::null;
			}
			else
			{
				return QString::null;
			}
		}
		block = block.next();
	}

	return href;
}

void TextManager::insertQuotedFragment(QTextCursor ACursor, const QTextDocumentFragment &AFragment)
{
	if (!AFragment.isEmpty())
	{
		QTextDocument doc;
		QTextCursor cursor(&doc);
		cursor.insertFragment(AFragment);

		cursor.movePosition(QTextCursor::Start);
		do { cursor.insertText("> "); } while (cursor.movePosition(QTextCursor::NextBlock));
		cursor.select(QTextCursor::Document);

		ACursor.beginEditBlock();
		if (!ACursor.atBlockStart())
// *** <<< eyeCU <<< ***
			ACursor.insertBlock();
		int start = ACursor.position();
		ACursor.insertFragment(AFragment);
		ACursor.setPosition(start);
		ACursor.setCharFormat(QTextCharFormat());
		ACursor.insertText("> ");
		do
		{
			QTextBlock block = ACursor.block();
			QList<QPair<int,int> > badUrls;
			for (QTextBlock::Iterator it = block.begin(); it!=block.end(); ++it)
			{
				QTextCharFormat format = it.fragment().charFormat();
				if (format.isAnchor() && format.anchorHref().startsWith("muc:"))
					badUrls.append(qMakePair(it.fragment().position(), it.fragment().length()));
			}

			for (QList<QPair<int,int> >::ConstIterator it = badUrls.constBegin(); it != badUrls.constEnd(); ++it)
			{
				ACursor.setPosition((*it).first);
				ACursor.setPosition((*it).first+(*it).second, QTextCursor::KeepAnchor);
				QTextCharFormat format = ACursor.charFormat();
				format.clearProperty(QTextFormat::AnchorHref);
				format.clearProperty(QTextFormat::AnchorName);
				format.clearProperty(QTextFormat::IsAnchor);
				if (format.underlineStyle()==QTextCharFormat::SingleUnderline)
					format.clearProperty(QTextFormat::TextUnderlineStyle);
				if (format.fontUnderline())
					format.clearProperty(QTextFormat::FontUnderline);
				ACursor.setCharFormat(format);
			}
		} while (ACursor.movePosition(QTextCursor::NextBlock));
		ACursor.movePosition(QTextCursor::EndOfBlock);
		ACursor.insertBlock(QTextBlockFormat(),QTextCharFormat());
// *** >>> eyeCU >>> ***
		ACursor.endEditBlock();
	}
}

QTextDocumentFragment TextManager::getTrimmedTextFragment(const QTextDocumentFragment &AFragment, bool APlainText)
{
	QTextDocument doc;
	QTextCursor cursor(&doc);
	if (APlainText)
	{
		QString text = AFragment.toPlainText();
		text.remove(QChar::Null);
		text.remove(QChar::ObjectReplacementCharacter);
		cursor.insertText(text);
	}
	else
	{
		cursor.insertFragment(AFragment);
	}

	cursor.movePosition(QTextCursor::Start);
	while (cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor) && cursor.selectedText().trimmed().isEmpty())
		cursor.removeSelectedText();

	cursor.movePosition(QTextCursor::End);
	while (cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor) && cursor.selectedText().trimmed().isEmpty())
		cursor.removeSelectedText();

	cursor.select(QTextCursor::Document);
	return cursor.selection();
}

QString TextManager::getElidedString(const QString &AString, Qt::TextElideMode AMode, int AMaxChars)
{
	if (AString.length()>AMaxChars && AMaxChars>3)
	{
		int stringChars = AMode!=Qt::ElideNone ? AMaxChars-3 : AMaxChars;

		QString string;
		if (AMode == Qt::ElideRight)
		{
			string = AString.left(stringChars);
			string.append("...");
		}
		else if (AMode == Qt::ElideLeft)
		{
			string = AString.right(stringChars);
			string.prepend("...");
		}
		else if (AMode == Qt::ElideMiddle)
		{
			QString leftString = AString.left(stringChars/2);
			QString rightString = AString.right(stringChars - stringChars/2);
			string = leftString+"..."+rightString;
		}
		else
		{
			string = AString.left(stringChars);
		}

		return string;
	}
	return AString;
}

bool TextManager::checkBlockStyle(const QString &AStyle)
{
	bool isDefaultBlock(true);
	QStringList splitted = AStyle.split(';');
	for (QStringList::ConstIterator it = splitted.constBegin(); it!=splitted.constEnd(); ++it)
	{
		QStringList pair = (*it).split(':');
		if (pair.size()==2)
		{
			QString name=pair.at(0).trimmed();
			QString value=pair.at(1).trimmed();
			if (name=="margin-bottom" || name=="margin-top")
			{
				if (value!="0px")
				{
					isDefaultBlock=false;
					break;
				}
			}
			else if (name=="white-space")
			{
				if (value!="pre-wrap")
				{
					isDefaultBlock=false;
					break;
				}
			}
			else if (name=="text-indent" || name=="text-align")
			{
				isDefaultBlock=false;
				break;
			}
		}
	}
	return isDefaultBlock;
}

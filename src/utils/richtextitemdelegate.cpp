#include <QTextDocument>
#include <QTextCursor>
#include "richtextitemdelegate.h"

void RichTextItemDelegate::paint(QPainter *APainter, const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const
{
	QStyleOptionViewItem option = AOption;
	initStyleOption(&option, AIndex);

	APainter->save();

	QTextDocument doc;
	QTextCursor cursor(&doc);
	doc.setHtml(option.text);

	/* Call this to get the focus rect and selection background. */
	option.text = "";
	option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, APainter);

	/* Draw using our rich text document. */
	APainter->translate(option.rect.left(), option.rect.top());
	QRect clip(0, 0, option.rect.width(), option.rect.height());
	doc.drawContents(APainter, clip);

	APainter->restore();
}

QSize RichTextItemDelegate::sizeHint(const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const
{
	QStyleOptionViewItem options = AOption;
	initStyleOption(&options, AIndex);

	QTextDocument doc;
	doc.setHtml(options.text);
	doc.setTextWidth(options.rect.width());
	return QSize(doc.idealWidth(), doc.size().height());
}

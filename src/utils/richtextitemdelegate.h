#ifndef RICHTEXTITEMDELEGATE_H
#define RICHTEXTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include "utilsexport.h"

class UTILS_EXPORT RichTextItemDelegate: public QStyledItemDelegate
{
	Q_OBJECT

protected:
	virtual void paint(QPainter *APainter, const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const override;
	virtual QSize sizeHint(const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const override;
};

#endif // RICHTEXTITEMDELEGATE_H

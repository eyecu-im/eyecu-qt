#ifndef TYPEITEMDELEGATE_H
#define TYPEITEMDELEGATE_H

#include <QStyledItemDelegate>

class TypeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TypeItemDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent){}
    void    paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize   sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const;
};

#endif // TYPEITEMDELEGATE_H

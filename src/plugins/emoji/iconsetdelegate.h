#ifndef ICONSETDELEGATE_H
#define ICONSETDELEGATE_H

#include <QHash>
#include <QPainter>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <interfaces/iemoji.h>

class IconsetDelegate :
	public QStyledItemDelegate
{
public:
	enum DataRole {
		IDR_NAME             = Qt::UserRole,
		IDR_ICON_ROW_COUNT,
		IDR_HIDE_SET_NAME
	};
public:
	IconsetDelegate(IEmoji *AEmoji, QObject *AParent = NULL);
	~IconsetDelegate();
	virtual void paint(QPainter *APainter, const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const;
	virtual QSize sizeHint(const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const;
protected:
	virtual bool editorEvent(QEvent *AEvent, QAbstractItemModel *AModel, const QStyleOptionViewItem &AOption, const QModelIndex &AIndex);
protected:
	void drawBackground(QPainter *APainter, const QStyleOptionViewItemV4 &AIndexOption) const;
	void drawFocusRect(QPainter *APainter, const QStyleOptionViewItemV4 &AIndexOption, const QRect &ARect) const;
	QRect checkButtonRect(const QStyleOptionViewItemV4 &AIndexOption, const QRect &ABounding, const QVariant &AValue) const;
	void drawCheckButton(QPainter *APainter, const QStyleOptionViewItemV4 &AIndexOption, const QRect &ARect, Qt::CheckState AState) const;
	QStyleOptionViewItemV4 indexStyleOption(const QStyleOptionViewItem &AOption, const QModelIndex &AIndex) const;
private:
	IEmoji *FEmoji;
};

#endif // ICONSETDELEGATE_H

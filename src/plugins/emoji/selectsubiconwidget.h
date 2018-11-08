#ifndef SELECTSUBICONWIDGET_H
#define SELECTSUBICONWIDGET_H

#include <QWidget>
#include <interfaces/iemoji.h>

class SelectSubIconWidget : public QWidget
{
	Q_OBJECT
public:
	SelectSubIconWidget(const QStringList &AEmojiIds, IEmoji *AEmoji, QWidget *AParent = nullptr);
	~SelectSubIconWidget();
signals:

public slots:

private:
	IEmoji *FEmoji;
	const QStringList FEmojiIds;
};

#endif // SELECTSUBICONWIDGET_H

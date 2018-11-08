#ifndef SELECTSUBICONMENU_H
#define SELECTSUBICONMENU_H

#include <QVBoxLayout>
#include <interfaces/iemoji.h>
#include <utils/menu.h>

class SelectSubIconMenu:
	public Menu
{
	Q_OBJECT
public:
	SelectSubIconMenu(IEmoji *AEmoji, QWidget *AParent = nullptr);
	~SelectSubIconMenu();

	void setEmojiIds(const QStringList &AEmojiIds);

	// QWidget
	virtual QSize sizeHint() const;


signals:
	void iconSelected(const QString &AIconKey);

protected slots:
	void onAboutToShow();

private:
	QVBoxLayout *FLayout;
	IEmoji *FEmoji;
	QStringList FEmojiIds;
};

#endif // SELECTSUBICONMENU_H

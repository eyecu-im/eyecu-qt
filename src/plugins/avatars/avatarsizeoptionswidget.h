#ifndef AVATARSIZEOPTIONSWIDGET_H
#define AVATARSIZEOPTIONSWIDGET_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class AvatarSizeOptionsWidget;
}

class AvatarSizeOptionsWidget:
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
public:
	explicit AvatarSizeOptionsWidget(QWidget *parent = 0);
	~AvatarSizeOptionsWidget();
	// IOptionsDialogWidget interface
	virtual QWidget *instance() {return this;}
public slots:
	virtual void apply();
	virtual void reset();
protected slots:
	void onValueChanged(int AValue);
signals:
	void modified();
	void childApply();
	void childReset();
private:
	Ui::AvatarSizeOptionsWidget *ui;
};

#endif // AVATARSIZEOPTIONSWIDGET_H

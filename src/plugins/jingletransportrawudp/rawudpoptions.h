#ifndef RAWUDPOPTIONS_H
#define RAWUDPOPTIONS_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class RawUdpOptions;
}

class RawUdpOptions:
	public QWidget, public IOptionsDialogWidget
{
	Q_OBJECT

public:
	explicit RawUdpOptions(QWidget *parent = 0);
	~RawUdpOptions();

	// IOptionsDialogWidget interface
public:
	virtual QWidget *instance() {return this;}

public slots:
	virtual void apply();
	virtual void reset();

signals:
	virtual void modified();
	virtual void childApply();
	virtual void childReset();

private:
	Ui::RawUdpOptions *ui;
};

#endif // RAWUDPOPTIONS_H

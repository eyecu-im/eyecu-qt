#ifndef RAWUDPOPTIONS_H
#define RAWUDPOPTIONS_H

#include <QWidget>
#include <QNetworkInterface>
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

	// IOptionsDialogWidget
	virtual QWidget *instance() {return this;}

public slots:
	virtual void apply();
	virtual void reset();

protected:
	static bool isLoopback(const QHostAddress &AHostAddress);

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::RawUdpOptions *ui;
};

#endif // RAWUDPOPTIONS_H

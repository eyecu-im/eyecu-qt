#ifndef OTROPTIONS_H
#define OTROPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class OtrOptions;
}

class OtrOptions:
		public QWidget,
		public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
public:
	explicit OtrOptions(QWidget *AParent = 0);
	~OtrOptions();

private:
	Ui::OtrOptions *ui;

	// IOptionsDialogWidget interface
public:
	virtual QWidget *instance() override;

public slots:
	virtual void apply() override;
	virtual void reset() override;

signals:
	void modified();
	void childApply();
	void childReset();
};

#endif // OTROPTIONS_H

#ifndef SETTINGSHERE_H
#define SETTINGSHERE_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class SettingsHere;
}

class SettingsHere:
		public QWidget,
		public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	explicit SettingsHere(QWidget *parent = nullptr);
	~SettingsHere();

	// IOptionsDialogWidget interface
	virtual QWidget *instance() override {return this;}

public slots:
	virtual void apply() override;
	virtual void reset() override;

protected:
	// QWidget interface
	virtual void changeEvent(QEvent *e) override;

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::SettingsHere *ui;
};

#endif // SETTINGSHERE_H

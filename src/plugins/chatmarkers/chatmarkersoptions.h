#ifndef CHATMARKERSOPTIONS_H
#define CHATMARKERSOPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class ChatMarkersOptions;
}

class ChatMarkersOptions:
		public QWidget,
		public IOptionsDialogWidget
{
	Q_OBJECT

public:
	ChatMarkersOptions(QWidget *AParent = nullptr);
	~ChatMarkersOptions();

	// IOptionsDialogWidget interface
	QWidget *instance();

public slots:
	void apply();
	void reset();

protected slots:
	void onShowDisplayedToggled(bool AChecked);
	void onShowAckToggled(bool AChecked);

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::ChatMarkersOptions *ui;
};

#endif // CHATMARKERSOPTIONS_H

#ifndef ICEOPTIONS_H
#define ICEOPTIONS_H

#include <QWidget>
#include <QTreeWidgetItem>

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class IceOptions;
}

class IceOptions:
	public QWidget,
	public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	explicit IceOptions(QWidget *parent = nullptr);
	virtual ~IceOptions();

	// IOptionsDialogWidget interface
	virtual QWidget *instance() override { return this; }

public slots:
	virtual void apply() override;
	virtual void reset() override;

protected slots:
	void onAdd();
	void onRemove();
	void onCurrentItemChanged(QTreeWidgetItem *ACurrent, QTreeWidgetItem *APrevious);

signals:
	void modified() override;
	void childApply() override;
	void childReset() override;

private:
	Ui::IceOptions *ui;
};

#endif // ICEOPTIONS_H

#ifndef MAPSEARCHPROVIDERHEREOPTIONS_H
#define MAPSEARCHPROVIDERHEREOPTIONS_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class MapSearchProviderHereOptions;
}

class MapSearchProviderHere;

class MapSearchProviderHereOptions:
		public QWidget,
		public IOptionsDialogWidget
{
	Q_OBJECT

public:
	explicit MapSearchProviderHereOptions(MapSearchProviderHere *AMapSearchProviderHere, QWidget *AParent = nullptr);
	~MapSearchProviderHereOptions();

	// IOptionsDialogWidget interface
	virtual QWidget *instance() override {return this;}

public slots:
	virtual void apply() override;
	virtual void reset() override;

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::MapSearchProviderHereOptions *ui;
	MapSearchProviderHere *FMapSearchProviderHere;

	// QWidget interface
protected:
	virtual void changeEvent(QEvent *e) override;
};

#endif // MAPSEARCHPROVIDERHEREOPTIONS_H

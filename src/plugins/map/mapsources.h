#ifndef MAPSOURCES_H
#define MAPSOURCES_H

#include <interfaces/imap.h>
#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

namespace Ui {
class MapSources;
}

class MapSources : public QWidget, public IOptionsDialogWidget
{
	Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
	MapSources(IOptionsManager *AOptionsManager, QWidget *AParent = 0);
	~MapSources();
	void addMapSource(const IOptionsDialogNode &ANode);

	// IOptionsDialogWidget interface
public:
	QWidget *instance() {return this;}

public slots:
	void apply(){}
	void reset(){}

protected slots:
	void onButtonClicked();

signals:
	void modified();
	void childApply();
	void childReset();

private:
	Ui::MapSources *ui;
	QHash<QPushButton *, QString> FNodeHash;
	IOptionsManager *FOptionsManager;
	IconStorage		*FIconStorage;
};

#endif // MAPSOURCES_H

#include <QPushButton>
#include <definitions/optionnodes.h>
#include <definitions/resources.h>
#include "ui_mapsources.h"
#include "mapsources.h"

MapSources::MapSources(IOptionsManager *AOptionsManager, QWidget *AParent) :
	QWidget(AParent),
	ui(new Ui::MapSources),
	FOptionsManager(AOptionsManager),
	FIconStorage(IconStorage::staticStorage(RSR_STORAGE_MENUICONS))
{
	ui->setupUi(this);
}

MapSources::~MapSources()
{
	delete ui;
}

void MapSources::addMapSource(const IOptionsDialogNode &ANode)
{
	QPushButton *button = new QPushButton(FIconStorage->getIcon(ANode.iconkey), ANode.caption, this);
	FNodeHash.insert(button, ANode.nodeId);
	layout()->addWidget(button);
	connect(button, SIGNAL(clicked()), SLOT(onButtonClicked()));
}

void MapSources::onButtonClicked()
{
	FOptionsManager->showOptionsDialog(FNodeHash.value(qobject_cast<QPushButton*>(sender())), OPN_MAP, window());
}

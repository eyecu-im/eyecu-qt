#include <definitions/optionvalues.h>
#include "mapsearchprovider2gis.h"
#include "mapsearchprovider2gisoptions.h"
#include "ui_mapsearchprovider2gisoptions.h"

MapSearchProvider2GisOptions::MapSearchProvider2GisOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapSearchProvider2GisOptions)
{
    ui->setupUi(this);
    reset();
}

MapSearchProvider2GisOptions::~MapSearchProvider2GisOptions()
{
    delete ui;
}

void MapSearchProvider2GisOptions::apply()
{
    if (ui->rbtFirm->isChecked())
        Options::node(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE).setValue(SearchFirm);
    else if (ui->rbtGeoObject->isChecked())
        Options::node(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE).setValue(SearchGeo);
}

void MapSearchProvider2GisOptions::reset()
{
    switch (Options::node(OPV_MAP_SEARCH_PROVIDER_2GIS_TYPE).value().toInt())
    {
        case SearchFirm:
            ui->rbtFirm->setChecked(true);
            break;
        case SearchGeo:
            ui->rbtGeoObject->setChecked(true);
            break;
    }
}

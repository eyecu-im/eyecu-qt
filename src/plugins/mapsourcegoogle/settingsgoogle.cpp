#include <definitions/optionvalues.h>
#include <utils/options.h>
#include "settingsgoogle.h"
#include "ui_settingsgoogle.h"

SettingsGoogle::SettingsGoogle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsGoogle)
{
    ui->setupUi(this);
    reset();
    connect(ui->spbSatelliteVersion, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->spbMapVersion, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->spbTerrainVersionRoads, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->spbTerrainVersionTerrain, SIGNAL(valueChanged(int)), SIGNAL(modified()));
}

SettingsGoogle::~SettingsGoogle()
{
    delete ui;
}

void SettingsGoogle::apply()
{
    Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_SATELLITE).setValue(ui->spbSatelliteVersion->value());
    Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_MAP).setValue(ui->spbMapVersion->value());
    Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_R).setValue(ui->spbTerrainVersionRoads->value());
    Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_T).setValue(ui->spbTerrainVersionTerrain->value());
    emit childApply();
}

void SettingsGoogle::reset()
{
    ui->spbSatelliteVersion->setValue(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_SATELLITE).value().toInt());
    ui->spbMapVersion->setValue(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_MAP).value().toInt());
    ui->spbTerrainVersionRoads->setValue(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_R).value().toInt());
    ui->spbTerrainVersionTerrain->setValue(Options::node(OPV_MAP_SOURCE_GOOGLE_VERSION_TERRAIN_T).value().toInt());
    emit childReset();
}

void SettingsGoogle::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void SettingsGoogle::onDefaultClicked()
{
    Options::node(OPV_MAP_SOURCE_GOOGLE).removeNode("version");
    ui->spbSatelliteVersion->blockSignals(true);
    ui->spbMapVersion->blockSignals(true);
    ui->spbTerrainVersionRoads->blockSignals(true);
    ui->spbTerrainVersionTerrain->blockSignals(true);
    reset();
    ui->spbSatelliteVersion->blockSignals(false);
    ui->spbMapVersion->blockSignals(false);
    ui->spbTerrainVersionRoads->blockSignals(false);
    ui->spbTerrainVersionTerrain->blockSignals(false);
}

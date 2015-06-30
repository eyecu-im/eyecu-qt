#include <definitions/optionvalues.h>
#include <utils/options.h>
#include "optionsyandex.h"
#include "ui_optionsyandex.h"

OptionsYandex::OptionsYandex(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsYandex)
{
    ui->setupUi(this);
    reset();
    connect(ui->ledSatellitePhotoVersion, SIGNAL(textChanged(QString)), SIGNAL(modified()));
    connect(ui->ledSchemeVersion, SIGNAL(textChanged(QString)), SIGNAL(modified()));
}

OptionsYandex::~OptionsYandex()
{
    delete ui;
}

void OptionsYandex::apply()
{
    Options::node(OPV_MAP_SOURCE_YANDEX_VERSION_SATELLITE).setValue(ui->ledSatellitePhotoVersion->text());
    Options::node(OPV_MAP_SOURCE_YANDEX_VERSION_SCHEME).setValue(ui->ledSchemeVersion->text());
    emit childApply();
}

void OptionsYandex::reset()
{
    ui->ledSatellitePhotoVersion->setText(Options::node(OPV_MAP_SOURCE_YANDEX_VERSION_SATELLITE).value().toString());
    ui->ledSchemeVersion->setText(Options::node(OPV_MAP_SOURCE_YANDEX_VERSION_SCHEME).value().toString());
    emit childReset();
}

void OptionsYandex::changeEvent(QEvent *e)
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

void OptionsYandex::onDefaultClicked()
{
    Options::node(OPV_MAP_SOURCE_YANDEX).removeNode("version");
    ui->ledSatellitePhotoVersion->blockSignals(true);
    ui->ledSchemeVersion->blockSignals(true);
    reset();
    ui->ledSatellitePhotoVersion->blockSignals(false);
    ui->ledSchemeVersion->blockSignals(false);
}

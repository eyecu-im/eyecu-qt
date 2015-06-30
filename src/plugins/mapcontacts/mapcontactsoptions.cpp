#include "mapcontactsoptions.h"
#include "ui_mapcontactsoptions.h"
#include "definitions/optionvalues.h"

MapContactsOptions::MapContactsOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapContactsOptions)
{
    ui->setupUi(this);
    reset();
	connect(ui->cmbMapContactsView, SIGNAL(currentIndexChanged(int)), this, SIGNAL(modified()));
    connect(ui->cbFollow, SIGNAL(toggled(bool)), this, SIGNAL(modified()));
}

MapContactsOptions::~MapContactsOptions()
{
    delete ui;
}

void MapContactsOptions::apply()
{
	Options::node(OPV_MAP_CONTACTS_VIEW).setValue(ui->cmbMapContactsView->currentIndex());
    Options::node(OPV_MAP_CONTACTS_FOLLOW).setValue(ui->cbFollow->isChecked());
    emit childApply();
}

void MapContactsOptions::reset()
{
	ui->cmbMapContactsView->setCurrentIndex(Options::node(OPV_MAP_CONTACTS_VIEW).value().toInt());
    ui->cbFollow->setChecked(Options::node(OPV_MAP_CONTACTS_FOLLOW).value().toBool());
    emit childReset();
}

void MapContactsOptions::changeEvent(QEvent *e)
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

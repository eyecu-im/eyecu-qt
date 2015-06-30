#include <QColorDialog>
#include <definitions/optionvalues.h>
#include "mapsearchoptions.h"
#include "ui_mapsearchoptions.h"

MapSearchOptions::MapSearchOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapSearchOptions)
{
    ui->setupUi(this);
    reset();
}

MapSearchOptions::~MapSearchOptions()
{
    delete ui;
}

void MapSearchOptions::changeEvent(QEvent *e)
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

void MapSearchOptions::onTextColorDialog()
{
    QColor color = QColorDialog::getColor(FCurrentColor, this, tr("Select label color"));
    if(color.isValid())
        if (FCurrentColor!=color)
        {
            FCurrentColor = color;
            setWidgetColor(ui->pbTextColor, FCurrentColor);
            emit modified();
        }
}

void MapSearchOptions::apply()
{
    Options::node(OPV_MAP_SEARCH_LABEL_COLOR).setValue(FCurrentColor);
}

void MapSearchOptions::reset()
{
    setWidgetColor(ui->pbTextColor, FCurrentColor = Options::node(OPV_MAP_SEARCH_LABEL_COLOR).value().value<QColor>());
}


void MapSearchOptions::setWidgetColor(QWidget *AWidget, const QColor &AColor)
{
    QColor contrast(AColor.black()<128?Qt::black:Qt::white);
    QPalette palette=AWidget->palette();
    palette.setColor(QPalette::Button, AColor);
    palette.setColor(QPalette::ButtonText, contrast);
    AWidget->setPalette(palette);
}

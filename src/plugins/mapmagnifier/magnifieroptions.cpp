#include <QColorDialog>
#include <QFontDialog>
#include <QPalette>
#include <definitions/optionvalues.h>
#include "magnifieroptions.h"
#include "ui_magnifieroptions.h"

MagnifierOptions::MagnifierOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MagnifierOptions)
{
    ui->setupUi(this);

    connect(ui->chkEnableCentralRulers, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->chkEnableScale, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->chkShowObjects, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->chkHighPrecision, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->chkZoomFactorDisplay, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->hslBlurRadius, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslBlurRadius, SIGNAL(valueChanged(int)), ui->lcdBlurRadius, SLOT(display(int)));
    connect(ui->hslSize, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslSize, SIGNAL(valueChanged(int)), ui->lcdSize, SLOT(display(int)));
    connect(ui->hslZoomFactor, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslZoomFactor, SIGNAL(valueChanged(int)), ui->lcdZoomFactor, SLOT(display(int)));
    connect(ui->hslHorizontalShift, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslHorizontalShift, SIGNAL(valueChanged(int)), ui->lcdHorizontalShift, SLOT(display(int)));
    connect(ui->hslVerticalShift, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslVerticalShift, SIGNAL(valueChanged(int)), ui->lcdVerticalShift, SLOT(display(int)));
    connect(ui->hslOpacity, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslOpacity, SIGNAL(valueChanged(int)), ui->lcdOpacity, SLOT(display(int)));
    connect(ui->hslZoomFactorOpacity, SIGNAL(valueChanged(int)), SIGNAL(modified()));
    connect(ui->hslZoomFactorOpacity, SIGNAL(valueChanged(int)), ui->lcdZoomFactorOpacity, SLOT(display(int)));
    connect(ui->btnShadowColor, SIGNAL(clicked()), SLOT(onShadowColorButtonClicked()));
    connect(ui->btnZoomFactorColor, SIGNAL(clicked()), SLOT(onZoomFactorColorButtonClicked()));
    connect(ui->btnZoomFactorFont, SIGNAL(clicked()), SLOT(onZoomFactorFontButtonClicked()));

    reset();

    ui->lcdSize->display(ui->hslSize->value());
    ui->lcdZoomFactor->display(ui->hslZoomFactor->value());
}

MagnifierOptions::~MagnifierOptions()
{
    delete ui;
}

void MagnifierOptions::apply()
{
    Options::node(OPV_MAP_MAGNIFIER_SCALE).setValue(ui->chkEnableScale->isChecked());
    Options::node(OPV_MAP_MAGNIFIER_RULERS).setValue(ui->chkEnableCentralRulers->isChecked());
    Options::node(OPV_MAP_MAGNIFIER_OBJECTS).setValue(ui->chkShowObjects->isChecked());
    Options::node(OPV_MAP_MAGNIFIER_HIGHPRECISION).setValue(ui->chkHighPrecision->isChecked());
    Options::node(OPV_MAP_MAGNIFIER_SIZE).setValue(ui->hslSize->value());
    Options::node(OPV_MAP_MAGNIFIER_ZOOM).setValue(ui->hslZoomFactor->value());
    Options::node(OPV_MAP_MAGNIFIER_SHADOW_BLUR).setValue(ui->hslBlurRadius->value());
    Options::node(OPV_MAP_MAGNIFIER_SHADOW_OPACITY).setValue(ui->hslOpacity->value());
    Options::node(OPV_MAP_MAGNIFIER_SHADOW_COLOR).setValue(FShadowColor);
    Options::node(OPV_MAP_MAGNIFIER_SHADOW_SHIFT).setValue(QPointF(ui->hslHorizontalShift->value(), ui->hslVerticalShift->value()));
    Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR).setValue(ui->chkZoomFactorDisplay->isChecked());
    Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY).setValue(ui->hslZoomFactorOpacity->value()/255.0);
    Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR).setValue(FZoomFactorColor);
    Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT).setValue(FZoomFactorFont);
    emit childApply();
}

void MagnifierOptions::reset()
{
    ui->chkEnableScale->setChecked(Options::node(OPV_MAP_MAGNIFIER_SCALE).value().toBool());
    ui->chkEnableCentralRulers->setChecked(Options::node(OPV_MAP_MAGNIFIER_RULERS).value().toBool());
    ui->chkShowObjects->setChecked(Options::node(OPV_MAP_MAGNIFIER_OBJECTS).value().toBool());
    ui->chkHighPrecision->setChecked(Options::node(OPV_MAP_MAGNIFIER_HIGHPRECISION).value().toBool());
    ui->chkZoomFactorDisplay->setChecked(Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR).value().toBool());
    ui->hslSize->setValue(Options::node(OPV_MAP_MAGNIFIER_SIZE).value().toInt());
    ui->hslZoomFactor->setValue(Options::node(OPV_MAP_MAGNIFIER_ZOOM).value().toInt());
    ui->hslOpacity->setValue(Options::node(OPV_MAP_MAGNIFIER_SHADOW_OPACITY).value().toInt());
    ui->hslZoomFactorOpacity->setValue(Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_OPACITY).value().toFloat()*255);
    ui->hslBlurRadius->setValue(Options::node(OPV_MAP_MAGNIFIER_SHADOW_BLUR).value().toInt());
    QPointF shift=Options::node(OPV_MAP_MAGNIFIER_SHADOW_SHIFT).value().value<QPointF>();
    ui->hslHorizontalShift->setValue(shift.x());
    ui->hslVerticalShift->setValue(shift.y());    

    FZoomFactorFont=Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_FONT).value().value<QFont>();
    ui->lblZoomFactorFont->setFont(FZoomFactorFont);
    ui->lblZoomFactorFont->setText(FZoomFactorFont.toString());

    FShadowColor=Options::node(OPV_MAP_MAGNIFIER_SHADOW_COLOR).value().value<QColor>();
    QColor contrast(FShadowColor.black()<128?Qt::black:Qt::white);
    QPalette palette=ui->btnShadowColor->palette();
    palette.setColor(QPalette::Button, FShadowColor);
    palette.setColor(QPalette::ButtonText, contrast);
    ui->btnShadowColor->setPalette(palette);

    FZoomFactorColor=Options::node(OPV_MAP_MAGNIFIER_ZOOMFACTOR_COLOR).value().value<QColor>();
    contrast=(FZoomFactorColor.black()<128?Qt::black:Qt::white);
    palette=ui->btnZoomFactorColor->palette();
    palette.setColor(QPalette::Button, FZoomFactorColor);
    palette.setColor(QPalette::ButtonText, contrast);
    ui->btnZoomFactorColor->setPalette(palette);
}

void MagnifierOptions::onShadowColorButtonClicked()
{
    QColor color=QColorDialog::getColor(FShadowColor, this, tr("Choose shadow color"));
    if (color.isValid())
        if (FShadowColor!=color)
        {
            FShadowColor=color;
            QColor contrast(FShadowColor.black()<128?Qt::black:Qt::white);
            QPalette palette=ui->btnShadowColor->palette();
            palette.setColor(QPalette::Button, FShadowColor);
            palette.setColor(QPalette::ButtonText, contrast);
            ui->btnShadowColor->setPalette(palette);
            emit modified();
        }
}

void MagnifierOptions::onZoomFactorColorButtonClicked()
{
    QColor color=QColorDialog::getColor(FZoomFactorColor, this, tr("Choose zoom factor color"));
    if (color.isValid())
        if (FZoomFactorColor!=color)
        {
            FZoomFactorColor=color;
            QColor contrast(FZoomFactorColor.black()<128?Qt::black:Qt::white);
            QPalette palette=ui->btnZoomFactorColor->palette();
            palette.setColor(QPalette::Button, FZoomFactorColor);
            palette.setColor(QPalette::ButtonText, contrast);
            ui->btnZoomFactorColor->setPalette(palette);
            emit modified();
        }
}

void MagnifierOptions::onZoomFactorFontButtonClicked()
{
    bool ok;
    QFont font=QFontDialog::getFont(&ok, FZoomFactorFont, this, tr("Choose zoom factor font"));
    if (ok)
    {
        if (FZoomFactorFont!=font)
        {
            FZoomFactorFont=font;
            ui->lblZoomFactorFont->setText(font.toString());
            ui->lblZoomFactorFont->setFont(font);
            emit modified();
        }
    }
}

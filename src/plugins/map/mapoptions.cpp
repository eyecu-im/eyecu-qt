#include <QFontDialog>
#include <QColorDialog>

#include "mapoptions.h"
#include "ui_mapoptions.h"
#include "definitions/optionvalues.h"

MapOptions::MapOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapOptions)
{
    ui->setupUi(this);
    reset();

    connect(ui->cmbBoxShape,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cmbBoxShadow,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cbBoxBgTransparent,SIGNAL(toggled(bool)),SIGNAL(modified()));
    connect(ui->cbCntrlBgTransparent,SIGNAL(toggled(bool)),SIGNAL(modified()));
	connect(ui->cbCenterMarkerVisible,SIGNAL(toggled(bool)),SIGNAL(modified()));
}

MapOptions::~MapOptions()
{
    delete ui;
}

void MapOptions::apply()
{
    int shape;
    switch (ui->cmbBoxShape->currentIndex())
    {
        case 1: shape=QFrame::Box; break;
        case 2: shape=QFrame::Panel; break;
        case 3: shape=QFrame::WinPanel; break;
        case 4: shape=QFrame::StyledPanel; break;
        default: shape=QFrame::NoFrame; break;
    }
    Options::node(OPV_MAP_OSD_BOX_SHAPE).setValue(shape);

    int shadow;
    switch (ui->cmbBoxShadow->currentIndex())
    {
        case 1: shadow=QFrame::Raised; break;
        case 2: shadow=QFrame::Sunken; break;
        default: shadow=QFrame::Plain; break;
    }
    Options::node(OPV_MAP_OSD_BOX_SHADOW).setValue(shadow);

    Options::node(OPV_MAP_OSD_TEXT_COLOR).setValue(currColor);

    Options::node(OPV_MAP_OSD_CONTR_FOREGROUND).setValue(contrForeground);
    Options::node(OPV_MAP_OSD_CONTR_BASE).setValue(contrBase);
    Options::node(OPV_MAP_OSD_CONTR_BUTTON).setValue(contrButton);
    Options::node(OPV_MAP_OSD_CONTR_LIGHT).setValue(contrLight);
    Options::node(OPV_MAP_OSD_CONTR_MIDLIGHT).setValue(contrMidlight);
    Options::node(OPV_MAP_OSD_CONTR_SHADOW).setValue(contrShadow);
    Options::node(OPV_MAP_OSD_CONTR_DARK).setValue(contrDark);
    Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_COLOR).setValue(contrBackground);
    Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_ALPHA).setValue(contrBackground.alpha());
    Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_TRANSPARENT).setValue(ui->cbCntrlBgTransparent->isChecked());

	Options::node(OPV_MAP_OSD_CMARKER_COLOR).setValue(centerMarker);
	Options::node(OPV_MAP_OSD_CMARKER_ALPHA).setValue(centerMarker.alpha());
	Options::node(OPV_MAP_OSD_CMARKER_VISIBLE).setValue(ui->cbCenterMarkerVisible->isChecked());

    Options::node(OPV_MAP_OSD_BOX_FOREGROUND).setValue(boxForeground);
    Options::node(OPV_MAP_OSD_BOX_LIGHT).setValue(boxLight);
    Options::node(OPV_MAP_OSD_BOX_MIDLIGHT).setValue(boxMidlight);
    Options::node(OPV_MAP_OSD_BOX_DARK).setValue(boxDark);
    Options::node(OPV_MAP_OSD_BOX_BACKGROUND_COLOR).setValue(boxBackground);
    Options::node(OPV_MAP_OSD_BOX_BACKGROUND_ALPHA).setValue(boxBackground.alpha());
    Options::node(OPV_MAP_OSD_BOX_BACKGROUND_TRANSPARENT).setValue(ui->cbBoxBgTransparent->isChecked());

    Options::node(OPV_MAP_OSD_FONT).setValue(currFont);
    Options::node(OPV_MAP_OSD_SHADOW_COLOR).setValue(currShadowColor);
    Options::node(OPV_MAP_OSD_SHADOW_ALPHA).setValue(currShadowColor.alpha());
    Options::node(OPV_MAP_OSD_SHADOW_BLUR).setValue(currShadowBlur);
    Options::node(OPV_MAP_OSD_SHADOW_SHIFT).setValue(currShadowShift);

    emit childApply();
}

void MapOptions::reset()
{
    int index;
    switch (Options::node(OPV_MAP_OSD_BOX_SHAPE).value().toInt())
    {
        case QFrame::Box: index=1; break;
        case QFrame::Panel: index=2; break;
        case QFrame::WinPanel: index=3; break;
        case QFrame::StyledPanel: index=4; break;
        default: index=0; break;
    }
    ui->cmbBoxShape->setCurrentIndex(index);
	onBoxFrameShapeChanged(index);

    switch (Options::node(OPV_MAP_OSD_BOX_SHADOW).value().toInt())
    {
        case QFrame::Raised: index=1; break;
        case QFrame::Sunken: index=2; break;
        default: index=0; break;
    }
    ui->cmbBoxShadow->setCurrentIndex(index);

    currColor = Options::node(OPV_MAP_OSD_TEXT_COLOR).value().value<QColor>();
    setWidgetColor(ui->pbTextColor, currColor);

    contrForeground = Options::node(OPV_MAP_OSD_CONTR_FOREGROUND).value().value<QColor>();
    setWidgetColor(ui->pbCntrlForeground, contrForeground);

    contrBase = Options::node(OPV_MAP_OSD_CONTR_BASE).value().value<QColor>();
    setWidgetColor(ui->pbCntrlBase, contrBase);

    contrButton = Options::node(OPV_MAP_OSD_CONTR_BUTTON).value().value<QColor>();
    setWidgetColor(ui->pbCntrlButton, contrButton);

    contrLight = Options::node(OPV_MAP_OSD_CONTR_LIGHT).value().value<QColor>();
    setWidgetColor(ui->pbCntrlLight, contrLight);

    contrMidlight = Options::node(OPV_MAP_OSD_CONTR_MIDLIGHT).value().value<QColor>();
    setWidgetColor(ui->pbCntrlMidlight, contrMidlight);

    contrShadow = Options::node(OPV_MAP_OSD_CONTR_SHADOW).value().value<QColor>();
    setWidgetColor(ui->pbCntrlShadow, contrShadow);

    contrDark = Options::node(OPV_MAP_OSD_CONTR_DARK).value().value<QColor>();
    setWidgetColor(ui->pbCntrlDark, contrDark);

    contrBackground = Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_COLOR).value().value<QColor>();
    setWidgetColor(ui->pbCntrlBackground, contrBackground);        

    int controlAplpha = Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_ALPHA).value().toInt();
    contrBackground.setAlpha(controlAplpha);
    ui->lcdCntrlBackgroundAlpha->display(controlAplpha);
    ui->slCntrlBackgroundAlpha->setValue(controlAplpha);
	bool checked = Options::node(OPV_MAP_OSD_CONTR_BACKGROUND_TRANSPARENT).value().toBool();
	ui->cbCntrlBgTransparent->setChecked(checked);
	disableCntrlBackgroundAlpha(checked);

	centerMarker = Options::node(OPV_MAP_OSD_CMARKER_COLOR).value().value<QColor>();
	setWidgetColor(ui->pbCenterMarkerColor, centerMarker);

	controlAplpha = Options::node(OPV_MAP_OSD_CMARKER_ALPHA).value().toInt();
	centerMarker.setAlpha(controlAplpha);
	ui->lcdCenterMarkerAlpha->display(controlAplpha);
    ui->slCenterMarkerAlpha->setValue(controlAplpha);
	checked = Options::node(OPV_MAP_OSD_CMARKER_VISIBLE).value().toBool();
	ui->cbCenterMarkerVisible->setChecked(checked);
	enableCenterMarkerAlpha(checked);

    currShadowColor = Options::node(OPV_MAP_OSD_SHADOW_COLOR).value().value<QColor>();
    setWidgetColor(ui->pbShadowColor, currShadowColor);

    int alpha = Options::node(OPV_MAP_OSD_SHADOW_ALPHA).value().toInt();
    currShadowColor.setAlpha(alpha);
    ui->slShadowAlpha->setValue(alpha);
    ui->lcdShadTranspar->display(alpha);    

    boxForeground = Options::node(OPV_MAP_OSD_BOX_FOREGROUND).value().toString();//  .value<QColor>();
    setWidgetColor(ui->pbBoxForeground, boxForeground);
    boxLight = Options::node(OPV_MAP_OSD_BOX_LIGHT).value().toString();//  .value<QColor>();
    setWidgetColor(ui->pbBoxLight, boxLight);
    boxMidlight = Options::node(OPV_MAP_OSD_BOX_MIDLIGHT).value().toString();//  .value<QColor>();
    setWidgetColor(ui->pbBoxMidlight, boxMidlight);
    boxDark = Options::node(OPV_MAP_OSD_BOX_DARK).value().toString();//  .value<QColor>();
    setWidgetColor(ui->pbBoxDark, boxDark);

    boxBackground = Options::node(OPV_MAP_OSD_BOX_BACKGROUND_COLOR).value().toString();//  .value<QColor>();
    setWidgetColor(ui->pbBackgroundColor, boxBackground);
    alpha = Options::node(OPV_MAP_OSD_BOX_BACKGROUND_ALPHA).value().toInt();
    ui->slBackgroundAlpha->setValue(alpha);
    ui->lcdBgColorTranspar->display(alpha);
	checked = Options::node(OPV_MAP_OSD_BOX_BACKGROUND_TRANSPARENT).value().toBool();
	ui->cbBoxBgTransparent->setChecked(checked);
	disableBoxBackgroundAlpha(checked);
    boxBackground.setAlpha(alpha);

    currFont = Options::node(OPV_MAP_OSD_FONT).value().value<QFont>();
    ui->lblFontTest->setFont(currFont);
    ui->lblFontTest->setText(currFont.toString());

    currShadowBlur = Options::node(OPV_MAP_OSD_SHADOW_BLUR).value().toReal();
    ui->slShadowBlur->setValue(currShadowBlur*10);
    ui->lcdShadBlur->display(currShadowBlur);

    currShadowShift = Options::node(OPV_MAP_OSD_SHADOW_SHIFT).value().toPointF();
    ui->slShadowShiftX->setValue(currShadowShift.x()*10);
    ui->lcdShadShiftX->display(currShadowShift.x());
    ui->slShadowShiftY->setValue(currShadowShift.y()*10);
    ui->lcdShadShiftY->display(currShadowShift.y());    

    emit childReset();
}

void MapOptions::modifyFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currFont, this);
    if(ok)
    {
        currFont = font;
        ui->lblFontTest->setFont(currFont);
        ui->lblFontTest->setText(currFont.toString());
        emit modified();
    }
}

void MapOptions::modifyColor()
{
    QColor color = QColorDialog::getColor(currColor, this, tr("Select text color"));
    if(color.isValid())
        if (currColor!=color)
        {
            currColor = color;
            setWidgetColor(ui->pbTextColor, currColor);
            emit modified();
        }
}

void MapOptions::modifyControlForeground()
{
    QColor color = QColorDialog::getColor(contrForeground, this, tr("Select control foreground color"));
    if(color.isValid())
        if (contrForeground!=color)
        {
            setWidgetColor(ui->pbCntrlForeground, contrForeground = color);
            emit modified();
        }
}

void MapOptions::modifyControlBase()
{
    QColor color = QColorDialog::getColor(contrBase, this, tr("Select control base color"));
    if(color.isValid())
        if (contrBase!=color)
        {
            setWidgetColor(ui->pbCntrlBase, contrBase = color);
            emit modified();
        }
}

void MapOptions::modifyControlButton()
{
    QColor color = QColorDialog::getColor(contrButton, this, tr("Select control button color"));
    if(color.isValid())
        if (contrBase!=color)
        {
            setWidgetColor(ui->pbCntrlButton, contrButton = color);
            emit modified();
        }
}

void MapOptions::modifyControlLight()
{
    QColor color = QColorDialog::getColor(contrLight, this, tr("Select control light color"));
    if(color.isValid())
        if (contrLight!=color)
        {
            setWidgetColor(ui->pbCntrlLight, contrLight = color);
            emit modified();
        }
}

void MapOptions::modifyControlMidlight()
{
    QColor color = QColorDialog::getColor(contrMidlight, this, tr("Select control midlight color"));
    if(color.isValid())
        if (contrMidlight!=color)
        {
            setWidgetColor(ui->pbCntrlMidlight, contrMidlight = color);
            emit modified();
        }
}

void MapOptions::modifyControlDark()
{
    QColor color = QColorDialog::getColor(contrDark, this, tr("Select control dark color"));
    if(color.isValid())
        if (contrDark!=color)
        {
            setWidgetColor(ui->pbCntrlDark, contrDark = color);
            emit modified();
        }
}

void MapOptions::modifyControlShadow()
{
    QColor color = QColorDialog::getColor(contrShadow, this, tr("Select control shadow color"));
    if(color.isValid())
        if (contrShadow!=color)
        {
            setWidgetColor(ui->pbCntrlShadow, contrShadow = color);
            emit modified();
        }
}

void MapOptions::modifyControlBackground()
{
    QColor color = QColorDialog::getColor(contrBackground, this, tr("Select control background color"));
    if(color.isValid())
    {
        color.setAlpha(ui->slCntrlBackgroundAlpha->value());
        if (contrBackground!=color)
        {
            setWidgetColor(ui->pbCntrlBackground, contrBackground = color);
            emit modified();
        }
    }
}

void MapOptions::modifyCenterMarkerColor()
{
	QColor color = QColorDialog::getColor(centerMarker, this, tr("Select center marker color"));
    if(color.isValid())
    {
        color.setAlpha(ui->slCenterMarkerAlpha->value());
		if (centerMarker!=color)
        {
			setWidgetColor(ui->pbCenterMarkerColor, centerMarker = color);
            emit modified();
        }
    }
}

void MapOptions::modifyShadowColor()
{
    QColor color = QColorDialog::getColor(currShadowColor, 0, tr("Select shadow color"));
    if(color.isValid())
    {
        color.setAlpha(ui->slShadowAlpha->value());
        if (currShadowColor!=color)
        {
            currShadowColor = color;
            setWidgetColor(ui->pbShadowColor, currShadowColor);
            emit modified();
        }
    }
}

void MapOptions::modifyBoxForeground()
{
    QColor color = QColorDialog::getColor(boxForeground, 0, tr("Select box foreground color"));
    if(color.isValid())
        if (boxForeground!=color)
        {
            setWidgetColor(ui->pbBoxForeground, boxForeground = color);
            emit modified();
        }
}

void MapOptions::modifyBoxLight()
{
    QColor color = QColorDialog::getColor(boxLight, 0, tr("Select box light color"));
    if(color.isValid())
        if (boxLight!=color)
        {
            setWidgetColor(ui->pbBoxLight, boxLight = color);
            emit modified();
        }
}

void MapOptions::modifyBoxMidlight()
{
    QColor color = QColorDialog::getColor(boxMidlight, 0, tr("Select box midlight color"));
    if(color.isValid())
        if (boxMidlight!=color)
        {
            setWidgetColor(ui->pbBoxMidlight, boxMidlight = color);
            emit modified();
        }
}

void MapOptions::modifyBoxDark()
{
    QColor color = QColorDialog::getColor(boxDark, 0, tr("Select box dark color"));
    if(color.isValid())
        if (boxDark!=color)
        {
            setWidgetColor(ui->pbBoxDark, boxDark = color);
            emit modified();
        }
}

void MapOptions::modifyBoxBackground()
{
    QColor color = QColorDialog::getColor(boxBackground, 0, tr("Select box background color"));
    if(color.isValid())
    {
        color.setAlpha(ui->lcdBgColorTranspar->value());
        if (boxBackground!=color)
        {
            setWidgetColor(ui->pbBackgroundColor, boxBackground = color);
            emit modified();
        }
    }
}

void MapOptions::onBackgroundAlphaChange(int value)
{
    boxBackground.setAlpha(value);
    ui->lcdBgColorTranspar->display(value);
    emit modified();
}

void MapOptions::onShadowAlphaChange(int value)
{
    currShadowColor.setAlpha(value);
    ui->lcdShadTranspar->display(value);
    emit modified();
}

void MapOptions::onShadowBlurChange(int blur)
{
    currShadowBlur=blur/10.0;
    ui->lcdShadBlur->display(currShadowBlur);
    emit modified();
}

void MapOptions::onShadowShiftXChange(int cx)
{
    currShadowShift.setX(cx/10.0);
    ui->lcdShadShiftX->display(currShadowShift.x());
    emit modified();
}

void MapOptions::onShadowShiftYChange(int cy)
{
    currShadowShift.setY(cy/10.0);
    ui->lcdShadShiftY->display(currShadowShift.y());
	emit modified();
}

void MapOptions::disableBoxBackgroundAlpha(bool ADisable)
{
	ui->slBackgroundAlpha->setDisabled(ADisable);
}

void MapOptions::disableCntrlBackgroundAlpha(bool ADisable)
{
	ui->slCntrlBackgroundAlpha->setDisabled(ADisable);
}

void MapOptions::enableCenterMarkerAlpha(bool AEnable)
{
	ui->slCenterMarkerAlpha->setEnabled(AEnable);
}

void MapOptions::onBoxFrameShapeChanged(int AIndex)
{
	ui->cmbBoxShadow->setEnabled(AIndex);
}

void MapOptions::onCntrBackgroundAlphaChange(int value)
{
    contrBackground.setAlpha(value);
    ui->lcdCntrlBackgroundAlpha->display(value);
    emit modified();
}

void MapOptions::onCenterMarkerAlphaChange(int value)
{
	centerMarker.setAlpha(value);
	ui->lcdCenterMarkerAlpha->display(value);
    emit modified();
}

void MapOptions::changeEvent(QEvent *e)
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

void MapOptions::setWidgetColor(QWidget *AWidget, const QColor &AColor)
{
    QColor contrast(AColor.black()<128?Qt::black:Qt::white);
    QPalette palette=AWidget->palette();
    palette.setColor(QPalette::Button, AColor);
    palette.setColor(QPalette::ButtonText, contrast);
    AWidget->setPalette(palette);
}

#include <QDebug>

#include "trackoptions.h"

TrackOptions::TrackOptions(ITracker *ATracker, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrackOptions),
    FTracker(ATracker)
{
    ui->setupUi(this);

    connect(ui->rButMap,SIGNAL(clicked()),this,SIGNAL(modified()));
    connect(ui->rButSat,SIGNAL(clicked()),this,SIGNAL(modified()));
    connect(ui->comBoxSize,SIGNAL(activated(int)),this,SIGNAL(modified()));
    connect(ui->comBoxType,SIGNAL(activated(int)),this,SIGNAL(modified()));

    connect(ui->pButColor,SIGNAL(clicked()),SIGNAL(modified()));
    connect(ui->pButTxtColor,SIGNAL(clicked()),SIGNAL(modified()));
    connect(ui->pButShColor,SIGNAL(clicked()),SIGNAL(modified()));
    connect(ui->pButFont,SIGNAL(clicked()),SIGNAL(modified()));

    connect(ui->rButMap,SIGNAL(clicked()),SLOT(bgrMap()));
    connect(ui->rButSat,SIGNAL(clicked()),SLOT(bgrSat()));

    connect(ui->pButColor,SIGNAL(clicked()),this,SLOT(modifyColor()));
    connect(ui->pButTxtColor,SIGNAL(clicked()),this,SLOT(modifyTxtColor()));
    connect(ui->pButShColor,SIGNAL(clicked()),this,SLOT(modifyShadColor()));
    connect(ui->pButFont,SIGNAL(clicked()),this,SLOT(modifyFont()));

    connect(ui->rButMap, SIGNAL(clicked()),this,SLOT(onBackgroundSelected()));
    connect(ui->rButSat, SIGNAL(clicked()),this,SLOT(onBackgroundSelected()));

    ui->rButMap->click();

//    init();
    reset();
}

TrackOptions::~TrackOptions()
{
    delete ui;
}

void TrackOptions::init()
{
}

void TrackOptions::modifyColor()
{
    QColor color = QColorDialog::getColor(FLineColor,this,tr("Select line color"));
    if(color.isValid())
    {
//        LINE draw
        FLineColor = color;
//        emit modified();
    }
}

void TrackOptions::modifyTxtColor()
{
    QColor color = QColorDialog::getColor(FCurrentTextColor,this,tr("Select text color"));
    if(color.isValid())
    {
        FCurrentTextColor = color;
        QPalette pal(ui->lblColor->palette());
        pal.setColor(QPalette::Text, color);
        ui->lblColor->setPalette(pal);


//        ui->lblColor->setPalette(FCurrentTextColor);
        emit modified();
    }
}

void TrackOptions::modifyShadColor()
{
    QColor color = QColorDialog::getColor(FCurrentShadowColor,this,tr("Select Shadow color"));
    if(color.isValid())
    {
        FCurrentShadowColor = color;
        ui->lblShad->setPalette(FCurrentShadowColor);
//lable->setText("<FONT COLOR=#008000>Some coloured text</FONT>");
//myLabel.setStyleSheet("QLabel { background-color : %s"%color.name())


/*
QPalette palette = ui->pLabel->palette();
 palette.setColor(ui->pLabel->backgroundRole(), Qt::yellow);
 palette.setColor(ui->pLabel->foregroundRole(), Qt::yellow);
 ui->pLabel->setPalette(palette);

 QPalette sample_palette;
sample_palette.setColor(QPalette::Window, Qt::white);
sample_palette.setColor(QPalette::WindowText, Qt::blue);

sample_label->setAutoFillBackground(true);
sample_label->setPalette(sample_palette);
sample_label->setText("What ever text");


QPalette pal(label->palette());
pal.setColor(QPalette::Text, color);
label->setPalette(pal);

*/






//        emit modified();
    }
}

void TrackOptions::modifyFont()
{
qDebug() << "TrackOptions::modifyFont()/font-in" << ui->lblShad->font();
    bool ok;
    QFont font = QFontDialog::getFont(&ok, FCurrentFont, this);
    if(ok)
    {
        FCurrentFont = font;
qDebug() << "TrackOptions::modifyFont()/font" << font;

        ui->lblShad->setFont(font);
        ui->lblColor->setFont(font);
//        emit modified();
qDebug() << "TrackOptions::modifyFont()/font-set" << ui->lblShad->font();
    }
}

void TrackOptions::bgrMap()
{
    QString fmap=IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->fileFullName("bgrmap");
    QString word=QString("background-image: url(%1);").arg(fmap);
    ui->frame->setStyleSheet(word);
}

void TrackOptions::bgrSat()
{
    QString fsat=IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->fileFullName("bgrsat");
    QString word=QString("background-image: url(%1);").arg(fsat);
    ui->frame->setStyleSheet(word);
}

void TrackOptions::onBackgroundSelected()
{
    ui->frame->setStyleSheet(
                QString("QFrame { background-image: url(%1) }; QLabel { background-image: none }")
                .arg(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)
                     ->fileFullName(sender()==ui->rButMap?"bgrmap":"bgrsat")));
}

void TrackOptions::apply()
{
    ui->lblShad->setFont(FCurrentFont);
    ui->lblColor->setFont(FCurrentFont);
    ui->lblShad->setPalette(FCurrentShadowColor);
    ui->lblColor->setPalette(FCurrentTextColor);

qDebug() << "TrackOptions::apply()";

    Options::node(OPV_TRC_LINECOLOR).setValue(FLineColor);
    Options::node(OPV_TRC_TEXTCOLOR).setValue(FCurrentTextColor);
    Options::node(OPV_TRC_SHADOWCOLOR).setValue(FCurrentShadowColor);
    Options::node(OPV_TRC_FONT).setValue(FCurrentFont);
    emit childApply();
}

void TrackOptions::reset()
{
    FLineColor = Options::node(OPV_TRC_LINECOLOR).value().toString();
    FCurrentTextColor = Options::node(OPV_TRC_TEXTCOLOR).value().toString();
    FCurrentShadowColor = Options::node(OPV_TRC_SHADOWCOLOR).value().toString();
    FCurrentFont = Options::node(OPV_TRC_FONT).value().value<QFont>();

    ui->lblColor->setFont(FCurrentFont);
    ui->lblShad->setFont(FCurrentFont);
    ui->lblShad->setPalette(FCurrentShadowColor);
    ui->lblColor->setPalette(FCurrentTextColor);

    emit childReset();
}

void TrackOptions::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

/*

void PoiOptions::apply()
{

    emit childApply();
}

void PoiOptions::reset()
{
   updateTypeWidget();


    QPalette palette;
    palette.setColor(QPalette::WindowText, FCurrentTextColor);
    ui->txtLabel->setPalette(palette); //"color:cyan;"
    palette.setColor(QPalette::WindowText, FCurrentTempTextColor);
    ui->txtTemporary->setPalette(palette); //"color:cyan;"
    palette.setColor(QPalette::WindowText, FCurrentShadowColor);
    ui->txtLabelShadow->setPalette(palette); //"color:red;"
    ui->txtTemporaryShadow->setPalette(palette); //"color:red;"

    emit childReset();
}

*/

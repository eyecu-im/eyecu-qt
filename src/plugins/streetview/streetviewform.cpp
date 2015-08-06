#include <QGraphicsDropShadowEffect>
#include <QPicture>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QList>
#include <QUrl>
#include <QBuffer>
#include <MercatorCoordinates>

#include <utils/iconstorage.h>
#include <utils/options.h>
#include <utils/shortcuts.h>
#include <utils/action.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/resources.h>
#include "definitions/menuicons.h"

#include "streetviewform.h"
#include "ui_streetviewform.h"

#define INC_VERT        10
#define INC_HOR         10

StreetViewForm::StreetViewForm(QHash<QUuid, IStreetViewProvider *> AProviders, IMap *AMap, StreetView *AStreetView, /*IConnectionManager *AConnectionManager,*/QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StreetViewForm),
    FNetworkAccessManager(new QNetworkAccessManager(this)),
    FHttpRequester(FNetworkAccessManager),
    FMap(AMap),
    FStreetView(AStreetView),
    FProviders(AProviders),
    FCurrentID(0)
{
    ui->setupUi(this);    
    setWindowIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_STREETVIEW));
    QStyle *style = QApplication::style();
	ui->pbMoveToLocation->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_GEOLOC));
    ui->tbView->setStyleSheet("text-align: left");
    ui->pbSaveToDisk->setIcon(style->standardIcon(QStyle::SP_DialogSaveButton));
    ui->pbSaveToDisk->setStyleSheet("text-align: left");

    ui->dselDirection->setSpacing(2,0);

    connect(Options::instance(), SIGNAL(optionsOpened()), SLOT(onOptionsOpened()));
    connect(Options::instance(), SIGNAL(optionsChanged(const OptionsNode &)), SLOT(onOptionsChanged(const OptionsNode &)));

    init();
}

StreetViewForm::~StreetViewForm()
{
    delete ui;
}

void StreetViewForm::init()
{
    FProvidersGroup=new QActionGroup(ui->tbView);
    for (QHash<QUuid, IStreetViewProvider *>::const_iterator it=FProviders.constBegin(); it!=FProviders.constEnd(); it++)
    {
        QAction *action=FProvidersGroup->addAction((*it)->sourceIcon(), (*it)->sourceName());
        action->setData(it.key().toString());
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(onProviderSelected()));
        ui->tbView->addAction(action);
        (*it)->setHttpRequester(&FHttpRequester);
    }
}

void StreetViewForm::selectProvider(QUuid AProviderUuid)
{
    QUuid newProviderUuid=FProviders.contains(AProviderUuid)?AProviderUuid:FProviders.keys()[0];
    if (newProviderUuid!=FCurrentProvider)
    {
        FCurrentProvider=newProviderUuid;
        if (!FCurrentProvider.isNull()){
            connect(FProviders[FCurrentProvider]->instance(),SIGNAL(imageReceived(QByteArray,QUrl)),SLOT(onImageReceived(QByteArray,QUrl)));
        }
        QList<QAction *> actions=FProvidersGroup->actions();
        for (QList<QAction *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
            if ((*it)->data()==AProviderUuid.toString())
            {
                (*it)->setChecked(true);
                ui->tbView->setIcon((*it)->icon());
                break;
            }
    }
}

void StreetViewForm::hideEvent(QHideEvent *AHideEvent)
{
	Q_UNUSED(AHideEvent)

    FStreetView->hideStreetViewMarker();
}

void StreetViewForm::onProviderSelected()
{
    Options::node(OPV_MAP_STREETVIEW_PROVIDER).setValue(qobject_cast<QAction *>(sender())->data().toString());
}

void StreetViewForm::onViewClicked()
{
    FStreetView->startStreetView(ui->spbLatitude->value(), ui->spbLongitude->value());
}

void StreetViewForm::startView()
{
    ui->pbSaveToDisk->setDisabled(true);
    ui->lblView->setText(QString("<font size='24' color='blue'>%1</font>").arg(tr("Please wait....")));
    QStringList indexs=ui->cbSize->currentText().split("x");
    int azimuth=ui->dselDirection->direction();
    if (azimuth<0)
        azimuth+=360;
    FProviders[FCurrentProvider]->getStreetView(QSize(indexs[0].toInt(), indexs[1].toInt()),
                                                ui->spbLatitude->value(),
                                                ui->spbLongitude->value(),
                                                azimuth,
                                                ui->hslFov->value(),
                                                ui->eselElevation->elevation());
}
/*
void StreetViewForm::onSettings()
{
    //FOptionsManager->showOptionsDialog(OPN_MAPSTREETVIEW);
}
*/
void StreetViewForm::onMoveToPosition()
{
    FMap->showObject(MOT_STREETVIEW, "StreetView", true);
}

void StreetViewForm::onImageReceived(const QByteArray &AResult, const QUrl &AUrl)
{    
    FLastProvider = FCurrentProvider;    
    if(AResult.size())
    {
        QPixmap pixmap;
        pixmap.loadFromData(AResult);
        FImageData = AResult;
        ui->lblView->setPixmap(pixmap);
        FUrl = AUrl;
    }
    else
    {
        FImageData.clear();
        FUrl.clear();
        ui->lblView->setText(QString("<font size='24' color='red'>%1</font>").arg(tr("HTTP error")));
    }
    ui->pbSaveToDisk->setDisabled(false);
    //! For resize Form ---
    QStringList indexs=ui->cbSize->currentText().split("x");
    int x = 3+indexs[0].toInt()+3;//MAX(indexs[0].toInt(),ui->groupBox->size().width())
    int y = 3+indexs[1].toInt()+2+ui->groupBox->size().height()+3;
    resize(x,y);
}

void StreetViewForm::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_MAP_STREETVIEW_PROVIDER));
    onOptionsChanged(Options::node(OPV_MAP_STREETVIEW_FOV));
    onOptionsChanged(Options::node(OPV_MAP_STREETVIEW_SIZE));
}
void StreetViewForm::onOptionsChanged(const OptionsNode &ANode)
{
    if  (ANode.path()==OPV_MAP_STREETVIEW_PROVIDER)
        selectProvider(ANode.value().toString());
    else if  (ANode.path()==OPV_MAP_STREETVIEW_FOV)
    {
        ui->hslFov->setValue(ANode.value().toInt());
        ui->lcdFov->display(ANode.value().toInt());
    }
    else if(ANode.path()==OPV_MAP_STREETVIEW_SIZE)
        ui->cbSize->setCurrentIndex(ANode.value().toInt());
}

void StreetViewForm::setLocation(double ALatitude, double ALongitude)
{
    ui->spbLatitude->setValue(ALatitude);
    ui->spbLongitude->setValue(ALongitude);
}

void StreetViewForm::setLocation(const MercatorCoordinates &ALocation)
{
    setLocation(ALocation.latitude(), ALocation.longitude());
}

void StreetViewForm::setAzimuth(int AAzimuth)
{
    ui->spbAzimuth->setValue(AAzimuth);
}

void StreetViewForm::setElevation(int AElevation)
{
    ui->spbElevation->setValue(AElevation);
}

void StreetViewForm::onFovValueChange(int AValue)
{
    Options::node(OPV_MAP_STREETVIEW_FOV).setValue(AValue);
    startView();
}

void StreetViewForm::onSizeSelected(int ASize)
{
    Options::node(OPV_MAP_STREETVIEW_SIZE).setValue(ASize);
    startView();
}

void StreetViewForm::onDirectionChange(qreal ADirection)
{
    ui->spbAzimuth->setValue(ADirection);
    startView();
}

void StreetViewForm::onDirectionTrack(qreal ADirection)
{
    ui->spbAzimuth->setValue(ADirection);
}

void StreetViewForm::onAzimuthEditFinished()
{
    ui->dselDirection->setDirection(ui->spbAzimuth->value());
}

void StreetViewForm::onElevationChange(qreal AElevation)
{
    ui->spbElevation->setValue(AElevation);
    startView();
}

void StreetViewForm::onElevationTrack(qreal AElevation)
{
    ui->spbElevation->setValue(AElevation);
}

void StreetViewForm::onElevationEditFinished()
{
    ui->eselElevation->setElevation(ui->spbElevation->value());
}

void StreetViewForm::onCopyClicked()
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;
    urls.append(FUrl);
    mimeData->setUrls(urls);
    mimeData->setImageData(ui->lblView->pixmap()->toImage());    
	mimeData->setHtml(QString("<img src=\"%1\" />").arg(QString::fromLatin1(FUrl.toEncoded())));
    QApplication::clipboard()->setMimeData(mimeData);
}

void StreetViewForm::onSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
								tr("Save file"), QDir(Options::node(OPV_MAP_STREETVIEW_IMAGEDIRECTORY).value().toString()).absoluteFilePath("StreetViewImage.jpg"), tr("JPEG fles (*.jpg *.jpeg)"));
    if (!fileName.isEmpty())
    {
		Options::node(OPV_MAP_STREETVIEW_IMAGEDIRECTORY).setValue(QFileInfo(fileName).absolutePath());
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
            return;
        file.write(FImageData);
        file.close();
    }
}

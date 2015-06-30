#include <QMessageBox>
#include <QGraphicsView>
#include <MercatorCoordinates>
#include <utils/iconstorage.h>
#include <utils/options.h>
#include <utils/shortcuts.h>
#include <utils/action.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/shortcuts.h>
#include <definitions/shortcutgrouporders.h>
#include "mapsearchdialog.h"
#include "ui_mapsearchdialog.h"

MapSearchDialog::MapSearchDialog(QHash<QUuid, IMapSearchProvider *> AProviders, IMap *AMap, IPoi *APoi, IOptionsManager *AOptionsManager, IConnectionManager *AConnectionManager, QWidget *AParent) :
    QDialog(AParent),
    ui(new Ui::MapSearchDialog),
    FNetworkAccessManager(new QNetworkAccessManager(this)),
    FHttpRequester(FNetworkAccessManager),
    FMap(AMap),
    FPoi(APoi),
    FOptionsManager(AOptionsManager),
    FConnectionManager(AConnectionManager),
    FProviders(AProviders),
    FCurrentID(0),
    FLimitMin(0),
    FLimitMax(0),
    FLimitDefault(0),
    FMoreResultsAvailable(false)
{
    ui->setupUi(this);
    ui->pbClear->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_EDIT_DELETE));
    ui->pbSettings->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_OPTIONS_DIALOG));

#if QT_VERSION >=0x040700
    ui->ledSearch->setPlaceholderText(tr("Enter search string"));
#endif

    //Shortcuts
    FPoi->setTreeWidgetShortcuts(ui->twFound, true);
    Shortcuts::bindObjectShortcut(SCT_MAPSEARCH_CLEARLIST, ui->pbClear);
    Shortcuts::bindObjectShortcut(SCT_MAPSEARCH_SHOW, ui->chkShow);
    Shortcuts::bindObjectShortcut(SCT_MAPSEARCH_LIMITRANGE, ui->chkLimitSearchRange);
    Shortcuts::insertWidgetShortcut(SCT_MAPSEARCH_SELECTPROVIDER, ui->tbSearch);
    connect(Shortcuts::instance(), SIGNAL(shortcutActivated(QString,QWidget*)), SLOT(onShortcutActivated(QString,QWidget*)));

    FProvidersGroup=new QActionGroup(ui->tbSearch);
    for (QHash<QUuid, IMapSearchProvider *>::const_iterator it=AProviders.constBegin(); it!=AProviders.constEnd(); it++)
    {
        QAction *action=FProvidersGroup->addAction((*it)->sourceIcon(), (*it)->sourceName());
        action->setData(it.key().toString());
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(onProviderSelected()));        
        ui->tbSearch->addAction(action);
        (*it)->setHttpRequester(&FHttpRequester);
    }    

	onOptionsOpened();	// Initialize dialog

    connect(FPoi->instance(), SIGNAL(poiModified(QString,int)), SLOT(onPoiModified(QString,int)));
    if (FMap)
    {
		connect(FMap->geoMap()->getScene()->instance(), SIGNAL(mapCenterChanged(double,double,bool)), SLOT(onMapSettingsChanged()));
		connect(FMap->geoMap()->getScene()->instance(), SIGNAL(mppChanged(double)), SLOT(onMapSettingsChanged()));
    }

    connect(Options::instance(), SIGNAL(optionsOpened()), SLOT(onOptionsOpened()));
    connect(Options::instance(), SIGNAL(optionsChanged(const OptionsNode &)), SLOT(onOptionsChanged(const OptionsNode &)));	
}

MapSearchDialog::~MapSearchDialog()
{
    delete ui;
}

void MapSearchDialog::selectProvider(QUuid AProviderUuid)
{
    QUuid newProviderUuid=FProviders.contains(AProviderUuid)?AProviderUuid:FProviders.keys()[0];
    if (newProviderUuid!=FCurrentProvider)
    {
        if (!FCurrentProvider.isNull())
            disconnect(FProviders[FCurrentProvider]->instance(), SIGNAL(receivedPoi(GeolocElement)), this, SLOT(onReceivedPoi(GeolocElement)));
        FCurrentProvider=newProviderUuid;
        if (!FCurrentProvider.isNull())
        {
            connect(FProviders[FCurrentProvider]->instance(), SIGNAL(receivedPoi(GeolocElement)), SLOT(onReceivedPoi(GeolocElement)));
            connect(FProviders[FCurrentProvider]->instance(), SIGNAL(searchFinished(bool)), SLOT(onSearchFinished(bool)));
            FProviders[FCurrentProvider]->getPageValues(&FLimitMin, &FLimitMax, &FLimitDefault);
            updateControls(FProviders[FCurrentProvider]);
            onMapSettingsChanged();
        }

        QList<QAction *> actions=FProvidersGroup->actions();
        for (QList<QAction *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
            if ((*it)->data()==AProviderUuid.toString())
            {
                (*it)->setChecked(true);
                ui->tbSearch->setIcon((*it)->icon());
                break;
            }
    }
}

QString MapSearchDialog::selectedId() const
{
    QList<QTreeWidgetItem *>selection = ui->twFound->selectedItems();
    return selection.size()==1?selection[0]->data(0, IPoi::PDR_ID).toString():QString();
}

void MapSearchDialog::showPois(bool AShow)
{
    ui->chkShow->setChecked(AShow);
    int count=ui->twFound->topLevelItemCount();
    for (int i=0; i<count; i++)
        if (AShow)
            FPoi->showSinglePoi(ui->twFound->topLevelItem(i)->data(0, IPoi::PDR_ID).toString());
        else
            FPoi->hideOnePoi(ui->twFound->topLevelItem(i)->data(0, IPoi::PDR_ID).toString());
}

void MapSearchDialog::setPoiColor(const QColor &AColor)
{
    int count=ui->twFound->topLevelItemCount();
    for (int i=0; i<count; i++)
    {
        QString id=ui->twFound->topLevelItem(i)->data(0, IPoi::PDR_ID).toString();
        GeolocElement poi=FPoi->getPoi(id);
		poi.setProperty("color", AColor.name());
        FPoi->putPoi(id, poi, false);
    }
}

void MapSearchDialog::updateControls(const IMapSearchProvider *ANewProvider)
{    
    if (ANewProvider)
    {
        int features = ANewProvider->features();
        ui->chkLimitSearchRange->setVisible(features&IMapSearchProvider::FeatureLimitRange);
        if (features&IMapSearchProvider::FeatureMaxResults)
        {
            ui->lblResultsPerPage->setVisible(true);
            ui->spbResultsPerPage->setVisible(true);
            ui->chkDefault->setVisible(true);
            ui->spbResultsPerPage->setMinimum(FLimitMin);
            ui->spbResultsPerPage->setMaximum(FLimitMax);
            if (Options::node(OPV_MAP_SEARCH_PAGESIZE_DEFAULT).value().toBool())
            {
                ui->chkDefault->setChecked(true);
                ui->spbResultsPerPage->setValue(FLimitDefault);
            }
            else
            {
                ui->chkDefault->setChecked(false);
                ui->spbResultsPerPage->setValue(Options::node(OPV_MAP_SEARCH_PAGESIZE).value().toInt());
            }
        }
        else
        {
            ui->lblResultsPerPage->setVisible(false);
            ui->spbResultsPerPage->setVisible(false);
            ui->chkDefault->setVisible(false);
        }
        if (features&IMapSearchProvider::FeatureNextPage)
        {
            ui->tbMore->setVisible(true);
            ui->tbMore->setDisabled(true);
        }
        else
            ui->tbMore->setVisible(false);
        if (features&IMapSearchProvider::FeatureLimitRangeAlways)
        {
            ui->chkLimitSearchRange->setChecked(true);
            ui->chkLimitSearchRange->setDisabled(true);
        }
        else
        {
            ui->chkLimitSearchRange->setChecked(Options::node(OPV_MAP_SEARCH_LIMITRANGE).value().toBool());
            ui->chkLimitSearchRange->setDisabled(false);
        }

    }
    else
        ui->chkLimitSearchRange->setVisible(false);
}

int MapSearchDialog::exec(bool AAutoShow)
{
    FAutoShow=AAutoShow;
    return QDialog::exec();
}

void MapSearchDialog::show(bool AAutoShow)
{
    FAutoShow=AAutoShow;
    QDialog::show();
}

void MapSearchDialog::onShowPois(bool AShow)
{
    Options::node(OPV_MAP_SEARCH_SHOW).setValue(AShow);
}

void MapSearchDialog::onLimitSearchRange(bool AShow)
{
    Options::node(OPV_MAP_SEARCH_LIMITRANGE).setValue(AShow);
}

void MapSearchDialog::onProviderSelected()
{
    Options::node(OPV_MAP_SEARCH_PROVIDER).setValue(qobject_cast<QAction *>(sender())->data().toString());
}

void MapSearchDialog::calculateSearchRange(double &AEast, double &ASouth, double &AWest, double &ANorth)
{
    if (FMap)
    {
		MapScene *scene = FMap->geoMap()->getScene();
        QSize size = scene->instance()->views().first()->size()/2;
		MercatorCoordinates southWest = FMap->geoMap()->getScene()->calculateLocation(-size.width(), size.height());
		MercatorCoordinates northEast = FMap->geoMap()->getScene()->calculateLocation(size.width(), -size.height());
        ASouth = southWest.latitude();
        AWest  = southWest.longitude();
        ANorth = northEast.latitude();
        AEast  = northEast.longitude();
    }
    else
    {
        ASouth = -90;
        ANorth =  90;
        AEast  = -180;
        AWest  =  180;
    }
}

void MapSearchDialog::onSearchClicked()
{
    QString searchString=ui->ledSearch->text();
    if (!searchString.isEmpty())
    {
        qreal east, north, south, west;
        calculateSearchRange(east, south, west, north);
        FPoiFound = false;
		FProviders[FCurrentProvider]->startSearch(searchString, south, west, north, east, FMap?FMap->geoMap()->getScene()->zoom():0, ui->chkLimitSearchRange->isChecked(), ui->chkDefault->isChecked()?-1:ui->spbResultsPerPage->value(), sender()==ui->tbMore);
    }
}

void MapSearchDialog::onReceivedPoi(const GeolocElement &APoi)
{
    FPoiFound=true;
    GeolocElement poi=APoi;
	poi.setProperty("color", Options::node(OPV_MAP_SEARCH_LABEL_COLOR).value().value<QColor>().name());
    FPoi->putPoi(QString("found/%1").arg(++FCurrentID), poi, Options::node(OPV_MAP_SEARCH_SHOW).value().toBool());
    ui->twFound->setFocus();
}

void MapSearchDialog::onSearchFinished(bool AMoreResultsAvailable)
{
    if (!FPoiFound)
        (new QMessageBox(QMessageBox::Information, tr("Map Search"), tr("Found nothing!"), QMessageBox::Ok))
            ->exec();
    FMoreResultsAvailable = AMoreResultsAvailable;
    FLastResultsPerPage   = ui->spbResultsPerPage->value();
    FLastLimitRange       = ui->chkLimitSearchRange->isChecked();
    FLastSearchString     = ui->ledSearch->text();
    FLastProvider         = FCurrentProvider;
    calculateSearchRange(FLastEast, FLastSouth, FLastWest, FLastNorth);
    onMapSettingsChanged();
}

void MapSearchDialog::onSettings()
{
    FOptionsManager->showOptionsDialog(OPN_MAPSEARCH);
}

void MapSearchDialog::onDefaultToggled(bool AState)
{
    Options::node(OPV_MAP_SEARCH_PAGESIZE_DEFAULT).setValue(AState);
}

void MapSearchDialog::onResultsPerPageChanged(int ANewValue)
{
    if (!ui->chkDefault->isChecked())
        Options::node(OPV_MAP_SEARCH_PAGESIZE).setValue(ANewValue);
}

void MapSearchDialog::onMapSettingsChanged()
{
    double east, south, west, north;
    calculateSearchRange(east, south, west, north);
    ui->tbMore->setEnabled(FMoreResultsAvailable &&
                           FLastResultsPerPage == ui->spbResultsPerPage->value() &&
                           FLastLimitRange     == ui->chkLimitSearchRange->isChecked() &&
                           FLastSearchString   == ui->ledSearch->text() &&
                           FLastProvider       == FCurrentProvider &&
                           FLastEast == east && FLastSouth == south && FLastWest == west && FLastNorth == north);
}

void MapSearchDialog::onPoiModified(const QString &AId, int AType)
{
    if (AId.startsWith("found/"))
    {
        switch (AType)
        {
            case IPoi::PMT_ADDED:
            {
                GeolocElement poi=FPoi->getPoi(AId);
				QIcon icon=FPoi->getTypeIcon(poi.type());
                if (icon.isNull())
                    icon=FPoi->getIcon(MNI_POI_NONE);
                QTreeWidgetItem *item=new QTreeWidgetItem(ui->twFound);
                item->setIcon(0, icon);
				item->setText(1, poi.text());
				item->setText(2, poi.description());
                item->setData(0, IPoi::PDR_ID, AId);
                ui->twFound->setCurrentItem(item);
                break;
            }

            case IPoi::PMT_REMOVED:
            {
                int count=ui->twFound->topLevelItemCount();
                for (int i=0; i<count; i++)
                    if (ui->twFound->topLevelItem(i)->data(0, IPoi::PDR_ID).toString()==AId)
                    {
                        delete ui->twFound->takeTopLevelItem(i);
                        break;
                    }
                break;
            }
        }
    }
}

void MapSearchDialog::onCustomContextMenuRequested(const QPoint &APos)
{
    QTreeWidgetItem *item=ui->twFound->itemAt(APos);
    if (item)
    {
        Menu *menu=new Menu(ui->twFound);
        FPoi->contextMenu(item->data(0, IPoi::PDR_ID).toString(), menu);
        menu->exec(ui->twFound->viewport()->mapToGlobal(APos));
        menu->deleteLater();
    }
}

void MapSearchDialog::onItemActivated(QTreeWidgetItem *ASelectedIitem, int AIndex)
{
    Q_UNUSED(ASelectedIitem);
    Q_UNUSED(AIndex);    
    if (FAutoShow)
    {
        FPoi->poiShow(ASelectedIitem->data(0, IPoi::PDR_ID).toString());
        reject();
    }
    else
        accept();
}

void MapSearchDialog::onShortcutActivated(const QString &AId, QWidget *AWidget)
{
	Q_UNUSED(AWidget)

    if (AId==SCT_MAPSEARCH_SELECTPROVIDER)
        ui->tbSearch->showMenu();
}

void MapSearchDialog::onClearListClicked()
{
    while (ui->twFound->topLevelItemCount())
        FPoi->removePoi(ui->twFound->topLevelItem(0)->data(0, IPoi::PDR_ID).toString());
}

void MapSearchDialog::onOptionsOpened()
{
	onOptionsChanged(Options::node(OPV_MAP_SEARCH_SHOW));
	if (FConnectionManager)
		onOptionsChanged(Options::node(OPV_MAP_SEARCH_PROXY));
	onOptionsChanged(Options::node(OPV_MAP_SEARCH_PROVIDER));
	onOptionsChanged(Options::node(OPV_MAP_SEARCH_PAGESIZE_DEFAULT));
}

void MapSearchDialog::onOptionsChanged(const OptionsNode &ANode)
{
    if (ANode.path()==OPV_MAP_SEARCH_SHOW)
        showPois(ANode.value().toBool());
    else if (ANode.path()==OPV_MAP_SEARCH_PROVIDER)
        selectProvider(ANode.value().toString());
    else if(ANode.path()==OPV_MAP_SEARCH_PROXY) // Proxy
        FNetworkAccessManager->setProxy(FConnectionManager->proxyById(ANode.value().toString()).proxy);
    else if (ANode.path()==OPV_MAP_SEARCH_LABEL_COLOR)
        setPoiColor(ANode.value().value<QColor>());
    else if (ANode.path()==OPV_MAP_SEARCH_PAGESIZE_DEFAULT)
    {
        ui->chkDefault->setChecked(ANode.value().toBool());
        ui->spbResultsPerPage->setDisabled(ANode.value().toBool());
        if (ANode.value().toBool())
            ui->spbResultsPerPage->setValue(FLimitDefault);
        else
            ui->spbResultsPerPage->setValue(Options::node(OPV_MAP_SEARCH_PAGESIZE).value().toInt());
    }
    else if (ANode.path()==OPV_MAP_SEARCH_LIMITRANGE)
        ui->chkLimitSearchRange->setChecked(ANode.value().toBool());
}

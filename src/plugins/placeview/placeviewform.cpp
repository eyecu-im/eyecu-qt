#include <QGraphicsDropShadowEffect>
#include <QPicture>
#include <QFileDialog>
#include <QTextDocument>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#include <QBuffer>
#include <MercatorCoordinates>

#include <utils/options.h>
#include <utils/shortcuts.h>
#include <utils/action.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/resources.h>
#include "definitions/menuicons.h"

#include "placeviewform.h"
#include "ui_placeviewform.h"

#define DELAY           3000        //! mls
#define PICT_SIZE_MAX   480

PlaceViewForm::PlaceViewForm(QHash<QUuid, IPlaceViewProvider *> AProviders, IMap *AMap,PlaceView *APlaceView, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaceViewForm),
      FNetworkAccessManager(new QNetworkAccessManager(this)),
      FHttpRequester(FNetworkAccessManager),
      FMap(AMap),
      FPlaceView(APlaceView),
      FProviders(AProviders),
      FCurrentID(0),
      FNewQuery(false),
      FNextPage("")
{
    ui->setupUi(this);
    setWindowTitle(tr("Place Viewer From eyeCU"));

    QStyle *style = QApplication::style();
    ui->tbGetView->setStyleSheet("text-align: left");

    ui->pbSaveToDisk->setIcon(style->standardIcon(QStyle::SP_DialogSaveButton));
    ui->pbSaveToDisk->setStyleSheet("text-align: left");
    ui->pbSaveToDisk->setDisabled(true);
	ui->pbSaveToDisk->setVisible(false);

    ui->pbClear->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_EDIT_DELETE));
    ui->pbClear->setStyleSheet("text-align: left");
    ui->pbClear->setDisabled(true);

    ui->btnMoveToLocation->setIcon(IconStorage::staticStorage(RSR_STORAGE_GEOLOC)->getIcon(MNI_GEOLOC));

    connect(ui->tbGetView, SIGNAL(clicked()),SLOT(onGetView()));

//    connect(ui->pbSaveToDisk, SIGNAL(clicked()),SLOT(onSaveToDisk()));
    connect(ui->pbClear, SIGNAL(clicked()),SLOT(onClearToMap()));
    connect(ui->btnMoveToLocation, SIGNAL(clicked()),SLOT(onMoveToPosition()));

    connect(ui->cBoxType, SIGNAL(currentIndexChanged(int)),SLOT(onCurrentIndexChanged(int)));

    connect(ui->cBoxRadius, SIGNAL(currentIndexChanged(int)),SLOT(onCurrentRadiusChanged(int)));
    connect(ui->cBoxRankby, SIGNAL(currentIndexChanged(int)),SLOT(onBoxRankby(int)));
    connect(ui->cBoxWay, SIGNAL(currentIndexChanged(int)),SLOT(onBoxWay(int)));

    connect(Options::instance(), SIGNAL(optionsOpened()), SLOT(onOptionsOpened()));
    connect(Options::instance(), SIGNAL(optionsChanged(const OptionsNode &)), SLOT(onOptionsChanged(const OptionsNode &)));

    setTypeGooglePoi();
    init();
}

PlaceViewForm::~PlaceViewForm()
{
    delete ui;
}


void PlaceViewForm::onGetView()
{
    //!----Waiting...
    QString text=QString("<font color='blue'>%1</font>").arg(tr("Please wait...."));
    blocking(true,text);
    emit poisDelete();
    //!------------
    if (!FNewQuery)
        FNumberHttpReuest=0;

    //QString ATypes  =   FListTypes[ui->cBoxType->currentIndex()];
    QString ATypes  =   FHashPoiTypes.key(ui->cBoxType->currentText());

    QString ARankby =   FListRankby[ui->cBoxRankby->currentIndex()];
    QString AWayToSearch=FListWayToSearch[ui->cBoxWay->currentIndex()];
    QString AKeyword= ui->lineKeyword->text();
    QString AId="request";
    QString ALanguage=QLocale().name().left(2);
    long    ARadius;
    if(ARankby=="distance")
        ARadius=0;
    else
        ARadius=ui->cBoxRadius->currentText().toFloat()*1000;
    FProviders[FCurrentProvider]->getAboutPlace(FLat,FLng,ARadius,ATypes,ARankby,AKeyword,AWayToSearch,FNextPage,ALanguage,AId);
}

void PlaceViewForm::hideEvent(QHideEvent *AHideEvent)
{
	Q_UNUSED(AHideEvent)

    FPlaceView->closeForm();
}

void PlaceViewForm::onMoveToPosition()
{
    FMap->showObject(MOT_PLACEVIEW, MNI_MAP_PLACEVIEW, true);
}
//!- only data writing to form --
void PlaceViewForm::formDialogActivate(double ALat, double ALng)
{
    FLat=ALat;
    FLng=ALng;
    QString text=QString("<font color='green'>Lat,Lng=%1,%2</font>").arg(FLat).arg(FLng);
    blocking(false,text);
}

void PlaceViewForm::init()
{
    FProvidersGroup=new QActionGroup(ui->tbGetView);
    for (QHash<QUuid, IPlaceViewProvider *>::const_iterator it=FProviders.constBegin(); it!=FProviders.constEnd(); it++)
    {
        QAction *action=FProvidersGroup->addAction((*it)->sourceIcon(), (*it)->sourceName());
        action->setData(it.key().toString());
        action->setCheckable(true);
        connect(action, SIGNAL(triggered()), SLOT(onProviderSelected()));
        ui->tbGetView->addAction(action);
        (*it)->setHttpRequester(&FHttpRequester);
    }
}

void PlaceViewForm::selectProvider(QUuid AProviderUuid)
{
    QUuid newProviderUuid=FProviders.contains(AProviderUuid)?AProviderUuid:FProviders.keys()[0];
    if (newProviderUuid!=FCurrentProvider)
    {
        FCurrentProvider=newProviderUuid;
        if (!FCurrentProvider.isNull()){
            connect(FProviders[FCurrentProvider]->instance(),SIGNAL(aboutPlaceReceived(const QByteArray&,const QString &,bool)),SLOT(onAboutReceived(const QByteArray&,const QString &,bool)));
            connect(FProviders[FCurrentProvider]->instance(),SIGNAL(photoPlaceReceived(const QByteArray&,const QString&,bool)),SLOT(onPhotoPlaceReceived(const QByteArray&,const QString&,bool)));
            connect(FProviders[FCurrentProvider]->instance(),SIGNAL(imageReceived(const QByteArray&,const QString&,bool)),SLOT(onImageReceived(const QByteArray&,const QString&,bool)));
        }
        QList<QAction *> actions=FProvidersGroup->actions();
        for (QList<QAction *>::const_iterator it=actions.constBegin(); it!=actions.constEnd(); it++)
            if ((*it)->data()==AProviderUuid.toString())
            {
                (*it)->setChecked(true);
                ui->tbGetView->setIcon((*it)->icon());
                break;
            }
    }
}

void PlaceViewForm::onProviderSelected()
{
    Options::node(OPV_MAP_PLACEVIEW_PROVIDER).setValue(qobject_cast<QAction *>(sender())->data().toString());
}

void PlaceViewForm::onCurrentIndexChanged(int AType)
{
    Options::node(OPV_MAP_PLACEVIEW_TYPE).setValue(AType);
//    Options::node(OPV_MAP_PLACEVIEW_TYPE).setValue(FHashPoiTypes.key(ui->cBoxType->currentText()));
}
void PlaceViewForm::onCurrentRadiusChanged(int ARadius)
{
    Options::node(OPV_MAP_PLACEVIEW_RADIUS).setValue(ARadius);
}

void PlaceViewForm::onBoxRankby(int APos)
{
    Options::node(OPV_MAP_PLACEVIEW_RANKBY).setValue(APos);
    ui->cBoxRadius->setDisabled(APos);
}

void PlaceViewForm::onBoxWay(int APos)
{
    Options::node(OPV_MAP_PLACEVIEW_WAY).setValue(APos);
}

void PlaceViewForm::onAboutReceived(const QByteArray &AResult, const QString &AId, bool AState)
{
    Q_UNUSED(AState)
    if (AResult.isNull()){
        QString text=QString("<font color='red'>%1</font>").arg(tr("Error HTTP!"));
        blocking(false,text);
    }
    else
        parseResult(AResult,AId);
}

//! "ICON:"
void PlaceViewForm::onImageReceived(const QByteArray &AResult,const QString &AId, bool AState)
{
    Q_UNUSED(AState)
    FNumberHttpReuest--;
    if (!AResult.isNull())
    {
        QString fileName;
        QString fileFormat;
        QSize AMaxSize;
        if(AId.contains("ICON:"))
        {
            QStringList name=AId.split(":");
            fileName =name[1];
            AMaxSize.setWidth(24);
            AMaxSize.setHeight(24);
        }
        else
        {
            QByteArray ARes(AResult);
            QBuffer buffer(&ARes);
            if(buffer.open(QIODevice::ReadOnly)){
                fileFormat= QImageReader::imageFormat(&buffer);
                buffer.close();
            }
            fileName =QString(AId).append(".").append(fileFormat);
            GeolocElement poi=FPoiMassive.value(AId);
			poi.setProperty("full_name", fileName);
            FPoiMassive.insert(AId,poi);
            //!--- Scale size ---
            float basa=PICT_SIZE_MAX;
			float width =poi.property("photo_width").toInt();
			float heigth=poi.property("photo_height").toInt();
            float rat=(width/heigth);
            if(rat>=1){
                width=basa;
                heigth=width/rat;
            }
            else{
                heigth=basa;
                width=heigth*rat;
            }
            AMaxSize.setWidth(width);
            AMaxSize.setHeight(heigth);
        }
//! Need save to disk in Temp directory
        if (!fileName.isEmpty())
        {
            QPixmap pixmap;
            pixmap.loadFromData(AResult);
            pixmap.scaled(AMaxSize,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(fileName);
        }
//! save to disk in Temp directory
    }
    superviseHttpRequest();
}

void PlaceViewForm::onPhotoPlaceReceived(const QByteArray &AResult,const QString &AId, bool AState)
{
    Q_UNUSED(AState)
    //! Receive HTML document -----
    FNumberHttpReuest--;
    if (!AResult.isNull())
    {
        QString content(AResult);
        QWebPage webPage;
        QWebFrame * frame = webPage.mainFrame();
        frame->setHtml(content);
        QWebElement doc = frame->documentElement();
        QString href=doc.findFirst("A").attribute("HREF");
        FProviders[FCurrentProvider]->getImage(href,AId);
        FNumberHttpReuest++;
    }
    superviseHttpRequest();
}

void  PlaceViewForm::superviseHttpRequest()
{
    if(FNumberHttpReuest==0 && !FNewQuery)
    {
        //QString text=QString("<font color='green'>Lat,Lng=%1,%2</font>").arg(FLat).arg(FLng);
        QString text=QString("<font color='green'>%1</font>").arg(tr("Ready!"));
        blocking(true,text);
        emit poisReady();
    }
}

void PlaceViewForm::repeatHttpRequest()
{
    onGetView();
}

void PlaceViewForm::blocking(bool status,QString AError)
{
//    ui->pbSaveToDisk->setEnabled(status);
    ui->pbClear->setEnabled(status);
    ui->lblView->setText(AError);
}

void PlaceViewForm::onClearToMap()
{
    blocking(false,"");
    FGoogleIcons.clear();
    emit poisDelete();
}

void PlaceViewForm::onOptionsOpened()
{
    onOptionsChanged(Options::node(OPV_MAP_PLACEVIEW_PROVIDER));
    onOptionsChanged(Options::node(OPV_MAP_PLACEVIEW_TYPE));
    onOptionsChanged(Options::node(OPV_MAP_PLACEVIEW_RADIUS));
    onOptionsChanged(Options::node(OPV_MAP_PLACEVIEW_RANKBY));
    onOptionsChanged(Options::node(OPV_MAP_PLACEVIEW_WAY));
}

void PlaceViewForm::onOptionsChanged(const OptionsNode &ANode)
{
    if  (ANode.path()==OPV_MAP_PLACEVIEW_PROVIDER)
        selectProvider(ANode.value().toString());
    else if(ANode.path()==OPV_MAP_PLACEVIEW_TYPE)
         ui->cBoxType->setCurrentIndex(ANode.value().toInt());

    else if(ANode.path()==OPV_MAP_PLACEVIEW_RADIUS)
        ui->cBoxRadius->setCurrentIndex(ANode.value().toInt());
    else if(ANode.path()==OPV_MAP_PLACEVIEW_RANKBY)
        ui->cBoxRankby->setCurrentIndex(ANode.value().toInt());
    else if(ANode.path()==OPV_MAP_PLACEVIEW_WAY)
        ui->cBoxWay->setCurrentIndex(ANode.value().toInt());
}

//status={"OK","ZERO_RESULTS","OVER_QUERY_LIMIT","REQUEST_DENIED","INVALID_REQUEST"}
//! Poi{"name","type","lat","lon","description","place_id","id",
//! "photo_is","photo_width","photo_height","photo_html_attribut","icon_name"}
void PlaceViewForm::parseResult(QByteArray ASearchResult, QString AId)
{
    Q_UNUSED(AId)
    QDomDocument doc;
    doc.setContent(ASearchResult);
    QDomElement placeSearchResponse=doc.documentElement();
    QDomElement status=placeSearchResponse.firstChildElement("status");
    if (!status.isNull())
    {
        if (status.text() == "OK")
        {
            if(!FNewQuery){
                FPoiMassive.clear();
                FNewQuery=true;
                FNextPage.clear();
            }
            for (QDomElement result=placeSearchResponse.firstChildElement("result");
                 !result.isNull();
                 result=result.nextSiblingElement("result"))
            {
                GeolocElement poi;
//!-- Form elements: name -------
                QString descript;
                QDomElement name=result.firstChildElement("name");
                if (!name.isNull()){
					poi.setProperty("name", name.text());
                    descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Name")).arg(name.text()));
                }
//!-- Form element type -----
                QString type_text;
                bool latch=false;
                for(QDomElement type=result.firstChildElement("type");
                    !type.isNull();
                    type=type.nextSiblingElement("type"))
                {
                    if(!latch){
                        latch=true;
						poi.setType(type.text());
                    }
                    type_text.append(type.text()).append(", ");
                }
                if(!type_text.isNull()){
                    type_text=type_text.remove(type_text.size()-2,2);
                    //poi.insert("type_text", type_text);
                    descript.append(QString("<tr><td width='60'><b>%1:</b></td><td>%2</td></tr>").arg(tr("Type")).arg(type_text));
                }
//! ------------
                QDomElement rating=result.firstChildElement("rating");
                if (!rating.isNull()){
                    //poi.insert("rating", rating.text());
                    descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Rating")).arg(rating.text()));
                }

                QDomElement price_level=result.firstChildElement("price_level");
                if (!price_level.isNull()){
                    //poi.insert("price_level", price_level.text());
                    descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Price level")).arg(price_level.text()));
                }

                QDomElement opening_hours=result.firstChildElement("opening_hours");
                if (!opening_hours.isNull())
                {
                    QDomElement open_now=opening_hours.firstChildElement("open_now");
                    if (!open_now.isNull()){
                        //poi.insert("open_now", open_now.text());
                        QString regim;
                        if(open_now.text()=="true")
                            regim=tr("Open");
                        else
                            regim=tr("Close");
                        descript.append(QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Now: ")).arg(regim));
                    }
                }
                QDomElement vicinity=result.firstChildElement("vicinity");
                if (!vicinity.isNull()){
                    //poi.insert("vicinity", vicinity.text());
                    descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Vicinity")).arg(vicinity.text()));
                }

//!-- Form element: address ----"Postal code"-
                QDomElement formatted_address=result.firstChildElement("formatted_address");
                if(!formatted_address.isNull()){
                    //poi.insert("address", formatted_address.text());
                    descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Address")).arg(formatted_address.text()));
                }

//! -- Form elements: events[] temporary not working !!!!----
                    for(QDomElement event=result.firstChildElement("events");
                        !event.isNull();
                        event=event.nextSiblingElement("events"))
                    {
                        QDomElement event_id=event.firstChildElement("event_id");
                        QDomElement summary=event.firstChildElement("summary");
                        QDomElement url=event.firstChildElement("url");
                    }

//!-- Form elements: lat,lon------
                QDomElement geometry=result.firstChildElement("geometry");
                if (!geometry.isNull())
                {
                    QDomElement location=geometry.firstChildElement("location");
                    if (!location.isNull())
                    {
                        QDomElement lat=location.firstChildElement("lat");
                        if (!lat.isNull())
							poi.setLat(lat.text().toDouble());
                        QDomElement lng=location.firstChildElement("lng");
                        if (!lng.isNull())
							poi.setLon(lng.text().toDouble());
                    }
					descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Location")).arg(getCoordString(poi.lat(), poi.lon())));
                }

//!-- Form element: scope -----
                    QDomElement scope=result.firstChildElement("scope");
                    if (!scope.isNull()){
                        //poi.insert("scope", scope.text());
                        descript.append(QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(tr("Scope")).arg(scope.text()));
                    }
				poi.setDescription(descript);

//!-- Form elements: place_id,reference, id ------
                QDomElement place_id=result.firstChildElement("place_id");
                if (!place_id.isNull())
					poi.setProperty("place_id", place_id.text());

                QDomElement reference=result.firstChildElement("reference");
                if (!reference.isNull())
					poi.setProperty("reference", reference.text());

                QDomElement id=result.firstChildElement("id");
                if (!id.isNull())
					poi.setProperty("id", id.text());

//!-- Form element: photo ----------------
/*              for(QDomElement photo=result.firstChildElement("photo");
                        !photo.isNull();
                        photo=photo.nextSiblingElement("photo"))
                    {  }
*/
                QDomElement photo=result.firstChildElement("photo");
                if (!photo.isNull())
                {
                    QDomElement photo_reference=photo.firstChildElement("photo_reference");
                    if (!photo_reference.isNull())
                    {
						poi.setProperty("photo_is", true);
                        QSize size;
                        QDomElement width=photo.firstChildElement("width");
                        if (!width.isNull()){
                            size.setWidth(width.text().toInt());
							poi.setProperty("photo_width", width.text().toInt());
                        }
                        QDomElement height=photo.firstChildElement("height");
                        if (!height.isNull()){
                            size.setWidth(height.text().toInt());
							poi.setProperty("photo_height", height.text().toInt());
                        }
                        QDomElement html_attributions=photo.firstChildElement("html_attributions");
                        if (!html_attributions.isNull())
							poi.setProperty("photo_html_attribut", html_attributions.text());
						FProviders[FCurrentProvider]->getPhotoPlace(size, photo_reference.text(), poi.property("id").toString());
                        FNumberHttpReuest++;
                    }
                }
//!-- Form element: icon_name ---------------
                QDomElement icon=result.firstChildElement("icon");
                if (!icon.isNull()){
                    QStringList split=icon.text().split("/");
                    QString iconName=split[split.size()-1];
					poi.setProperty("icon_name", iconName);
                    QString icon_ID=QString("ICON:").append(iconName);
                    if(!FGoogleIcons.contains(iconName))
                    {
                        FGoogleIcons.append(iconName);
                        FProviders[FCurrentProvider]->getImage(icon.text(),icon_ID);
                        FNumberHttpReuest++;
                    }
                }
                //! ----Insert poi in massive --------
				FPoiMassive.insert(poi.property("id").toString(), poi);
            }

            QDomElement next_page=placeSearchResponse.firstChildElement("next_page_token");
            if (!next_page.isNull())
            {
                FNextPage=next_page.text();
                 QTimer::singleShot(DELAY, this, SLOT(repeatHttpRequest()));//! Next query
            }
            else
            {
                FNewQuery=false;
                FNextPage.clear();
                FGoogleIcons.clear();
            }
        }
        else if (status.text() == "INVALID_REQUEST"){
            if(FNewQuery)
                QTimer::singleShot(DELAY, this, SLOT(repeatHttpRequest()));//! Next query
            else{
                QString text=QString("<font color='red'>%1</font>").arg(tr("INVALID REQUEST!"));
                blocking(false,text);
            }
        }
        else if(status.text() == "ZERO_RESULTS"){
            QString text=QString("<font color='red'>%1</font>").arg(tr("ZERO RESULTS!"));
            blocking(false,text);
        }
        else if(status.text() == "OVER_QUERY_LIMIT"){
            QString text=QString("<font color='red'>%1</font>").arg(tr("OVER QUERY LIMIT!"));
            blocking(false,text);
        }
        else if(status.text() == "REQUEST_DENIED"){
            QString text=QString("<font color='red'>%1</font>").arg(tr("REQUEST DENIED!"));
            blocking(false,text);
        }
        superviseHttpRequest();
    }
}

QString PlaceViewForm::getCoordString(double ALat, double ALon) const
{
	QString lat = QString::number(ALat, 'f', 8);
	QString lon = QString::number(ALon, 'f', 8);
    return tr("%1, %2").arg((lat[0]=='-')?tr("%1S").arg(lat.mid(1, 8)):tr("%1N").arg(lat.mid(0, 8)))
                       .arg((lon[0]=='-')?tr("%1W").arg(lon.mid(1, 8)):tr("%1E").arg(lon.mid(0, 8)));
}


void PlaceViewForm::setTypeGooglePoi()
{
    FListTypes
        << "accounting"
        << "airport"
        << "amusement_park"
        << "aquarium"
        << "art_gallery"
        << "atm"
        << "bakery"
        << "bank"
        << "bar"
        << "beauty_salon"
        << "bicycle_store"
        << "book_store"
        << "bowling_alley"
        << "bus_station"
        << "cafe"
        << "campground"
        << "car_dealer"
        << "car_rental"
        << "car_repair"
        << "car_wash"
        << "casino"
        << "cemetery"
        << "church"
        << "city_hall"
        << "clothing_store"
        << "convenience_store"
        << "courthouse"
        << "dentist"
        << "department_store"
        << "doctor"
        << "electrician"
        << "electronics_store"
        << "embassy"
        << "establishment"
        << "finance"
        << "fire_station"
        << "florist"
        << "food"
        << "funeral_home"
        << "furniture_store"
        << "gas_station"
        << "general_contractor"
        << "grocery_or_supermarket"
        << "gym"
        << "hair_care"
        << "hardware_store"
        << "health"
        << "hindu_temple"
        << "home_goods_store"
        << "hospital"
        << "insurance_agency"
        << "jewelry_store"
        << "laundry"
        << "lawyer"
        << "library"
        << "liquor_store"
        << "local_government_office"
        << "locksmith"
        << "lodging"
        << "meal_delivery"
        << "meal_takeaway"
        << "mosque"
        << "movie_rental"
        << "movie_theater"
        << "moving_company"
        << "museum"
        << "night_club"
        << "painter"
        << "park"
        << "parking"
        << "pet_store"
        << "pharmacy"
        << "physiotherapist"
        << "place_of_worship"
        << "plumber"
        << "police"
        << "post_office"
        << "real_estate_agency"
        << "restaurant"
        << "roofing_contractor"
        << "rv_park"
        << "school"
        << "shoe_store"
        << "shopping_mall"
        << "spa"
        << "stadium"
        << "storage"
        << "store"
        << "subway_station"
        << "synagogue"
        << "taxi_stand"
        << "train_station"
        << "travel_agency"
        << "university"
        << "veterinary_care";

    QStringList listPoiTr;
    listPoiTr
        << tr("Accounting")
        << tr("Airport")
        << tr("Amusement park")
        << tr("Aquarium")
        << tr("Art gallery")
        << tr("Atm")
        << tr("Bakery")
        << tr("Bank")
        << tr("Bar")
        << tr("Beauty salon")
        << tr("Bicycle store")
        << tr("Book store")
        << tr("Bowling alley")
        << tr("Bus station")
        << tr("Cafe")
        << tr("Campground")
        << tr("Car dealer")
        << tr("Car rental")
        << tr("Car repair")
        << tr("Car wash")
        << tr("Casino")
        << tr("Cemetery")
        << tr("Church")
        << tr("City hall")
        << tr("Clothing store")
        << tr("Convenience store")
        << tr("Court house")
        << tr("Dentist")
        << tr("Department store")
        << tr("Doctor")
        << tr("Electrician")
        << tr("Electronics store")
        << tr("Embassy")
        << tr("Establishment")
        << tr("Finance")
        << tr("Fire station")
        << tr("Florist")
        << tr("Food")
        << tr("Funeral home")
        << tr("Furniture store")
        << tr("Gas station")
        << tr("General contractor")
        << tr("Grocery or supermarket")
        << tr("Gym")
        << tr("Hair care")
        << tr("Hardware store")
        << tr("Health")
        << tr("Hindu temple")
        << tr("Home goods store")
        << tr("Hospital")
        << tr("Insurance agency")
        << tr("Jewelry store")
        << tr("Laundry")
        << tr("Lawyer")
        << tr("Library")
        << tr("Liquor store")
        << tr("Localgovernment office")
        << tr("Locksmith")
        << tr("Lodging")
        << tr("Meal delivery")
        << tr("Meal takeaway")
        << tr("Mosque")
        << tr("Movie rental")
        << tr("Movie theater")
        << tr("Moving company")
        << tr("Museum")
        << tr("Night club")
        << tr("Painter")
        << tr("Park")
        << tr("Parking")
        << tr("Pet store")
        << tr("Pharmacy")
        << tr("Physiotherapist")
        << tr("Place of worship")
        << tr("Plumber")
        << tr("Police")
        << tr("Post office")
        << tr("Real estate agency")
        << tr("Restaurant")
        << tr("Roofing contractor")
        << tr("Rv park")
        << tr("School")
        << tr("Shoe store")
        << tr("Shopping mall")
        << tr("Spa")
        << tr("Stadium")
        << tr("Storage")
        << tr("Store")
        << tr("Subway station")
        << tr("Synagogue")
        << tr("Taxi stand")
        << tr("Train station")
        << tr("Travel agency")
        << tr("University")
        << tr("Veterinary care");

    for(int i=0;i<listPoiTr.size();i++)
        FHashPoiTypes.insert(FListTypes[i],listPoiTr[i]);

    listPoiTr.sort();
    ui->cBoxType->insertItems(0,listPoiTr);

    FListWayToSearch << "nearbysearch" << "radarsearch" << "textsearch";
    QStringList wayToSearchTr;
    wayToSearchTr <<tr("Near by search") << tr("Radar search") << tr("Text search");
    ui->cBoxWay->insertItems(0,wayToSearchTr);

    FListRankby << "prominence" << "distance";
    QStringList rankbyTr;
    rankbyTr << tr("Prominence") << tr("Distance");
    ui->cBoxRankby->insertItems(0,rankbyTr);
}

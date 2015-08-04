#include <QUrl>
#include <QSvgWidget>
#include <QSvgRenderer>
#include <QAbstractItemModel>
#include <QStringListModel>

#include <definitions/resources.h>
#include <definitions/namespaces.h>
#include "definitions/mapicons.h"

#include "newpoi.h"
#include "ui_newpoi.h"

#define  ID_ENGLISH 100
#define  FOLDER_ICON        "folder"
#define  FOLDEROPEN_ICON    "folderopen"

NewPoi::NewPoi(Poi *APoi, IMapLocationSelector *AMapLocationSelector, QList<IAccount *> &AAccounts, const QString &ATitle, const GeolocElement &APoiData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewPoi),
    FPoi(APoi),
    FMapLocationSelector(AMapLocationSelector),
    FAccounts(&AAccounts),
    FCountryIconStorage(IconStorage::staticStorage(RSR_STORAGE_COUNTRY)),    
    FLocationSelect(false),
    FExtendedView(true),
    FCountryCodeSet(false),
    FSchemes(QStringList() << "http" << "https" << "ftp" << "xmpp" << "mailto" << "tel" << "native")
{
    ui->setupUi(this);
    setWindowTitle(ATitle);
    setWindowIcon(APoi->getIcon(MNI_POI_ADD));
    init();
    if (!APoiData.isEmpty())
        setEditPoi(APoiData);
    onMoreClicked();

    if (FMapLocationSelector)
        connect(ui->selectLocation, SIGNAL(clicked()), SLOT(onSelectLocationClicked()));
    else
        ui->selectLocation->hide();
}

NewPoi::~NewPoi()
{
    delete ui;
}

void NewPoi::onNameEdited(const QString &AComboBox)
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setDisabled(AComboBox.isEmpty() && !FEmptyNameAllowed);
}

void NewPoi::onMoreClicked()
{
    if((FExtendedView=!FExtendedView))
		ui->gpbAddress->show();
    else
		ui->gpbAddress->hide();
}

void NewPoi::init()
{
    if (FMapLocationSelector)
		ui->selectLocation->setIcon(IconStorage::staticStorage(RSR_STORAGE_MAP)->getIcon(MPI_NEWCENTER));
    ui->pbTimestamp->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_CLIENTINFO_TIME));

    fillCountryMap();

    //--- Setup country names and codes ---
    ui->boxCountry->addItem(tr("None"));
	ui->boxCountryCode->addItem(tr("None"));
    QStringList keys=FCountryCodeMap.keys();
    for(QStringList::const_iterator it=keys.constBegin(); it!=keys.constEnd(); it++)
    {
        ui->boxCountry->addItem(FCountryCodeMap[*it], *it);
        ui->boxCountryCode->addItem(FCountryIconStorage->getIcon(*it), *it);
    }
	ui->boxCountry->setCurrentIndex(0);

    //--- Setup POI types ---
    QHash<QString, QString> lstTranslated = FPoi->getTranslatedTypes();
    QMap<QString, QString> lstAllType = FPoi->getTypeMap();

    QStringList rootTypes(FPoi->getTypeMap().keys());
    rootTypes.removeDuplicates();

    ui->boxType->setItemDelegate(new TypeItemDelegate);
    ui->boxType->addItem(FPoi->getIcon(MNI_POI_NONE), lstTranslated.value("none"), "dir:none");
    for(QStringList::const_iterator it=rootTypes.constBegin(); it!=rootTypes.constEnd(); it++)
    {
        QStringList lst = lstAllType.values(*it);
        ui->boxType->addItem(FPoi->getTypeIcon(*it), lstTranslated.value(*it), "dir:"+*it);
        for(QStringList::const_iterator pit=lst.constBegin(); pit!=lst.constEnd(); pit++)
        {
            QString id=*it+':'+*pit;
            ui->boxType->addItem(FPoi->getTypeIcon(id), lstTranslated.value(id), id);
        }
    }
    ui->boxType->setEditable(false);
    setTimestamp(QDateTime());
}

GeolocElement NewPoi::getPoi()
{
    GeolocElement geoloc;

    if(!ui->editText->text().isEmpty())
		geoloc.setText(ui->editText->text());
    if(ui->dspbLatitude->hasAcceptableInput())
		geoloc.setLat(ui->dspbLatitude->value());
    if(ui->dspbLongitude->hasAcceptableInput())
		geoloc.setLon(ui->dspbLongitude->value());
    if(!ui->editDescription->toPlainText().isEmpty())
		geoloc.setDescription(ui->editDescription->toPlainText());

    if(ui->boxType->currentIndex()>=0) //>0
    {
        int index = ui->boxType->currentIndex();
		QString type =ui->boxType->itemData(index, Qt::UserRole).toString();
		QStringList lst=type.split(":");
        if(lst[0]=="dir")
			type=lst[1];
		if (type!="none")
			geoloc.setType(type);
    }

	if(ui->boxCountry->currentIndex())
		geoloc.setCountry(ui->boxCountry->currentText());
	if(ui->boxCountryCode->currentIndex())
		geoloc.setCountryCode(ui->boxCountryCode->currentText());
    if(!ui->editRegion->text().isEmpty())
		geoloc.setRegion(ui->editRegion->text());
    if(!ui->editLocality->text().isEmpty())
		geoloc.setLocality(ui->editLocality->text());
    if(!ui->editStreet->text().isEmpty())
		geoloc.setStreet(ui->editStreet->text());
    if(!ui->editBuilding->text().isEmpty())
		geoloc.setBuilding(ui->editBuilding->text());
    if(!ui->editFloor->text().isEmpty())
		geoloc.setFloor(ui->editFloor->text());
    if(!ui->editRoom->text().isEmpty())
		geoloc.setRoom(ui->editRoom->text());
    if(!ui->editPostalCode->text().isEmpty())
		geoloc.setPostalCode(ui->editPostalCode->text());
    if(!ui->editArea->text().isEmpty())
		geoloc.setArea(ui->editArea->text());
    if(ui->dteTimestamp->hasAcceptableInput())
		geoloc.setTimeStamp(ui->dteTimestamp->dateTime());

    if(!ui->editUri->text().isEmpty())
    {
		QUrl url =QUrl::fromUserInput(ui->editUri->text());
        if (url.isValid())
			geoloc.setUri(url);
    }
    return geoloc;
}

void NewPoi::initStreamList(const QString &ABareJid, bool AEditable)
{
    if (FAccounts)
    {
        Jid streamJid;
        if (!ABareJid.isEmpty())
            streamJid=FPoi->findStreamJid(ABareJid);
        for (QList<IAccount *>::const_iterator it=FAccounts->constBegin(); it!=FAccounts->constEnd(); it++)
            ui->boxAccount->addItem((*it)->name(), QVariant((*it)->streamJid().full()));
        ui->boxAccount->setCurrentIndex(streamJid.isValid()?ui->boxAccount->findData(QVariant(streamJid.full())):0);
    }
    ui->boxAccount->setEnabled(AEditable);
}

Jid NewPoi::getStreamJid()
{
    return Jid(ui->boxAccount->itemData(ui->boxAccount->currentIndex()).toString());
}

void NewPoi::setEditPoi(const GeolocElement &AElement)
{
	if(AElement.hasProperty(GeolocElement::Text))
		ui->editText->setText(AElement.text());
	if(AElement.hasProperty(GeolocElement::Lat))
		ui->dspbLatitude->setValue(AElement.lat());
	if(AElement.hasProperty(GeolocElement::Lon))
		ui->dspbLongitude->setValue(AElement.lon());
	if(AElement.hasProperty(GeolocElement::Description))
		ui->editDescription->setPlainText(AElement.description());

	QString type = AElement.hasProperty(GeolocElement::Type)?AElement.type():"none";
	int index = ui->boxType->findData(type,Qt::UserRole,Qt::MatchStartsWith | Qt::MatchEndsWith);
    if(index>=0)
        ui->boxType->setCurrentIndex(index);

	if(AElement.hasProperty(GeolocElement::CountryCode))
    {
		int index=ui->boxCountryCode->findText(AElement.countryCode(), Qt::MatchExactly);
        if (index>=0)
        {
            ui->boxCountryCode->setCurrentIndex(index);
            FCountryCodeSet = true;
        }
    }
    else
        ui->boxCountryCode->setEditText(QString());

	ui->boxCountry->setEditText(AElement.country());

	if(AElement.hasProperty(GeolocElement::Region))
		ui->editRegion->setText(AElement.region());
	if(AElement.hasProperty(GeolocElement::Locality))
		ui->editLocality->setText(AElement.locality());
	if(AElement.hasProperty(GeolocElement::Street))
		ui->editStreet->setText(AElement.street());
	if(AElement.hasProperty(GeolocElement::Building))
		ui->editBuilding->setText(AElement.building());
	if(AElement.hasProperty(GeolocElement::Floor))
		ui->editFloor->setText(AElement.floor());
	if(AElement.hasProperty(GeolocElement::Room))
		ui->editRoom->setText(AElement.room());
	if(AElement.hasProperty(GeolocElement::PostalCode))
		ui->editPostalCode->setText(AElement.postalCode());
	if(AElement.hasProperty(GeolocElement::Area))
		ui->editArea->setText(AElement.area());

	if(AElement.hasProperty(GeolocElement::TimeStamp))
		setTimestamp(AElement.timeStamp());

	if(AElement.hasProperty(GeolocElement::Uri))
        ui->editUri->setText(AElement.uri().toString());
    FCountryCodeSet = false;
}

void NewPoi::hideEvent(QHideEvent *AEvent)
{
	Q_UNUSED(AEvent)

    if (FLocationSelect)
    {
        FLocationSelect=false;
        FMapLocationSelector->finishSelectLocation(this, true);
    }
}

void NewPoi::changeEvent(QEvent *AEvent)
{
    QDialog::changeEvent(AEvent);
    switch (AEvent->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void NewPoi::setLocation(const MercatorCoordinates &ACoordinates)
{
	ui->dspbLatitude->setValue(ACoordinates.latitude());
	ui->dspbLongitude->setValue(ACoordinates.longitude());
}

void NewPoi::allowEmptyName(bool AAllow)
{
    FEmptyNameAllowed=AAllow;
    onNameEdited(ui->editText->text());
}

// SLOTS
void NewPoi::onCountrySelected(int AIndex)
{
	if (AIndex>0)
    {
        QString code=ui->boxCountry->itemData(AIndex).toString();
        if (!FCountryCodeSet)
		{
			int index = ui->boxCountryCode->findText(code);
			if (index<0)
				index = 0;
			ui->boxCountryCode->setCurrentIndex(index);
		}
    }
    else
        if (!FCountryCodeSet)
			ui->boxCountryCode->setCurrentIndex(0);
}

void NewPoi::onCountryCodeSelected(int AIndex)
{
    QString code(ui->boxCountryCode->itemText(AIndex));
	int index = ui->boxCountry->findData(code);
	if (index<0)
		index=0;
	ui->boxCountry->setCurrentIndex(index);
    ui->wgtFlag->load(FCountryIconStorage->fileFullName(code));
    QSize ssizeHint = ui->wgtFlag->sizeHint();
    ssizeHint.scale(48,48,Qt::KeepAspectRatio);
	ui->wgtFlag->resize(ssizeHint);
}

void NewPoi::onComboBoxEdited(const QString &AText)
{
	QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
	if (comboBox && AText.isEmpty())
		comboBox->setCurrentIndex(0);
}

void NewPoi::onSetTimestampClicked()
{
    setTimestamp(ui->dteTimestamp->isVisible()?QDateTime():
#if QT_VERSION >= 0x040700
                                               QDateTime::currentDateTimeUtc()
#else
                                               QDateTime::currentDateTime().toUTC()
#endif
                                               );
}

void NewPoi::onSelectLocationClicked()
{    
    if (FMapLocationSelector->selectLocation(this))
    {
        FLocationSelect=true;
        ui->selectLocation->setEnabled(false);
    }
}

void NewPoi::onLocationSelected()
{
    setLocation(FMapLocationSelector->selectedLocation());
    onLocationSelectionCancelled();
}

void NewPoi::onLocationSelectionCancelled()
{
    FLocationSelect=false;
    ui->selectLocation->setEnabled(true);
    activateWindow();
}

void NewPoi::onSchemeChanged(int AIndex)
{
    QUrl url(ui->editUri->text());
    url.setScheme(AIndex?FSchemes[AIndex-1]:QString());
    ui->editUri->setText(url.toString());
}

void NewPoi::onUriChanged(const QString &ANewUri)
{
    QUrl url = QUrl::fromUserInput(ANewUri);
    ui->cmbScheme->blockSignals(true);
    ui->cmbScheme->setCurrentIndex(FSchemes.indexOf(url.scheme())+1);
    ui->cmbScheme->blockSignals(false);
}

void NewPoi::setTimestamp(const QDateTime &ATimeStamp)
{
    if (ATimeStamp.isValid())
    {
        ui->dteTimestamp->setDateTime(ATimeStamp);
        ui->dteTimestamp->setVisible(true);
        ui->pbTimestamp->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_EDIT_DELETE));
        ui->pbTimestamp->setToolTip(tr("Remove timestamp"));
    }
    else
    {
		ui->dteTimestamp->clear();
        ui->dteTimestamp->setVisible(false);
        ui->pbTimestamp->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_CLIENTINFO_TIME));
        ui->pbTimestamp->setToolTip(tr("Set current time"));
    }
}

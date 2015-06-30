#ifndef NEWPOI_H
#define NEWPOI_H

#include <interfaces/imap.h>
#include <interfaces/imaplocationselector.h>
#include <interfaces/iaccountmanager.h>
#include <utils/iconstorage.h>
#include <utils/jid.h>

#include "poi.h"
#include "typeitemdelegate.h"

namespace Ui {
    class NewPoi;
}

class NewPoi : public QDialog, public ILocationSelector
{
    Q_OBJECT
    Q_INTERFACES(ILocationSelector)
public:    
	NewPoi(Poi *APoi, IMapLocationSelector *AMapLocationSelector, QList<IAccount *> &AAccounts, const QString &ATitle, const GeolocElement &APoiData = GeolocElement(), QWidget *parent = 0);
	~NewPoi();

	// ILocationSelector interface
	virtual QObject *instance() { return this; }

	GeolocElement getPoi();
	void initStreamList(const QString &ABareJid = QString(), bool AEditable=true);
	Jid getStreamJid();
	void setLocation(const MercatorCoordinates &ACoordinates);
	void allowEmptyName(bool AAllow=true);

protected:
    void setEditPoi(const GeolocElement &AElement);
    void setTimestamp(const QDateTime &ATimeStamp);
    void fillCountryMap();
    void changeEvent(QEvent *AEvent);
    void init();
    void loadDef();
    void hideEvent (QHideEvent *AEvent);

protected slots:
    void onCountrySelected(int AIndex);
    void onCountryCodeSelected(int AIndex);
    void onComboBoxEdited(const QString &AComboBox);
    void onNameEdited(const QString &AComboBox);
    void onSelectLocationClicked();
    void onSetTimestampClicked();
    void onLocationSelected();
    void onLocationSelectionCancelled();
    void onMoreClicked();
    void onSchemeChanged(int AIndex);
    void onUriChanged(const QString &ANewUri);

private:
	Ui::NewPoi				*ui;
    Poi                     *FPoi;
    IMapLocationSelector*   FMapLocationSelector;
    QList<IAccount*>*       FAccounts;
    IconStorage*            FCountryIconStorage;
    QMap<QString, QString>  FCountryCodeMap;

    bool                    FEmptyNameAllowed;
    bool                    FLocationSelect;
    bool                    FExtendedView;
    bool                    FCountryCodeSet;

    const QStringList       FSchemes;
};

#endif // NEWPOI_H

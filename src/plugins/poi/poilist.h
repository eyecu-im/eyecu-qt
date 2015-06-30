#ifndef POILIST_H
#define POILIST_H

#include <interfaces/ipoi.h>
#include <definitions/menuicons.h>
#include <utils/jid.h>

#include "ui_poilist.h"

namespace Ui {
    class PoiList;
}

class PoiList : public QDialog
{
    Q_OBJECT

public:
    PoiList(IPoi *APoi, QWidget *parent = 0);
    ~PoiList();

    void fillTable(const PoiHash &APoiHash, QString ABareJid);
    void fillTable(QHash<QString, PoiHash> &AGeolocHash);
    void appendStreamPois(const PoiHash &APoiHash, QString ABareJid);
    QString selectedId() const;

    Ui::PoiList *ui;

protected:
    void changeEvent(QEvent *e);

protected slots:
    void onItemActivated(QTreeWidgetItem *selectPitem, int);
    void onCustomContextMenuRequested(const QPoint &APos);
    void onPoiModified(const QString &AId, int AType);
    void onPoisLoaded(const QString &ABareSteamJid, const PoiHash &APoiHash);
    void onPoisRemoved(const QString &ABareSteamJid);

private:
    IPoi    *FPoi;
    QString FBareStreamJid;
};

#endif // POILIST_H

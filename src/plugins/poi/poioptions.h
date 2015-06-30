#ifndef POIOPTIONS_H
#define POIOPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>
#include <utils/iconstorage.h>
#include <definitions/resources.h>

#include "poi.h"
#include "ui_poioptions.h"

namespace Ui
{
    class poiOptions;
}

class PoiOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    PoiOptions(Poi *APoi, QWidget *parent = 0);
    ~PoiOptions();
    virtual QWidget* instance() { return this; }
    Ui::poiOptions *ui;
    QFont  FCurrentFont;
    QColor FCurrentTextColor;
    QColor FCurrentShadowColor;
    QColor FCurrentTempTextColor;
    QStringList getFilter();

// IOptionsWidget
public slots:
	virtual void apply();
	virtual void reset();

signals:
	void modified();
	void childApply();
	void childReset();

protected slots:
    void modifyFont();
    void modifyColor();
    void modifyTempColor();
    void modifyShadowColor();
    void onBackgroundSelected();
    void onItemChanged(QTreeWidgetItem*,int);
    void onCheckAll();
    void onUncheckAll();

private:
    Poi *FPoi;
    QStringList FPoiFilter;
    QStringList FRootTypes;
    QMap<QString, QString> FSubTypes;
    QHash<QString, QString> FTranslatedTypes;

protected:
    void createTypeWidget();
    void updateTypeWidget();
    void groupItemChanged(QTreeWidgetItem *item);
    void changeEvent(QEvent *e);
};

#endif // POIOPTIONS_H

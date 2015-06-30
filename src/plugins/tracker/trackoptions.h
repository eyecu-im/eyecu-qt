#ifndef TRACKOPTIONS_H
#define TRACKOPTIONS_H

#include <QFontDialog>
#include <QColorDialog>

#include <interfaces/itracker.h>
#include <interfaces/ioptionsmanager.h>
#include <utils/iconstorage.h>
#include <definitions/resources.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>

#include "ui_trackoptions.h"

namespace Ui {
    class TrackOptions;
}

class TrackOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    TrackOptions(ITracker *AManager,QWidget *parent = 0);
    ~TrackOptions();
    virtual QWidget* instance() { return this; }
    Ui::TrackOptions *ui;
    QFont  FCurrentFont;
    QColor FLineColor;
    QColor FCurrentTextColor;
    QColor FCurrentShadowColor;


public slots:
    virtual void apply();
    virtual void reset();
signals:
    void modified();
    void childApply();
    void childReset();
    void locModify();

protected slots:
    void modifyColor();
    void modifyTxtColor();
    void modifyShadColor();
    void modifyFont();
    void bgrMap();
    void bgrSat();
    void onBackgroundSelected();

protected:
    void changeEvent(QEvent *e);
    void init();

private:
	ITracker *FTracker;
};

#endif // TRACKOPTIONS_H

#ifndef MAPOPTIONS_H
#define MAPOPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class MapOptions;
}

class MapOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit MapOptions(QWidget *parent = 0);
    ~MapOptions();

// IOptionsDialoWidget
    QWidget* instance() {return this; }

public slots:
    void apply();
    void reset();

protected slots:
    void modifyFont();
    void modifyColor();
    void modifyControlForeground();
    void modifyControlBase();
    void modifyControlButton();
    void modifyControlLight();
    void modifyControlMidlight();
    void modifyControlDark();
    void modifyControlShadow();
    void modifyControlBackground();
    void modifyCenterMarkerColor();
    void modifyShadowColor();

    void modifyBoxForeground();
    void modifyBoxLight();
    void modifyBoxMidlight();
    void modifyBoxDark();
    void modifyBoxBackground();

    void onBackgroundAlphaChange(int);
    void onCntrBackgroundAlphaChange(int);
    void onCenterMarkerAlphaChange(int value);

    void onShadowAlphaChange(int);
    void onShadowBlurChange(int blur);
    void onShadowShiftXChange(int);
    void onShadowShiftYChange(int cy);

// IOptionsWidget
signals:
    void modified();
    void childApply();
    void childReset();

protected:
    void changeEvent(QEvent *e);
    void setWidgetColor(QWidget *AWidget, const QColor &AColor);

private:
    Ui::MapOptions *ui;

    QFont  currFont;
    QColor currColor;
    QColor currShadowColor;
    qreal  currShadowBlur;

    QColor contrBase;
    QColor contrButton;
    QColor contrForeground;
    QColor contrLight;
    QColor contrMidlight;
    QColor contrShadow;
    QColor contrDark;
    QColor contrBackground;
    QColor contrCentralMarker;

    QColor boxForeground;
    QColor boxLight;
    QColor boxMidlight;
    QColor boxDark;
    QColor boxBackground;

    QPointF currShadowShift;
};

#endif // MAPOPTIONS_H

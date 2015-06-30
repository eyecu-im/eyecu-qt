#ifndef MAGNIFIEROPTIONS_H
#define MAGNIFIEROPTIONS_H

#include <QWidget>
#include <QPointF>
#include <QColor>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class MagnifierOptions;
}

class MagnifierOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:   
    explicit MagnifierOptions(QWidget *parent = 0);
    ~MagnifierOptions();

    virtual QWidget* instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

protected slots:
    void onShadowColorButtonClicked();
    void onZoomFactorColorButtonClicked();
    void onZoomFactorFontButtonClicked();

signals:
    void modified();
    void childApply();
    void childReset();

private:
    Ui::MagnifierOptions *ui;
    QColor  FShadowColor;
    QColor  FZoomFactorColor;
    QFont   FZoomFactorFont;
};

#endif // MAGNIFIEROPTIONS_H

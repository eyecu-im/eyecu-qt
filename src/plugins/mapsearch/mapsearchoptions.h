#ifndef MAPSEARCHOPTIONS_H
#define MAPSEARCHOPTIONS_H

#include <QColor>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class MapSearchOptions;
}

class MapSearchOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit MapSearchOptions(QWidget *parent = 0);
    ~MapSearchOptions();
    QWidget *instance() {return this;}

protected:
    void changeEvent(QEvent *e);
    void setWidgetColor(QWidget *AWidget, const QColor &AColor);

protected slots:
    void onTextColorDialog();

private:
    Ui::MapSearchOptions *ui;
    QColor  FCurrentColor;

// IOptionsWidget
public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();
};

#endif // MAPSEARCHOPTIONS_H

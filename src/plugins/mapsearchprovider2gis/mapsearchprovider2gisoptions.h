#ifndef MAPSEARCHPROVIDER2GISOPTIONS_H
#define MAPSEARCHPROVIDER2GISOPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
    class MapSearchProvider2GisOptions;
}

class MapSearchProvider2GisOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    enum {
        SearchFirm,
        SearchGeo
    };

    explicit MapSearchProvider2GisOptions(QWidget *parent = 0);
    ~MapSearchProvider2GisOptions();

    // IOptionsWidget interface
    QWidget *instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

private:
    Ui::MapSearchProvider2GisOptions *ui;
};

#endif // MAPSEARCHPROVIDER2GISOPTIONS_H

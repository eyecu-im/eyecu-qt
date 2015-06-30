#ifndef TUNEOPTIONS_H
#define TUNEOPTIONS_H

#include <QUuid>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/itune.h>
#include <utils/options.h>

namespace Ui {
class TuneOptions;
}

class TuneOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    TuneOptions(QHash<QUuid, ITuneInfoRequester *> ARequesters, QWidget *parent = 0);
    ~TuneOptions();

    // IOptionsWidget
    virtual QWidget* instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

    void clearCache();

private:
    Ui::TuneOptions *ui;
};

#endif // TUNEOPTIONS_H

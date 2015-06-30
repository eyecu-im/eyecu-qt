#ifndef TUNEINFOREQUESTERLASTFMOPTIONS_H
#define TUNEINFOREQUESTERLASTFMOPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
    class TuneInfoRequesterLastFmOptions;
}

class TuneInfoRequesterLastFmOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit TuneInfoRequesterLastFmOptions(QWidget *parent = 0);
    ~TuneInfoRequesterLastFmOptions();

    // IOptionsWidget
    virtual QWidget* instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();
    
private:
    Ui::TuneInfoRequesterLastFmOptions *ui;
};

#endif // TUNEINFOREQUESTERLASTFMOPTIONS_H

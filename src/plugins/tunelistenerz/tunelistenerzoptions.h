#ifndef TUNELISTENERZOPTIONS_H
#define TUNELISTENERZOPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

namespace Ui {
class TuneListenerZOptions;
}

class TuneListenerZOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit TuneListenerZOptions(QWidget *parent = 0);
    ~TuneListenerZOptions();

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
    Ui::TuneListenerZOptions *ui;
};

#endif // TUNELISTENERZOPTIONS_H

#ifndef TUNELISTENERQUPLAYEROPTIONS_H
#define TUNELISTENERQUPLAYEROPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

namespace Ui {
class TuneListenerQuPlayerOptions;
}

class TuneListenerQuPlayerOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)
    
public:
    explicit TuneListenerQuPlayerOptions(QWidget *parent = 0);
    ~TuneListenerQuPlayerOptions();

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
    Ui::TuneListenerQuPlayerOptions *ui;
};

#endif // TUNELISTENERQUPLAYEROPTIONS_H

#ifndef TUNELISTENERPM123OPTIONS_H
#define TUNELISTENERPM123OPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

namespace Ui {
class TuneListenerPm123Options;
}

class TuneListenerPm123Options : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
    Q_INTERFACES(IOptionsWidget)
    
public:
    explicit TuneListenerPm123Options(QWidget *parent = 0);
    ~TuneListenerPm123Options();

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
    Ui::TuneListenerPm123Options *ui;
};

#endif // TUNELISTENERPM123OPTIONS_H

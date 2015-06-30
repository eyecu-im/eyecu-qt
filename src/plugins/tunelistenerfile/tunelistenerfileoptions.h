#ifndef TUNELISTENERFILEOPTIONS_H
#define TUNELISTENERFILEOPTIONS_H

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

namespace Ui {
class TuneListenerFileOptions;
}

class TuneListenerFileOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    explicit TuneListenerFileOptions(QWidget *parent = 0);
    ~TuneListenerFileOptions();

// IOptionsWidget
    virtual QWidget* instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected slots:
    void onBrowseClicked();

private:
    Ui::TuneListenerFileOptions *ui;
};

#endif // TUNELISTENERFILEOPTIONS_H

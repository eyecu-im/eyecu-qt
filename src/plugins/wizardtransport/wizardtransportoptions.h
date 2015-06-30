#ifndef WIZARDTRANSPORTOPTIONS_H
#define WIZARDTRANSPORTOPTIONS_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>


namespace Ui {
class WizardTransportOptions;
}

class WizardTransportOptions : public QWidget, public IOptionsWidget
{
    Q_OBJECT
    Q_INTERFACES(IOptionsWidget)

public:
    explicit WizardTransportOptions(QWidget *parent = 0);
    ~WizardTransportOptions();
    // IOptionsWidget
    virtual QWidget* instance() {return this; }

public slots:
    virtual void apply();
    virtual void reset();
protected slots:
    void onStartWizard();

signals:
    void modified();
    void childApply();
    void childReset();
private:
    Ui::WizardTransportOptions *ui;
};

#endif // WIZARDTRANSPORTOPTIONS_H

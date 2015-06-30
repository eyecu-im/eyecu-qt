#ifndef CONTACTPROXIMITYNOTIFICATIONOPTIONS_H
#define CONTACTPROXIMITYNOTIFICATIONOPTIONS_H

#include <QSpinBox>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class ContactProximityNotificationOptions;
}

class ContactProximityNotificationOptions : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    explicit ContactProximityNotificationOptions(QWidget *parent = 0);
    ~ContactProximityNotificationOptions();   
    // IOptionsWidget interface
    virtual QWidget *instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

protected:
    void updateUnits(QSpinBox *ASpinBox);

protected slots:
    void onValueChanged(int AValue);
    void onCheckStateChanged(int AChecked);

signals:
    void modified();
    void childApply();
    void childReset();

private:
    Ui::ContactProximityNotificationOptions *ui;
};

#endif // CONTACTPROXIMITYNOTIFICATIONOPTIONS_H

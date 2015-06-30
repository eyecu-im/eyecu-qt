#ifndef SETTINGSGOOGLE_H
#define SETTINGSGOOGLE_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class SettingsGoogle;
}

class SettingsGoogle : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES (IOptionsDialogWidget)
    
public:
    explicit SettingsGoogle(QWidget *parent = 0);
    ~SettingsGoogle();

    //IOptionsWidgets
    virtual QWidget* instance() {return this;}

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected:
    void changeEvent(QEvent *e);

protected slots:
    void onDefaultClicked();

private:
    Ui::SettingsGoogle *ui;
};

#endif // SETTINGSGOOGLE_H

#ifndef OPTIONSYANDEX_H
#define OPTIONSYANDEX_H

#include <QWidget>
#include <interfaces/ioptionsmanager.h>

namespace Ui {
class OptionsYandex;
}

class OptionsYandex : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES (IOptionsDialogWidget)

public:
    explicit OptionsYandex(QWidget *parent = 0);
    ~OptionsYandex();

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
    Ui::OptionsYandex *ui;
};

#endif // OPTIONSYANDEX_H

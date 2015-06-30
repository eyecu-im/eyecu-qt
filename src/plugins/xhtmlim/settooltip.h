#ifndef SETTOOLTIP_H
#define SETTOOLTIP_H

#include <QDialog>

namespace Ui {
class SetToolTip;
}

class SetToolTip : public QDialog
{
    Q_OBJECT
    
public:
    enum Type
    {
        None,
        Acronym,
        Abbreviation
    };

    explicit SetToolTip(int AType, const QString &ATitleText=QString(), QWidget *parent = 0);
    ~SetToolTip();
    QString toolTipText() const;
    int type() const;
    
private:
    Ui::SetToolTip *ui;
};

#endif // SETTOOLTIP_H

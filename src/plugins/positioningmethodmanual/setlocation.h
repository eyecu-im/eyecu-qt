#ifndef SETLOCATION_H
#define SETLOCATION_H

#include <QDialog>
#include <QDialogButtonBox>
#include <MercatorCoordinates>

namespace Ui {
    class SetLocation;
}

class SetLocation : public QDialog {
    Q_OBJECT
public:
	enum DialogCode {Discarded=2};

    SetLocation(const MercatorCoordinates &ACoordinates, const QIcon &AIcon, QWidget *parent = 0);
    ~SetLocation();

    MercatorCoordinates coordinates() const;

protected:
    void changeEvent(QEvent *e);

protected slots:
	void discard();

private:
    Ui::SetLocation *ui;

};

#endif // SETLOCATION_H



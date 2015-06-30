#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H

#include <QSlider>
#include "utilsexport.h"

class UTILS_EXPORT VolumeControl : public QSlider
{
    Q_OBJECT
public:
    explicit VolumeControl(QWidget *parent = 0);
    explicit VolumeControl(Qt::Orientation orientation, QWidget *parent = 0);
    
signals:
    
public slots:

protected:
    virtual void paintEvent(QPaintEvent *ev);
    
};

#endif // VOLUMECONTROL_H

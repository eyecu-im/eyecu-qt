#ifndef INFOWIDGETLABEL_H
#define INFOWIDGETLABEL_H

#include <QLabel>

class InfoWidgetLabel : public QLabel
{
    Q_OBJECT
public:
    explicit InfoWidgetLabel(QWidget *parent = 0);

protected:
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent * ev);

private:
    QPoint clickPos;

signals:
    void clicked(QString, QString);
    
public slots:

};

#endif // INFOWIDGETLABEL_H

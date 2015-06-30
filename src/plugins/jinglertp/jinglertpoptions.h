#ifndef JINGLERTPOPTIONS_H
#define JINGLERTPOPTIONS_H

#include <QWidget>
#include <QDebug>
#include <QAudioDeviceInfo>

#include <interfaces/ioptionsmanager.h>
#include <utils/options.h>

#include "ui_jinglertpoptions.h"

namespace Ui {
class JingleRtpOptions;
}

class JingleRtpOptions :
    public QWidget,
	public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    explicit JingleRtpOptions(QWidget *parent = 0);
    ~JingleRtpOptions();
    virtual QWidget* instance() { return this; }
    Ui::JingleRtpOptions *ui;

public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected slots:
    void modify(int);
    void deviceChanged(int index);
    void videoDeviceChanged(int index);

protected:
    void changeEvent(QEvent *e);

    void getSuppSampleRates();

private:
    QAudioDeviceInfo m_device;
};

#endif // JINGLERTPOPTIONS_H

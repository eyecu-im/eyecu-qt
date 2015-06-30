#ifndef MULTIMEDIAPLAYEROPTIONS_H
#define MULTIMEDIAPLAYEROPTIONS_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
class MultimediaPlayerOptions;
}

class MultimediaPlayerOptions:
        public QWidget,
		public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    explicit MultimediaPlayerOptions(QWidget *parent = 0);
    ~MultimediaPlayerOptions();

// IOptionsWidget
    QWidget* instance() {return this; }
public slots:
    virtual void apply();
    virtual void reset();

signals:
    void modified();
    void childApply();
    void childReset();

private:
    Ui::MultimediaPlayerOptions *ui;
};

#endif // MULTIMEDIAPLAYEROPTIONS_H

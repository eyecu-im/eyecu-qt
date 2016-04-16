#ifndef AVATAROPTIONSWIDGET_H
#define AVATAROPTIONSWIDGET_H

#include <interfaces/ioptionsmanager.h>

namespace Ui {
    class AvatarOptionsWidget;
}

class AvatarOptionsWidget : public QWidget, public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    enum ShowAvatar
    {
        Right,
        Left
    };

	AvatarOptionsWidget(const OptionsNode &ANode, bool AOfflineAvailable, QWidget *parent = 0);
    ~AvatarOptionsWidget();
// IOptionsWidget
    QWidget* instance() {return this; }

public slots:
    void apply();
    void reset();

signals:
    void modified();
    void childApply();
    void childReset();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AvatarOptionsWidget *ui;
	OptionsNode FNode;
};

#endif // AVATAROPTIONSWIDGET_H

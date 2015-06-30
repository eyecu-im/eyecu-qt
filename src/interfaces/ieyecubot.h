#ifndef IEYECUBOT_H
#define IEYECUBOT_H

#include <QObject>

#define EYECUBOT_UUID "{639EDDAA-A684-42e4-A9AD-28FC9BCB8F7C}"

class IEyecubot {
public:
	virtual QObject *instance() =0;

};

Q_DECLARE_INTERFACE(IEyecubot,"My.Plugin.IEyecubot/1.0")

#endif	//IEYECUBOT_H

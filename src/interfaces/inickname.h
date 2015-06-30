#ifndef INICKNAME_H
#define INICKNAME_H

#include <QObject>

#define NICKNAME_UUID "{BCA8F64A-D2C8-DE3B-2C84-BCF7510DA076}"
 
class INickname {
public:
	virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(INickname, "RWS.Plugin.INickname/1.0")

#endif	//INICKNAME_H

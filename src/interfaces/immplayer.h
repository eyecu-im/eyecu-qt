#ifndef IMMPLAYER_H
#define IMMPLAYER_H

#include <QObject>

#define MMPLAYER_UUID "{3d5702bc-29b9-40f2-88fe-85887cd6d8d7}"
 
class IMmPlayer {
public:
    virtual QObject *instance() = 0;
};

Q_DECLARE_INTERFACE(IMmPlayer, "RWS.Plugin.IMmPlayer/1.0")

#endif	//IMMPLAYER_H

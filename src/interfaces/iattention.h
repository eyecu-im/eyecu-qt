#ifndef IATTENTION_H
#define IATTENTION_H

#include <QObject>
#include <QIcon>

#define ATTENTION_UUID "{9F8DEB29-4AA4-3727-8474-A34B35B7630C}"
                     
class IAttention {

public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IAttention, "RWS.Plugin.IAttention/1.0")

#endif	//IATTENTION_H

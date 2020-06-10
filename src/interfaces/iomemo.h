#ifndef IOMEMO
#define IOMEMO_H

#include <QObject>
#include <utils/stanza.h>

#define OMEMO_UUID "{b462d3c0-388a-c42f-69b7-cb8c42a55381}"
 
class IOmemo
{
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IOmemo, "RWS.Plugin.IOmemo/1.0")

#endif	//IOMEMO_H
